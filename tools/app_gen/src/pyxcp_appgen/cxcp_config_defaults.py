import itertools as it
import json
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional

DEFAULT_SPEC_PATH = Path(__file__).parents[4] / "xcp_config_defaults.json"


def load_spec(spec_path: Path = DEFAULT_SPEC_PATH) -> List[Dict[str, Any]]:
    with open(spec_path, "r", encoding="utf-8") as f:
        return json.load(f)


def grouped_spec(spec: List[Dict[str, Any]]) -> Iterable[Iterable[Dict[str, Any]]]:
    return it.groupby(sorted(spec, key=lambda x: x["group"]), key=lambda x: x["group"])

def _to_c_value(entry: Dict[str, Any], value: Any) -> Optional[str]:
    kind = entry.get("type")
    if kind == "define":
        return None
    if kind == "bool":
        if isinstance(value, str):
            return "XCP_ON" if value.strip().upper() in {"XCP_ON", "ON", "TRUE"} else "XCP_OFF"
        return "XCP_ON" if bool(value) else "XCP_OFF"
    if kind == "string" and isinstance(value, str) and not value.startswith('"'):
        return f"\"{value}\""
    return str(value)


GROUPS = [
    'identification',
    'general',
    'protection',
    'checksum',
    'cmd_std',
    'cmd_daq',
    'cmd_cal',
    'cmd_pag',
    'cmd_pgm',
    'daq',
    'pgm',
    'customization',
    'api',
    'time_correlation',
    'application',
]

TR_REL = {
    "CAN": ['tl_can'],
    "ETH": ['tl_eth', 'eth_discovery'],
    "SXI": ['tl_sxi']
}

TR_SEL = {
    "CAN": "    #define TP_CAN\n    #define XCP_TRANSPORT_LAYER XCP_ON_CAN\n",
    "ETH": "    #define TP_ETHER\n    #define XCP_TRANSPORT_LAYER XCP_ON_ETHERNET\n",
    "SXI": "    #define TP_SXI\n    #define XCP_TRANSPORT_LAYER XCP_ON_SXI\n"
}

def render_xcp_config(
    destination: Path,
    transport: str,
    spec: Optional[Iterable[Dict[str, Any]]] = None,
    overrides: Optional[Dict[str, Any]] = None,
) -> None:
    overrides = overrides or {}
    spec = list(spec) if spec is not None else load_spec()

    lines = [
        "#if !defined(__XCP_CONFIG_H)",
        "    #define __XCP_CONFIG_H\n\n",
    ]
    tr_specific = TR_REL.get(transport)
    if tr_specific:
        GROUPS.extend(tr_specific)
    lines.append(TR_SEL.get(transport, "\n"))
    PARAMETER_GROUPS = {group_name: sorted(group, key=lambda x: x["name"]) for group_name, group in grouped_spec(spec)}
    for group_name in GROUPS:
        group = PARAMETER_GROUPS.get(group_name)
        if not group:
            print("Empty group for:", group_name)
            continue
        lines.append(f"    /*\n    ** {group_name.upper().replace('_', ' ')}\n    */")
        for entry in sorted(group, key=lambda x: x["name"]):
            name = entry["name"]
            value = overrides.get(name, entry.get("default"))
            c_value = _to_c_value(entry, value)
            if c_value is None:
                lines.append(f"    #define {name}")
            else:
                lines.append(f"    #define {name} ({c_value})")
    lines.append("")
    lines.append("#endif /* __XCP_CONFIG_H*/\n")
    lines.append("")

    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text("\n".join(lines), encoding="utf-8")

__all__ = ["DEFAULT_SPEC_PATH", "load_spec", "render_xcp_config"]
