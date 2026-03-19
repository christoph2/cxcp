import pytest
from pyxcp_appgen import utils


def test_validate_events_no_events():
    """Test with no events, should return a default event."""
    assert utils.validate_events(None) == [{"name": "default_event", "number": 0, "interval": 10}]
    assert utils.validate_events([]) == [{"name": "default_event", "number": 0, "interval": 10}]


def test_validate_events_valid_events():
    """Test with a list of valid events."""
    events = [{"name": "e1", "number": 1}, {"name": "e2", "number": 0}]
    assert utils.validate_events(events) == events


def test_validate_events_auto_numbering():
    """Test automatic numbering of events."""
    events = [{"name": "e1"}, {"name": "e2", "number": 5}, {"name": "e3"}]
    expected = [
        {"name": "e1", "number": 0},
        {"name": "e2", "number": 5},
        {"name": "e3", "number": 1},
    ]
    assert utils.validate_events(events) == expected


def test_validate_events_duplicate_name():
    """Test for duplicate event names, should raise ValueError."""
    events = [{"name": "e1"}, {"name": "e1"}]
    with pytest.raises(ValueError, match="Duplicate event name: e1"):
        utils.validate_events(events)


def test_validate_events_duplicate_number():
    """Test for duplicate event numbers, should raise ValueError."""
    events = [{"name": "e1", "number": 1}, {"name": "e2", "number": 1}]
    with pytest.raises(ValueError, match="Duplicate event number: 1"):
        utils.validate_events(events)


def test_validate_events_missing_name():
    """Test for an event with a missing name, should raise ValueError."""
    events = [{"number": 0}]
    with pytest.raises(ValueError, match="Event name is required"):
        utils.validate_events(events)


def test_validate_mod_common_none():
    """Test with no mod_common, should return full defaults."""
    result = utils.validate_mod_common(None)
    assert "byte_order" in result
    assert result["alignment_byte"] == 1


def test_validate_mod_common_partial():
    """Test with a partial mod_common, should fill in missing values."""
    mod_common = {"byte_order": "MSB_FIRST"}
    result = utils.validate_mod_common(mod_common)
    assert result["byte_order"] == "MSB_FIRST"
    assert result["alignment_word"] == 2


def test_get_initializer():
    """Test the get_initializer mapping."""
    assert utils.INITIALIZERS.get("uint32") == "0UL"
    assert utils.INITIALIZERS.get("float") == "0.0f"
    assert utils.INITIALIZERS.get("invalid_type") is None


def test_c_to_asam_type():
    """Test the type mapping for A2L."""
    assert utils.TYPES.get("uint8") == "UBYTE"
    assert utils.TYPES.get("double") == "FLOAT64_IEEE"
    assert utils.TYPES.get("invalid_type") is None
