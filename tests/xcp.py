
import ctypes

from loader import API, Function, Variable

from xcp_types import (
    Xcp_ReturnType, XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType, XcpDaq_ODTEntryIntegerType,
    XcpDaq_ListConfigurationType, XcpDaq_ListStateType, XcpDaq_ODTEntryType, XcpDaq_EventType,
    XcpDaq_ProcessorStateType, XcpDaq_ProcessorType, XcpDaq_MessageType, XcpDaq_EntityType
)

class XCP(API):
    FUNCTIONS = (
        Function("Xcp_Init"),
    )

class DAQ(API):
    FUNCTIONS = (
        Function("XcpDaq_Init"),
        Function("XcpDaq_Free", Xcp_ReturnType),
        Function("XcpDaq_Alloc", Xcp_ReturnType, [XcpDaq_ListIntegerType]),
        Function("XcpDaq_AllocOdt", Xcp_ReturnType, [XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType]),
        Function("XcpDaq_AllocOdtEntry", Xcp_ReturnType, [XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType, XcpDaq_ODTEntryIntegerType]),
        Function("XcpDaq_GetListState", ctypes.POINTER(XcpDaq_ListStateType), [XcpDaq_ListIntegerType]),
        Function("XcpDaq_GetListConfiguration", ctypes.POINTER(XcpDaq_ListConfigurationType), [XcpDaq_ListIntegerType]),
        Function("XcpDaq_GetOdtEntry", ctypes.POINTER(XcpDaq_ODTEntryType), [XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType, XcpDaq_ODTEntryIntegerType]),
        Function("XcpDaq_ValidateConfiguration", ctypes.c_bool),
        Function("XcpDaq_ValidateOdtEntry", ctypes.c_bool, [XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType, XcpDaq_ODTEntryIntegerType]),
        Function("XcpDaq_MainFunction"),
        Function("XcpDaq_AddEventChannel", None, [XcpDaq_ListIntegerType, ctypes.c_uint16]),
        Function("XcpDaq_CopyMemory", None, [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint32]),
        Function("XcpDaq_GetEventConfiguration", ctypes.POINTER(XcpDaq_EventType), [ctypes.c_uint16]),
        Function("XcpDaq_TriggerEvent", None, [ctypes.c_int8]),
        Function("XcpDaq_GetProperties", None, [ctypes.POINTER(ctypes.c_int8)]),
        Function("XcpDaq_GetListCount", XcpDaq_ListIntegerType),
        Function("XcpDaq_SetProcessorState", None, [ctypes.c_int8]), # XcpDaq_ProcessorStateType
        Function("XcpDaq_StartSelectedLists"),
        Function("XcpDaq_StopSelectedLists"),
        Function("XcpDaq_StopAllLists"),
        Function("XcpDaq_GetFirstPid", ctypes.c_bool, [XcpDaq_ListIntegerType, ctypes.POINTER(XcpDaq_ODTIntegerType)]),
        Function("XcpDaq_DequeueMessage", ctypes.c_bool, [ctypes.POINTER(XcpDaq_MessageType)]),
        Function("XcpDaq_SetPointer", None, [XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType, XcpDaq_ODTEntryIntegerType]),
        Function("XcpDaq_GetCounts", None, [ctypes.POINTER(ctypes.c_uint16), ctypes.POINTER(ctypes.c_uint16), ctypes.POINTER(ctypes.c_uint16)]),
        Function("XcpDaq_TotalDynamicEntityCount", ctypes.c_uint16),
        Function("XcpDaq_GetDynamicEntities", ctypes.POINTER(XcpDaq_EntityType)),
        Function("XcpDaq_GetDynamicEntity", ctypes.POINTER(XcpDaq_EntityType), [ctypes.c_uint16]),
        Function("XcpDaq_GetDtoBuffer", ctypes.POINTER(ctypes.c_uint8)),
        #Function("", ),
        #Function("", ),
    )

"""
/*
**  Predefined DAQ constants.
*/
#if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
extern const XcpDaq_ODTEntryType XcpDaq_PredefinedOdtEntries[];
extern const XcpDaq_ODTType XcpDaq_PredefinedOdts[];
extern const XcpDaq_ListConfigurationType XcpDaq_PredefinedLists[];
extern const XcpDaq_ListIntegerType XcpDaq_PredefinedListCount;
extern XcpDaq_ListStateType XcpDaq_PredefinedListsState[];
#endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */
extern const XcpDaq_EventType XcpDaq_Events[];
"""
