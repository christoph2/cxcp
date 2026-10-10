<%!
import json
%>
${json.dumps([
    {
        "name": g.get("name"),
        "event_num": g.get("event_num", g.get("event", 0)),
        "stim": bool(g.get("stim", False)),
        "enable_timestamps": bool(g.get("enable_timestamps", False)),
        "measurements": [e for e in (measurement_entry(m, measurements_map, symbol_addresses) for m in g.get("measurements", [])) if e is not None],
        "priority": g.get("priority", 0),
        "prescaler": g.get("prescaler", 1)
    } for g in groups
], indent=4)}
