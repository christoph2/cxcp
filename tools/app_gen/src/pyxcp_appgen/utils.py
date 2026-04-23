import json
from hashlib import sha256
import logging
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Union

from objutils.elf import ElfParser

# Type definitions for clarity
TYPES: Dict[str, str] = {
    "uint8": "UBYTE", "int8": "SBYTE", "uint16": "UWORD", "int16": "SWORD",
    "uint32": "ULONG", "int32": "SLONG", "uint64": "A_UINT64", "int64": "A_INT64",
    "float16": "FLOAT16_IEEE", "float": "FLOAT32_IEEE", "double": "FLOAT64_IEEE",
}

INITIALIZERS: Dict[str, str] = {
    "uint8": "0", "int8": "0", "uint16": "0U", "int16": "0",
    "uint32": "0UL", "int32": "0", "uint64": "0ULL", "int64": "0",
    "float16": "0.0", "float": "0.0f", "double": "0.0",
}

TYPE_MAP_A2L: Dict[str, str] = {
    "uint8": "U8", "int8": "I8", "uint16": "U16", "int16": "I16",
    "uint32": "U32", "int32": "I32", "uint64": "U64", "int64": "I64",
    "float16": "F16", "float": "F32", "double": "F64"
}


def sha256_hash_of_file(file_name: Union[str, Path]) -> Optional[str]:
    """Calculates the SHA256 hash of a file."""
    file_path = Path(file_name)
    if not file_path.exists():
        logging.getLogger("rich").info(f"File {file_name} does not exist.")
        return None
    with file_path.open("rb") as f:
        return sha256(f.read()).hexdigest()


def load_symbol_addresses_from_elf(elf_file: Path, symbol_list: List[str]) -> Dict[str, int]:
    """Looks up symbol addresses in an ELF file."""
    logger = logging.getLogger("rich")
    if not symbol_list:
        logger.info("No measurement names found; skipping DWARF lookup.")
        return {}
    if not elf_file.exists():
        logger.warning(f"ELF file '{elf_file}' does not exist; skipping.")
        return {}

    try:
        ep = ElfParser(str(elf_file))
        symbols = ep.symbols.fetch(symbol_list=symbol_list, group_by_section=False)
        symbol_addresses = {s.symbol_name: s.st_value for s in symbols}
        logger.info(f"Found {len(symbol_addresses)} ELF variable symbols.")

        missing = set(symbol_list) - set(symbol_addresses.keys())
        if missing:
            logger.info(f"Symbols not found in ELF: {sorted(missing)}")
        else:
            logger.info("All symbols found in ELF.")
        return symbol_addresses
    except Exception as e:
        logger.error(f"ELF symbol query failed: {e}")
        raise


def validate_events(events: Optional[List[Dict[str, Any]]]) -> List[Dict[str, Any]]:
    """Validates and completes event configuration."""
    logger = logging.getLogger("rich")
    if not events:
        logger.warning("No events configured. Adding default event.")
        return [{"name": "default_event", "number": 0, "interval": 10}]

    names: Set[str] = set()
    numbers: Set[int] = set()

    for event in events:
        name = event.get("name")
        if not name:
            raise ValueError("Event name is required.")
        if name in names:
            raise ValueError(f"Duplicate event name: {name}")
        names.add(name)

        number = event.get("number")
        if number is not None:
            if not isinstance(number, int) or number < 0:
                raise ValueError(f"Event number must be a non-negative integer, got: {number}")
            if number in numbers:
                raise ValueError(f"Duplicate event number: {number}")
            numbers.add(number)

    next_number = 0
    for event in events:
        if event.get("number") is None:
            while next_number in numbers:
                next_number += 1
            event["number"] = next_number
            numbers.add(next_number)
    return events


def validate_mod_common(mod_common: Optional[Dict[str, Any]]) -> Dict[str, Any]:
    """Validates and completes mod_common configuration with defaults."""
    logger = logging.getLogger("rich")
    defaults = {
        "byte_order": "MSB_LAST", "alignment_byte": 1, "alignment_word": 2,
        "alignment_long": 4, "alignment_float16_ieee": 2, "alignment_float32_ieee": 4,
        "alignment_float64_ieee": 8, "alignment_int64": 8,
    }
    if not mod_common:
        logger.warning("No mod_common configuration found. Adding defaults.")
        return defaults

    for key, value in defaults.items():
        if key not in mod_common:
            logger.info(f"Setting default for mod_common: '{key}': {value}")
            mod_common[key] = value
    return mod_common


def load_config_json(json_file: Path) -> Dict[str, Any]:
    """Loads the main JSON configuration file."""
    logger = logging.getLogger("rich")
    try:
        with open(json_file, "r", encoding="utf-8") as f:
            data_model = json.load(f)
        if not isinstance(data_model, dict):
            logger.error(f"Invalid JSON format in {json_file}. Expected a dictionary.")
            raise ValueError("JSON model must be a dictionary.")
        return data_model
    except FileNotFoundError:
        logger.error(f"JSON file not found: {json_file}")
        raise
    except json.JSONDecodeError as e:
        logger.error(f"Error parsing JSON: {e}")
        raise
