import logging
import subprocess
import sys
from pathlib import Path
from typing import List, Union


def run_arduino_cli(commands: List[str], working_directory: Union[str, Path] = ".") -> None:
    """Executes an arduino-cli command and raises an exception on failure."""
    arduino_cli = "arduino-cli"
    full_cmd = [arduino_cli] + commands
    logging.getLogger("rich").info(f"Executing: {' '.join(full_cmd)}")
    result = subprocess.run(
        full_cmd,
        cwd=working_directory,
        capture_output=True,
        text=True,
        encoding='utf-8',
        errors='replace'
    )
    if result.returncode != 0:
        logging.getLogger("rich").error(f"arduino-cli failed with exit code {result.returncode}")
        if result.stdout:
            logging.getLogger("rich").error(f"STDOUT:\n{result.stdout}")
        if result.stderr:
            logging.getLogger("rich").error(f"STDERR:\n{result.stderr}")
        raise subprocess.CalledProcessError(result.returncode, full_cmd, result.stdout, result.stderr)


class ArduinoManager:
    """Handles Arduino CLI operations like compile, upload, and clean."""

    def __init__(self, sketch_path: Path):
        self.sketch_path = sketch_path
        self.build_path = self.sketch_path / "arduino-build"

    def build(self, fqbn: str) -> None:
        """Compiles the project."""
        commands = [
            "compile", "-v", "--fqbn", fqbn,
            "--build-path", str(self.build_path),
            str(self.sketch_path / "sketch.ino")
        ]
        run_arduino_cli(commands, self.sketch_path)

    def upload(self, fqbn: str, port: str) -> None:
        """Uploads the compiled sketch to the board."""
        commands = [
            "upload", "-v", "-p", port, "--fqbn", fqbn,
            "--input-dir", str(self.build_path)
        ]
        run_arduino_cli(commands, self.sketch_path)

    def clean(self) -> None:
        """Cleans the build artifacts."""
        commands = ["clean", "--build-path", str(self.build_path)]
        try:
            run_arduino_cli(commands, self.sketch_path)
            logging.getLogger("rich").info("Clean successful.")
        except subprocess.CalledProcessError as e:
            logging.getLogger("rich").warning(f"Clean command failed but continuing: {e}")
