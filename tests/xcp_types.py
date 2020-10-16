
import ctypes
import enum

class StructureWithEnums(ctypes.Structure):
    """Add missing enum feature to ctypes Structures.
    """
    _map = {}

    def __getattribute__(self, name):
        _map = ctypes.Structure.__getattribute__(self, '_map')
        value = ctypes.Structure.__getattribute__(self, name)
        if name in _map:
            EnumClass = _map[name]
            if isinstance(value, ctypes.Array):
                return [EnumClass(x) for x in value]
            else:
                return EnumClass(value)
        else:
            return value

    def __str__(self):
        result = []
        result.append("struct {0} {{".format(self.__class__.__name__))
        for field in self._fields_:
            attr, attrType = field
            if attr in self._map:
                attrType = self._map[attr]
            value = getattr(self, attr)
            result.append("    {0} [{1}] = {2!r};".format(attr, attrType.__name__, value))
        result.append("};")
        return '\n'.join(result)

    __repr__ = __str__

class Xcp_ReturnType(enum.IntEnum):
    ERR_CMD_SYNCH           = 0x00

    ERR_CMD_BUSY            = 0x10
    ERR_DAQ_ACTIVE          = 0x11
    ERR_PGM_ACTIVE          = 0x12

    ERR_CMD_UNKNOWN         = 0x20
    ERR_CMD_SYNTAX          = 0x21
    ERR_OUT_OF_RANGE        = 0x22
    ERR_WRITE_PROTECTED     = 0x23
    ERR_ACCESS_DENIED       = 0x24
    ERR_ACCESS_LOCKED       = 0x25
    ERR_PAGE_NOT_VALID      = 0x26
    ERR_MODE_NOT_VALID      = 0x27
    ERR_SEGMENT_NOT_VALID   = 0x28
    ERR_SEQUENCE            = 0x29
    ERR_DAQ_CONFIG          = 0x2A

    ERR_MEMORY_OVERFLOW     = 0x30
    ERR_GENERIC             = 0x31
    ERR_VERIFY              = 0x32
    ERR_RESOURCE_TEMPORARY_NOT_ACCESSIBLE = 0x33

    ERR_SUCCESS             = 0xff

class XcpDaq_EntityKindType(enum.IntEnum):
    XCP_ENTITY_UNUSED       = 0
    XCP_ENTITY_DAQ_LIST     = 1
    XCP_ENTITY_ODT          = 2
    XCP_ENTITY_ODT_ENTRY    = 3

class XcpDaq_MtaType(ctypes.Structure):
    _fields_ = [
        ("address", ctypes.c_uint32),
    ]

class XcpDaq_ODTEntryType(ctypes.Structure):
    _fields_ = [
        ("mta", XcpDaq_MtaType),
        ("length", ctypes.c_int32),
    ]

class XcpDaq_ODTType(ctypes.Structure):
    _fields_ = [
        ("numOdtEntries", ctypes.c_uint8),
        ("firstOdtEntry", ctypes.c_uint16),
    ]

class XcpDaq_DynamicListType(ctypes.Structure):
    _fields_ = [
        ("numOdts", ctypes.c_uint8),
        ("firstOdt", ctypes.c_uint16),
        ("mode", ctypes.c_uint8),
    ]

class XcpDaq_EntityUnionType(ctypes.Structure):
    _fields_ = [
        ("odtEntry", XcpDaq_ODTEntryType),
        ("odt", XcpDaq_ODTType),
        ("daqList", XcpDaq_DynamicListType),
    ]

class XcpDaq_EntityType(ctypes.Structure):
    _fields_ = [
        ("kind", ctypes.c_uint8),
        ("entity", XcpDaq_EntityUnionType),
    ]

class XcpDaq_ListMode(enum.IntEnum):
    XCP_DAQ_LIST_MODE_ALTERNATING =  0x01
    XCP_DAQ_LIST_MODE_DIRECTION   =  0x02
    XCP_DAQ_LIST_MODE_TIMESTAMP   =  0x10
    XCP_DAQ_LIST_MODE_PID_OFF     =  0x20
    XCP_DAQ_LIST_MODE_SELECTED    =  0x40
    XCP_DAQ_LIST_MODE_STARTED     =  0x80


XcpDaq_ListIntegerType = ctypes.c_uint8
XcpDaq_ODTIntegerType = ctypes.c_uint8
XcpDaq_ODTEntryIntegerType = ctypes.c_uint8


class XcpDaq_ListConfigurationType(ctypes.Structure):
    _fields_ = [
        ("numOdts", XcpDaq_ODTIntegerType),
        ("firstOdt", ctypes.c_uint16),
    ]


class XcpDaq_ListStateType(ctypes.Structure):
    _fields_ = [
        ("mode", ctypes.c_uint8),
#        ("prescaler", ctypes.c_uint8),
#        ("counter", ctypes.c_uint8),
    ]

class XcpDaq_EventType(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char_p),
        ("nameLen", ctypes.c_uint8),
        ("properties", ctypes.c_uint8),
        ("timeunit", ctypes.c_uint8),
        ("cycle", ctypes.c_uint8),
        #("priority", ctypes.c_uint8),

    ]

class XcpDaq_ProcessorStateType(enum.IntEnum):
    XCP_DAQ_STATE_UNINIT            = 0
    XCP_DAQ_STATE_CONFIG_INVALID    = 1
    XCP_DAQ_STATE_CONFIG_VALID      = 2
    XCP_DAQ_STATE_STOPPED           = 3
    XCP_DAQ_STATE_RUNNING           = 4

class XcpDaq_ProcessorType(ctypes.Structure):
    _fields_ = [
        #("state", XcpDaq_ProcessorStateType),
        ("state", ctypes.c_uint8)
    ]

class XcpDaq_MessageType(ctypes.Structure):
    _fields_ = [
        ("dlc", ctypes.c_uint8),
        ("data", ctypes.POINTER(ctypes.c_uint8)),
    ]

class XcpDaq_PointerType(ctypes.Structure):
    _fields_ = [
        ("daqList", XcpDaq_ListIntegerType),
        ("odt", XcpDaq_ODTIntegerType),
        ("odtEntry", XcpDaq_ODTEntryIntegerType),
    ]
