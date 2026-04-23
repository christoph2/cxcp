import json
import logging
from pathlib import Path
from typing import Any, Dict, Union

from .utils import sha256_hash_of_file


class ProjectState:
    """Manages the project's state, including file hashes and board selection."""

    def __init__(self, state_file: Path):
        self.logger = logging.getLogger("rich")
        self.state_file = state_file
        self.state: Dict[str, Any] = {"hashes": {}, "boards": {"selected": {}, "available": {}}}
        self.load()

    @property
    def hashes(self) -> Dict[str, str]:
        return self.state.get("hashes", {})

    @property
    def boards(self) -> Dict[str, Any]:
        return self.state.get("boards", {})

    def load(self) -> None:
        if self.state_file.exists():
            try:
                with self.state_file.open("r", encoding="utf-8") as f:
                    self.state = json.load(f)
            except (json.JSONDecodeError, IOError) as e:
                self.logger.warning(f"Could not load state file {self.state_file}: {e}. Starting with fresh state.")
                self.state = {"hashes": {}, "boards": {"selected": {}, "available": {}}}

    def save(self) -> None:
        try:
            self.state_file.parent.mkdir(parents=True, exist_ok=True)
            with self.state_file.open("w", encoding="utf-8") as f:
                json.dump(self.state, f, indent=4)
        except IOError as e:
            self.logger.error(f"Failed to save state file {self.state_file}: {e}")

    def file_is_modified(self, file_path: Path) -> bool:
        """Checks if a file has been modified since the last hash was stored."""
        current_hash = sha256_hash_of_file(file_path)
        stored_hash = self.hashes.get(str(file_path))
        return current_hash is None or stored_hash is None or stored_hash != current_hash

    def update_hash(self, file_path: Path) -> None:
        """Updates the stored hash for a file."""
        new_hash = sha256_hash_of_file(file_path)
        if new_hash:
            self.hashes[str(file_path)] = new_hash
