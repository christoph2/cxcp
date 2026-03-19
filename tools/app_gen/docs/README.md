# pyxcp-appgen

Arduino-based cXCP/pyXCP code generator and build/upload frontend.

## Overview

`pyxcp-appgen` generates Arduino sketch source code and A2L files from a JSON
project model, compiles and uploads the sketch via `arduino-cli`, and produces
a pyXCP recorder configuration.

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
| `generate` | Generate Arduino sketch from JSON model          |
| `build`    | Compile sketch to ELF via arduino-cli            |
| `a2l`      | Extract ELF symbols and generate A2L + XCP config|
| `upload`   | Upload compiled sketch to the board              |
| `clean`    | Remove build artifacts                           |

Flags:
- `--port`: Serial port of the board (e.g., `COM4`).
- `--fqbn`: Fully qualified board name for `arduino-cli` (e.g., `arduino:avr:uno`).

Omitting a command runs the full chain: `generate` → `build` → `a2l`.

### Required cXCP header

Set the environment variable `CXCP_SRC_PATH` to the root of the cXCP repository so `tools/xcp.h` can be copied into `sketch/` and tracked. The generator will copy it automatically when missing or changed.

### cXCP configuration defaults

`pyxcp_appgen` keeps a single source of truth for cXCP options in `src/pyxcp_appgen/data/xcp_config_defaults.json` (names, types, default values, enums). On generation, this JSON is rendered into `sketch/xcp_config.h`; edit the JSON instead of the header and re-run the generator.

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
