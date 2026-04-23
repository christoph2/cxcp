# python
import argparse
import json
import logging
import os
import shutil
import sys
import traceback
from pathlib import Path
from typing import Any, Dict, List, Optional, Union

from arduinodisco import discover_boards
from rich.console import Console
from rich.logging import RichHandler
from rich.prompt import Prompt
from rich.table import Table

from pyxcp_appgen.arduino import ArduinoManager
from pyxcp_appgen.renderer import Renderer
from pyxcp_appgen.state import ProjectState
from pyxcp_appgen.cxcp_config_defaults import DEFAULT_SPEC_PATH, render_xcp_config
from pyxcp_appgen import utils

VALID_TRANSPORTS = ("SXI", "CAN", "ETH")

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TEMPLATES_DIR = os.path.join(SCRIPT_DIR, "templates")


class AppGenerator:
    """
    Generator für A2L- und Arduino-Quellcode basierend auf einem JSON-Modell.
    """

    def __init__(self, json_file: str, port: Optional[str] = None, fqbn: Optional[str] = None) -> None:
        self.logger = logging.getLogger("rich")

        self.project_config_file: Path = Path(json_file).absolute()
        self.base_dir: Path = self.project_config_file.parent.absolute()
        self.sketch_pth: Path = self.base_dir / "sketch"
        self.sketch_ino: Path = self.sketch_pth / "sketch.ino"
        self.elf_file: Path = self.sketch_pth / "arduino-build" / "sketch.ino.elf"
        self.cli_port = port
        self.cli_fqbn = fqbn

        self.state = ProjectState(self.sketch_pth / ".project-state.json")
        self.arduino_manager = ArduinoManager(self.sketch_pth)

        self.data_model = utils.load_config_json(self.project_config_file)
        self.set_defaults()
        self._ensure_xcp_header()
        self._ensure_xcp_config()

        self._handle_board_selection()

        self.symbol_addresses: Dict[str, int] = {}
        self.renderer = Renderer(TEMPLATES_DIR, self.data_model, self.symbol_addresses)

    def __del__(self) -> None:
        self.state.save()

    def _handle_board_selection(self) -> None:
        available_boards = {p.port.device: p.board.fqbn for p in discover_boards()}
        self.state.boards["available"] = available_boards

        if self.cli_port or self.cli_fqbn:
            if not self.cli_port or not self.cli_fqbn:
                raise RuntimeError("Both --port and --fqbn are required when selecting a board via CLI.")
            self.logger.info(f"Using CLI board selection: {self.cli_fqbn} on {self.cli_port}")
            self.state.boards["selected"] = {self.cli_port: self.cli_fqbn}
            return

        selected_port_fqbn = self.state.boards.get("selected")
        if selected_port_fqbn:
            port, fqbn = next(iter(selected_port_fqbn.items()))
            if port in available_boards and available_boards[port] == fqbn:
                self.logger.info(f"Using previously selected board: {fqbn} on {port}")
                return  # Board is still valid

        selected_board = self.discover_board(available_boards)
        if selected_board:
            fqbn = selected_board.board.fqbn
            port = selected_board.port.device
            self.logger.info(f"Selected board: {fqbn}, Port: {port}")
            self.state.boards["selected"] = {port: fqbn}
        elif not self.state.boards.get("selected"):
            self.logger.error("No board selected or discovered. Cannot proceed.")
            raise RuntimeError("Board selection is required.")

    def set_defaults(self) -> None:
        self.sketch_pth.mkdir(parents=True, exist_ok=True)

        project = self.data_model.get("project")
        if not project:
            raise ValueError("No 'project' configuration found in JSON.")

        project.setdefault("asap2_version", [1, 71])
        project_name = project.get("name", "project_name")


        arduino = self.data_model.get("arduino")
        if arduino is None:
            self.data_model.setdefault("arduino", {})
            arduino = self.data_model["arduino"]

        if not "transport" in arduino:
            self.logger.warning("No 'transport' configuration found. Adding default 'SXI'.")
            arduino.setdefault("transport", "SXI")

        self.transport = arduino["transport"].upper()
        if self.transport not in VALID_TRANSPORTS:
            self.logger.error(f"Invalid transport {self.transport!r}. Choose one of  {VALID_TRANSPORTS!r}.")
            raise
        module = project.get("module")
        if not module:
            raise ValueError("No 'module' configuration found in 'project'.")

        try:
            module["events"] = utils.validate_events(module.get("events"))
            module["mod_common"] = utils.validate_mod_common(module.get("mod_common"))
        except ValueError as e:
            self.logger.error(f"Configuration validation failed: {e}")
            raise

        module.setdefault("includes", ["XCP_104.aml"])

        self.a2l_file = self.sketch_pth / f"{project_name}.a2l"
        self.xcp_rec_config_file = self.sketch_pth / "xcp_rec_config.json"
        self.xcp_config_file = self.sketch_pth / "xcp_config.h"

    def _ensure_xcp_header(self) -> None:
        """Ensure xcp.h is present in sketch/ and tracked for changes."""
        dest = self.sketch_pth / "xcp.h"
        env_root = os.environ.get("CXCP_SRC_PATH")
        source = Path(env_root) / "tools" / "xcp.h" if env_root else None

        source_hash = utils.sha256_hash_of_file(source) if source and source.exists() else None
        dest_hash = utils.sha256_hash_of_file(dest)

        if source_hash is None and not dest.exists():
            raise FileNotFoundError(
                "xcp.h not found. Set CXCP_SRC_PATH to the cXCP repository root so tools/xcp.h is available."
            )

        copy_needed = source_hash is not None and source_hash != dest_hash
        if copy_needed:
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, dest)
            self.logger.info(f"Copied xcp.h from {source} to {dest}")

        # Always track dest hash for build invalidation
        self.state.update_hash(dest)

    def _ensure_xcp_config(self, force: bool = False) -> None:
        """Generate xcp_config.h from the JSON defaults."""
        spec_path = DEFAULT_SPEC_PATH
        dest = self.xcp_config_file
        needs_regen = (
            force
            or not dest.exists()
            or self.state.file_is_modified(dest)
            or self.state.file_is_modified(spec_path)
        )
        if needs_regen:
            render_xcp_config(dest, self.transport)
            self.logger.info(f"Generated cXCP config header: {dest}")
        self.state.update_hash(dest)
        self.state.update_hash(spec_path)

    def generate(self, force: bool = False) -> None:
        """Generates the Arduino sketch."""
        if force or self.state.file_is_modified(self.project_config_file):
            self.renderer.render_sketch(self.sketch_ino)
            self.state.update_hash(self.project_config_file)
            self.logger.info("Sketch successfully generated.")
        else:
            self.logger.info("Sketch is up to date.")

    def build(self, force: bool = False) -> None:
        """Compiles the project to an ELF file."""
        user_code = self.sketch_pth / "user_code.ino"
        if not user_code.exists():
            user_code.write_text("// User code goes here\nvoid setup_user() {}\nvoid loop_user() {}\n",
                                 encoding="utf-8")
            self.logger.info(f"Stub created: {user_code}")

        rebuild_needed = (
                force
                or not self.elf_file.exists()
                or self.state.file_is_modified(self.sketch_ino)
                or self.state.file_is_modified(user_code)
        )

        if rebuild_needed:
            selected = self.state.boards.get("selected")
            if not selected:
                raise RuntimeError("No board selected. Build not possible.")
            fqbn = list(selected.values())[0]

            self.arduino_manager.build(fqbn)
            self.state.update_hash(self.elf_file)
            self.state.update_hash(self.sketch_ino)
            self.state.update_hash(user_code)
            self.logger.info(f"Build successful. ELF file: {self.elf_file}")
        else:
            self.logger.info("Build is up to date.")

    def create_a2l_artifacts(self, force: bool = False) -> None:
        """Extracts symbols and generates A2L + pyXCP config."""
        if not self.elf_file.exists():
            raise FileNotFoundError("ELF file missing. Please build first.")

        self.load_symbol_addresses()
        self.renderer.symbol_addresses = self.symbol_addresses

        if force or self.state.file_is_modified(self.a2l_file):
            self.renderer.render_a2l(self.a2l_file)
            self.state.update_hash(self.a2l_file)
            self.logger.info(f"A2L file generated: {self.a2l_file}")

        if force or self.state.file_is_modified(self.xcp_rec_config_file):
            self.renderer.render_xcp_rec_config(self.xcp_rec_config_file)
            self.state.update_hash(self.xcp_rec_config_file)
            self.logger.info(f"XCP recorder config generated: {self.xcp_rec_config_file}")

    def upload(self) -> None:
        """Uploads the sketch to the board."""
        selected = self.state.boards.get("selected")
        if not selected:
            raise RuntimeError("No board or port selected. Upload not possible.")
        port, fqbn = next(iter(selected.items()))
        self.arduino_manager.upload(fqbn, port)
        self.logger.info("Upload successful.")

    def clean(self) -> None:
        self.arduino_manager.clean()

    def load_symbol_addresses(self) -> None:
        meas = self.renderer.get_measurements()
        symbol_list = [m.get("name") for m in meas if m.get("name")]
        self.symbol_addresses = utils.load_symbol_addresses_from_elf(self.elf_file, symbol_list)

    def discover_board(self, available_boards: Dict[str, str]) -> Optional[Any]:
        found_boards = discover_boards()
        if not found_boards:
            self.logger.warning("No boards found.")
            return None

        if len(found_boards) == 1:
            return found_boards[0]

        # Prioritize known boards
        known_ports = self.state.boards.get("available", {}).keys()
        sorted_boards = sorted(found_boards, key=lambda b: b.port.device not in known_ports)

        console = Console()
        table = Table(title="Available boards")
        table.add_column("#", justify="right", style="cyan", no_wrap=True)
        table.add_column("FQBN", style="magenta")
        table.add_column("Port", style="green")

        for i, board_info in enumerate(sorted_boards):
            table.add_row(str(i), board_info.board.fqbn, board_info.port.device)

        console.print(table)
        choices = [str(i) for i in range(len(sorted_boards))]
        choice = Prompt.ask("Select a board", choices=choices, default="0")
        return sorted_boards[int(choice)]


def main() -> None:
    logging.basicConfig(level="INFO", format="%(message)s", datefmt="[%X]",
                        handlers=[RichHandler(rich_tracebacks=True)])

    parser = argparse.ArgumentParser(description="Generate app code from JSON model.")
    parser.add_argument("json_model", help="Path to the a2l_model.json file")
    parser.add_argument("--port", help="Serial port of the Arduino board (e.g., COM4)")
    parser.add_argument("--fqbn", help="Fully qualified board name for arduino-cli (e.g., arduino:avr:uno)")

    subparsers = parser.add_subparsers(dest="command", help="Commands")

    generate_parser = subparsers.add_parser("generate", help="Generate Arduino sketch from configuration")
    generate_parser.add_argument("--force", action="store_true", help="Force generation")

    build_parser = subparsers.add_parser("build", help="Compile the sketch to an ELF file")
    build_parser.add_argument("--force", action="store_true", help="Force build")

    a2l_parser = subparsers.add_parser("a2l", help="Extract symbols from ELF and generate A2L/pyXCP config")
    a2l_parser.add_argument("--force", action="store_true", help="Force A2L generation")

    subparsers.add_parser("upload", help="Upload the sketch to the board")
    subparsers.add_parser("clean", help="Clean build artifacts")

    parser.add_argument("--force", action="store_true", help="Force execution of all steps in the default chain")

    args = parser.parse_args()
    try:
        gen = AppGenerator(args.json_model, port=args.port, fqbn=args.fqbn)
        force = getattr(args, "force", False)

        if args.command == "generate":
            gen.generate(force=force)
        elif args.command == "build":
            gen.build(force=force)
        elif args.command == "a2l":
            gen.create_a2l_artifacts(force=force)
        elif args.command == "upload":
            gen.upload()
        elif args.command == "clean":
            gen.clean()
        else:
            # Full chain
            gen.generate(force=force)
            gen.build(force=force)
            gen.create_a2l_artifacts(force=force)

    except (ValueError, FileNotFoundError, RuntimeError) as e:
        logging.getLogger("rich").error(f"A clear error occurred: {e}")
        sys.exit(1)
    except Exception as e:
        logging.getLogger("rich").error(f"An unexpected error occurred: {e}")
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
