"""PC/CMake target support for generated cXCP applications."""

from __future__ import annotations

import copy
import logging
import re
import shutil
import subprocess
from pathlib import Path
from typing import Any, Dict, Iterable, List

DEFAULT_SEGMENTS = [
    {
        "name": "calibration",
        "type": "FLASH",
        "base_address": 0x80000000,
        "size": 0x4000,
        "page_size": 0x2000,
        "sector_size": 0x100,
        "pages": 2,
        "address_extension": 0,
    },
    {
        "name": "measurements",
        "type": "RAM",
        "base_address": 0x20000000,
        "size": 0x1000,
        "page_size": 0x1000,
        "sector_size": 1,
        "pages": 1,
        "address_extension": 0,
    },
]

_C_TYPES = {
    "uint8": "uint8_t", "int8": "int8_t", "uint16": "uint16_t", "int16": "int16_t",
    "uint32": "uint32_t", "int32": "int32_t", "uint64": "uint64_t", "int64": "int64_t",
    "float16": "float", "float": "float", "double": "double",
}


def _number(value: Any, field: str) -> int:
    if isinstance(value, bool):
        raise ValueError(f"PC segment {field!r} must be an integer, not a boolean.")
    try:
        return int(value, 0) if isinstance(value, str) else int(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"PC segment {field!r} must be an integer or a 0x-prefixed string.") from exc


def c_identifier(name: str) -> str:
    """Return a deterministic C identifier suitable for generated local symbols."""
    identifier = re.sub(r"\W", "_", name)
    if not identifier or identifier[0].isdigit():
        identifier = f"segment_{identifier}"
    return identifier


def normalize_pc_config(config: Dict[str, Any]) -> Dict[str, Any]:
    """Validate the PC configuration and complete its Flash Emulator layout."""
    result = dict(config)
    transport = str(result.setdefault("transport", "ETH")).upper()
    if transport != "ETH":
        raise ValueError("The PC target currently supports only pc.transport = 'ETH'.")
    result["transport"] = transport

    pe_symbols = result.get("pe_symbols")
    if pe_symbols is not None:
        if not isinstance(pe_symbols, dict) or not isinstance(pe_symbols.get("file"), str):
            raise ValueError("pc.pe_symbols must be an object with a string 'file'.")
        aliases = pe_symbols.get("aliases", {})
        if not isinstance(aliases, dict) or not all(
                isinstance(measurement, str) and isinstance(symbol, str)
                for measurement, symbol in aliases.items()
        ):
            raise ValueError("pc.pe_symbols.aliases must map measurement names to PE symbol names.")
        normalized_pe_symbols = dict(pe_symbols)
        normalized_pe_symbols["aliases"] = aliases
        normalized_pe_symbols["rva_base"] = _number(
            pe_symbols.get("rva_base", 0), "pe_symbols.rva_base"
        )
        normalized_pe_symbols["required"] = bool(pe_symbols.get("required", True))
        result["pe_symbols"] = normalized_pe_symbols

    raw_segments = result.get("segments", copy.deepcopy(DEFAULT_SEGMENTS))
    if not isinstance(raw_segments, list) or not raw_segments:
        raise ValueError("pc.segments must be a non-empty list.")

    segments: List[Dict[str, Any]] = []
    seen_names = set()
    for index, raw_segment in enumerate(raw_segments):
        if not isinstance(raw_segment, dict):
            raise ValueError(f"pc.segments[{index}] must be an object.")
        segment = dict(raw_segment)
        name = segment.get("name")
        if not isinstance(name, str) or not name:
            raise ValueError(f"pc.segments[{index}].name must be a non-empty string.")
        if name in seen_names:
            raise ValueError(f"Duplicate PC segment name: {name}")
        seen_names.add(name)

        memory_type = str(segment.get("type", "RAM")).upper()
        if memory_type not in {"FLASH", "EEPROM", "RAM"}:
            raise ValueError(f"Unsupported PC segment type {memory_type!r} for {name!r}.")
        address = _number(segment.get("base_address", segment.get("address")), f"{name}.base_address")
        size = _number(segment.get("size"), f"{name}.size")
        page_size = _number(segment.get("page_size", size), f"{name}.page_size")
        sector_size = _number(segment.get("sector_size", 1), f"{name}.sector_size")
        pages = _number(segment.get("pages", size // page_size if page_size else 0), f"{name}.pages")
        extension = _number(segment.get("address_extension", 0), f"{name}.address_extension")
        write_size = _number(segment.get("write_size", 1), f"{name}.write_size")
        blocks = _number(segment.get("blocks", 1), f"{name}.blocks")
        if min(size, page_size, sector_size, pages, write_size, blocks) <= 0:
            raise ValueError(f"PC segment {name!r} sizes, pages, write_size and blocks must be positive.")
        if size != page_size * pages:
            raise ValueError(f"PC segment {name!r} requires size == page_size * pages.")
        if not 0 <= extension <= 255:
            raise ValueError(f"PC segment {name!r}.address_extension must be in range 0..255.")
        if memory_type == "RAM" and pages != 1:
            raise ValueError(f"RAM PC segment {name!r} must have exactly one page.")

        segment.update({
            "name": name, "c_name": c_identifier(name), "type": memory_type,
            "base_address": address, "size": size, "page_size": page_size,
            "sector_size": sector_size, "pages": pages, "address_extension": extension,
            "write_size": write_size, "blocks": blocks,
        })
        segments.append(segment)
    result["segments"] = segments
    return result


def _measurement_size(measurement: Dict[str, Any]) -> int:
    try:
        return {
            "uint8": 1, "int8": 1, "uint16": 2, "int16": 2,
            "uint32": 4, "int32": 4, "float": 4, "float16": 2,
            "uint64": 8, "int64": 8, "double": 8,
        }[measurement.get("type")]
    except KeyError as exc:
        raise ValueError(
            f"Measurement {measurement.get('name', '<unnamed>')!r} has an unsupported PC type "
            f"{measurement.get('type')!r}."
        ) from exc


def _segment_for_address(segments: Iterable[Dict[str, Any]], address: int, size: int) -> Dict[str, Any]:
    for segment in segments:
        # A paged Flash Emulator segment exposes one page at its stable virtual address.
        visible_size = segment["size"] if segment["type"] == "RAM" else segment["page_size"]
        if segment["base_address"] <= address and address + size <= segment["base_address"] + visible_size:
            return segment
    raise ValueError(f"Virtual measurement address 0x{address:X} is outside every PC segment.")


def assign_measurement_addresses(
        measurements: Iterable[Dict[str, Any]], segments: List[Dict[str, Any]]
) -> Dict[str, int]:
    """Return the stable virtual address map and complete omitted measurement addresses."""
    allocation_segment = next((segment for segment in segments if segment["type"] == "RAM"), segments[0])
    next_address = allocation_segment["base_address"]
    addresses: Dict[str, int] = {}

    for measurement in measurements:
        name = measurement["name"]
        size = _measurement_size(measurement)
        configured_address = measurement.get("ecu_address", measurement.get("address"))
        if configured_address is None:
            alignment = min(size, 8)
            next_address = (next_address + alignment - 1) & ~(alignment - 1)
            address = next_address
            next_address += size
        else:
            address = _number(configured_address, f"measurement {name}.ecu_address")
        segment = _segment_for_address(segments, address, size)
        measurement["ecu_address"] = f"0x{address:X}"
        measurement.setdefault("ecu_address_extension", segment["address_extension"])
        addresses[name] = address
    return addresses


def assign_pe_rva_addresses(
        measurements: Iterable[Dict[str, Any]],
        segments: List[Dict[str, Any]],
        rvas: Dict[str, int],
        rva_base: int,
        aliases: Dict[str, str],
        required: bool,
) -> Dict[str, int]:
    """Overlay configured virtual addresses from PE RVAs onto the Flash layout."""
    addresses: Dict[str, int] = {}
    missing = []
    for measurement in measurements:
        name = measurement["name"]
        symbol = measurement.get("pe_symbol", aliases.get(name, name))
        rva = rvas.get(symbol)
        if rva is None:
            if required:
                missing.append(symbol)
            continue
        address = rva_base + rva
        segment = _segment_for_address(segments, address, _measurement_size(measurement))
        measurement["ecu_address"] = f"0x{address:X}"
        measurement["ecu_address_extension"] = segment["address_extension"]
        addresses[name] = address
    if missing:
        raise ValueError(f"Required PE/COFF symbols were not found: {sorted(missing)}")
    return addresses


class PcManager:
    """Runs CMake for a generated PC target without any Arduino dependency."""

    def __init__(self, project_path: Path) -> None:
        self.project_path = project_path
        self.build_path = project_path / "build"
        self.logger = logging.getLogger("rich")

    def _run(self, command: List[str]) -> None:
        self.logger.info("Executing: %s", " ".join(command))
        result = subprocess.run(command, cwd=self.project_path, capture_output=True, text=True, encoding="utf-8")
        if result.returncode != 0:
            if result.stdout:
                self.logger.error("STDOUT:\n%s", result.stdout)
            if result.stderr:
                self.logger.error("STDERR:\n%s", result.stderr)
            raise subprocess.CalledProcessError(result.returncode, command, result.stdout, result.stderr)

    def configure(self) -> None:
        self._run(["cmake", "-S", str(self.project_path), "-B", str(self.build_path)])

    def build(self, config: str = "Debug") -> None:
        self.configure()
        self._run(["cmake", "--build", str(self.build_path), "--config", config])
        self._run(["cmake", "--install", str(self.build_path), "--config", config])

    def clean(self) -> None:
        if (self.build_path / "CMakeCache.txt").exists():
            self._run(["cmake", "--build", str(self.build_path), "--target", "clean"])
        elif self.build_path.exists():
            shutil.rmtree(self.build_path)
