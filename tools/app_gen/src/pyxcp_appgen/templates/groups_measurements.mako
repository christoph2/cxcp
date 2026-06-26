<%!
import json

def format_data(groups, measurement_entry, measurements_map, symbol_addresses):
	daq_lists = [
	    {
	        "name": g.get("name"),
	        "event_num": g.get("event_num", g.get("event", 0)),
	        "stim": bool(g.get("stim", False)),
	        "enable_timestamps": bool(g.get("enable_timestamps", False)),
	        "measurements": [e for e in (measurement_entry(m, measurements_map, symbol_addresses) for m in g.get("measurements", [])) if e is not None],
	        "priority": g.get("priority", 0),
	        "prescaler": g.get("prescaler", 1)
	    } for g in groups
	]
	body = {
		"daq_lists": daq_lists,
		"type": "xmraw",
		"runtime_seconds": 900,
	}
	return json.dumps(body, indent=4)
%>
${format_data(groups, measurement_entry, measurements_map, symbol_addresses)}
