# pyxcp-appgen

Arduino and native-PC cXCP/pyXCP code generator and build frontend.

## Overview

`pyxcp-appgen` generates source code and A2L files from a JSON project model.
The default target remains Arduino: it compiles and uploads a sketch through
`arduino-cli`. The optional PC target emits a native CMake project backed by
the cXCP Flash Emulator and XCP-on-Ethernet.

## Installation

```bash
pip install -e .
```

## Usage

```bash
pyxcp-appgen <model.json> [generate|build|a2l|upload|clean] [--force] [--port COM4] [--fqbn arduino:avr:uno]
```

| Command    | Description                                      |
|------------|--------------------------------------------------|
| `generate` | Generate sources for the configured target       |
| `build`    | Build through `arduino-cli` or CMake             |
| `a2l`      | Generate A2L + pyXCP recorder configuration      |
| `upload`   | Upload an Arduino sketch (Arduino only)          |
| `clean`    | Remove the target's build artifacts               |

Flags:
- `--port`: Serial port of the board (e.g., `COM4`).
- `--fqbn`: Fully qualified board name for `arduino-cli` (e.g., `arduino:avr:uno`).

Omitting a command runs the full chain: `generate` → `build` → `a2l`.

## Targets and output paths

`target` is a top-level optional field. Its default is `"arduino"`, preserving
the historical behavior and default `sketch/` output directory. `output_dir`
is also top-level and must be relative to the model JSON file; it replaces that
target's default output directory.

Set `"target": "pc"` to generate a CMake project (default output: `pc/`). The
PC target deliberately has no board discovery, `arduino-cli`, or upload path.
Its generated `CMakeLists.txt` requires `CXCP_SRC_PATH` to point at the cXCP
repository root and reports a clear CMake configuration error otherwise.

```json
{
  "target": "pc",
  "output_dir": "generated/engine_pc",
  "pc": {
    "transport": "ETH",
    "segments": [
      {
        "name": "calibration",
        "type": "FLASH",
        "base_address": "0x80000000",
        "size": "0x4000",
        "page_size": "0x2000",
        "sector_size": "0x100",
        "pages": 2,
        "address_extension": 0
      },
      {
        "name": "measurements",
        "type": "RAM",
        "base_address": "0x20000000",
        "size": "0x1000",
        "page_size": "0x1000",
        "sector_size": 1,
        "pages": 1,
        "address_extension": 0
      }
    ]
  },
  "project": { "...": "normal project model" }
}
```

`pc.transport` currently supports only `"ETH"` and defaults to it.
`pc.segments` defaults to the two segments above. A segment's `size` must
equal `page_size * pages`; RAM segments must have one page. `type` is one of
`FLASH`, `EEPROM`, or `RAM`; optional `write_size` and `blocks` both default
to 1. `base_address` can also be written as `address`. Numeric fields accept
JSON numbers or `0x`-prefixed strings.

For PC, a measurement's stable virtual address is its `ecu_address` (or
`address`) and its extension is `ecu_address_extension` (or
`address_extension`). If omitted, addresses are deterministically allocated
from the first RAM segment and written into the generated A2L/address map.
All addresses must fit a configured virtual segment. The emitted
`address_map.json`, A2L, and recorder config share this map.

### Optional PE/COFF RVA symbol map

PC projects can optionally derive A2L addresses from PE/COFF symbols after a
Windows build. This uses `objutils.pecoff.PeParser` when it is installed; the
normal PC path has no dependency on this optional parser. PE values are always
treated as RVAs (the equivalent of `oj_coff_extract.py --no-image-base`):
COFF section-relative values are made relative to their section RVA and
preferred-image-base PDB values have their image base removed. Therefore A2L
addresses remain stable with PE32, PE32+, and ASLR.

The extracted RVA is added to the configured `rva_base`, then checked against
the Flash Emulator segments. This preserves the configured virtual-address
contract: the A2L still addresses emulator memory, never a process pointer or
ASLR-dependent load address.

```json
{
  "pc": {
    "pe_symbols": {
      "file": "generated/engine_pc/build/Debug/engine.exe",
      "rva_base": "0x20000000",
      "aliases": { "engine.speed": "speed_symbol" },
      "required": true
    }
  }
}
```

`file` is relative to the JSON model unless absolute. Measurements use their
`pe_symbol`, an `aliases` value, or their name (in that order). `rva_base`
defaults to zero; set it so the resulting addresses fit a configured segment.
`required` defaults to true and gives a clear error for missing symbols; use
false to retain configured/emulator addresses for unavailable symbols. Run
`a2l` after the PE exists to refresh the A2L, recorder config, and address map.

The generated PC project contains `CMakeLists.txt`, `main.cpp`,
`application.cpp`, `xcp_config.h`, A2L, recorder configuration, and address
map. Build it with:

```bash
export CXCP_SRC_PATH=/path/to/cxcp  # PowerShell: $env:CXCP_SRC_PATH = 'C:\path\to\cxcp'
pyxcp-appgen model.json
# or: cmake -S generated/engine_pc -B generated/engine_pc/build
#     cmake --build generated/engine_pc/build
```

`project.module` is the preferred single-module form. A legacy
`project.modules` list with exactly one module is accepted and normalized.
Measurements and characteristics may use `id` in place of `name`; it is
normalized without replacing an explicitly supplied `name`.

### Required cXCP header

Set the environment variable `CXCP_SRC_PATH` to the root of the cXCP repository so `tools/xcp.h` can be copied into `sketch/` and tracked. The generator will copy it automatically when missing or changed.

### cXCP configuration defaults

`pyxcp_appgen` keeps a single source of truth for cXCP options in `xcp_config_defaults.json` at the repository root (names, types, default values, enums, group). On generation, this JSON is rendered into `sketch/xcp_config.h`; edit the JSON instead of the header and re-run the generator.

## Project structure

```
tools/app_gen/
├── src/pyxcp_appgen/   # Python package
│   └── templates/      # Mako templates
├── tests/              # pytest tests
├── docs/               # Documentation
├── examples/           # Example projects and scripts
└── pyproject.toml
```
