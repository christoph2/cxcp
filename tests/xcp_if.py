
import ctypes

from loader import API, Function, Variable

from xcp_types import (
    Xcp_ReturnType, XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType, XcpDaq_ODTEntryIntegerType,
    XcpDaq_ListConfigurationType, XcpDaq_ListStateType, XcpDaq_ODTEntryType, XcpDaq_EventType,
    XcpDaq_ProcessorStateType, XcpDaq_ProcessorType, XcpDaq_MessageType, XcpDaq_EntityType,
    XcpDaq_QueueType
)


class XCP(API):
    FUNCTIONS = (
        Function("Xcp_Init"),

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
        Function("XcpDaq_QueueInit"),
        Function("XcpDaq_QueueFull", ctypes.c_bool),
        Function("XcpDaq_QueueEmpty", ctypes.c_bool),
        Function("XcpDaq_QueueEnqueue", ctypes.c_bool, [ctypes.c_uint16, ctypes.POINTER(ctypes.c_uint8)]),
        Function("XcpDaq_QueueDequeue", ctypes.c_bool, [ctypes.POINTER(ctypes.c_uint16), ctypes.POINTER(ctypes.c_uint8)]),
        Function("XcpDaq_QueueGetVar", None, [ctypes.POINTER(XcpDaq_QueueType)]),
    )

    def get_daq_counts(self):
        entityCount = ctypes.c_uint16()
        listCount = ctypes.c_uint16()
        odtCount = ctypes.c_uint16()
        self.XcpDaq_GetCounts(ctypes.byref(entityCount), ctypes.byref(listCount), ctypes.byref(odtCount))
        return (entityCount.value, listCount.value, odtCount.value, )

    def get_daq_queue_var(self):
        queue = XcpDaq_QueueType()
        self.XcpDaq_QueueGetVar(ctypes.byref(queue))
        return queue

    def daq_enqueue(self, data: bytes):
        u8_array = ctypes.c_uint8 * len(data)
        return self.XcpDaq_QueueEnqueue(len(data), u8_array.from_buffer(bytearray(data)))

    def daq_dequeue(self):
        length = ctypes.c_uint16()
        data = (ctypes.c_uint8 * 256)()
        res = self.XcpDaq_QueueDequeue(ctypes.byref(length), data)
        return res, bytes(data[ : length.value])
