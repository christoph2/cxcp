import logging
from pathlib import Path
from typing import Any, Dict, List, Optional, Union

from mako.exceptions import text_error_template
from mako.lookup import TemplateLookup

from . import utils


class Renderer:
    """Handles rendering of Mako templates for source code and A2L files."""

    def __init__(self, templates_dir: str, data_model: Dict[str, Any], symbol_addresses: Dict[str, int]):
        self.logger = logging.getLogger("rich")
        self.lookup = TemplateLookup(
            directories=[templates_dir],
            input_encoding="utf-8",
            output_encoding="utf-8",
        )
        self.data_model = data_model
        self.symbol_addresses = symbol_addresses

    def _render_to_file(self, template_name: str, out_path: Path, **context: Any) -> None:
        try:
            tpl = self.lookup.get_template(template_name)
            render_ctx = {
                "project": self.data_model.get("project"),
                "c_to_asam_type": self.c_to_asam_type,
                "get_initializer": self.get_initializer,
                "symbol_addresses": self.symbol_addresses,
                **context,
            }
            rendered = tpl.render(**render_ctx)
        except Exception:
            self.logger.error(text_error_template().render())
            raise

        out_path.parent.mkdir(parents=True, exist_ok=True)
        if isinstance(rendered, bytes):
            rendered = rendered.decode("utf-8")
        with open(out_path, "w", encoding="utf-8", newline="\n") as f:
            f.write(rendered)
        self.logger.info(f"Wrote rendered template to {out_path}")

    def render_sketch(self, sketch_ino_path: Path) -> None:
        self._render_to_file("arduino_src.mako", sketch_ino_path)

    def render_a2l(self, a2l_file_path: Path) -> None:
        self._render_to_file("module.mako", a2l_file_path)

    def render_xcp_rec_config(self, config_path: Path) -> None:
        groups = self.get_groups()
        measurements = self.get_measurements()
        measurements_map = {m.get("name"): m for m in measurements if m.get("name")}
        self._render_to_file(
            "groups_measurements.mako",
            config_path,
            groups=groups,
            measurements_map=measurements_map,
            measurement_entry=self.measurement_entry,
        )

    def get_measurements(self) -> list:
        return self.data_model.get("project", {}).get("module", {}).get("measurements", [])

    def get_groups(self) -> list:
        defaults = {"priority": 0, "prescaler": 1, "stim": False, "enable_timestamps": False}
        groups = self.data_model.get("project", {}).get("module", {}).get("groups", [])
        for group in groups:
            for k, v in defaults.items():
                group.setdefault(k, v)
        return groups

    @staticmethod
    def c_to_asam_type(tp: Optional[str]) -> str:
        return utils.TYPES.get(tp, "<none>") if tp else "<none>"

    @staticmethod
    def get_initializer(measurement: Dict[str, Any]) -> str:
        tp = measurement.get("type")
        return utils.INITIALIZERS.get(tp, "0") if tp else "0"

    @staticmethod
    def measurement_entry(item: Union[str, Dict[str, Any]], measurements_map: Dict[str, Dict[str, Any]],
                          symbol_addresses: Dict[str, int]) -> Optional[List[Any]]:
        name = item if isinstance(item, str) else item.get("name")
        if not name:
            return None
        meas = measurements_map.get(name, {})
        addr = symbol_addresses.get(name)
        tp = meas.get("type")
        code = utils.TYPE_MAP_A2L.get(tp, "none")
        return [name, addr, 0, code]
