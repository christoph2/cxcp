
import ctypes
import threading

from xcp_types import (
        Xcp_ReturnType, XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType, XcpDaq_ODTEntryIntegerType,
        XcpDaq_ListConfigurationType, XcpDaq_ListStateType, XcpDaq_ODTEntryType, XcpDaq_EventType,
        XcpDaq_ProcessorStateType, XcpDaq_ProcessorType, XcpDaq_MessageType, XcpDaq_EntityType
    )


class Singleton(object):
    _lock = threading.Lock()

    def __new__(cls, *args, **kws):
        if not hasattr(cls, '_instance'):
            try:
                cls._lock.acquire()
                if not hasattr(cls, '_instance'):
                    cls._instance = super(Singleton, cls).__new__(cls)
                    cls.initialize()
            finally:
                cls._lock.release()
        return cls._instance


class Function:
    """
    """

    def __init__(self, function_name, result_type = None, arg_types = [], error_checker = None, alias = None):
        self.function_name = function_name
        self.result_type = result_type
        self.arg_types = arg_types
        self.error_checker = error_checker
        self.alias = alias = alias


class Variable:
    """
    """


def functionFactory(library, functionName, resultType, argTypes, errorChecker = None):
    if not errorChecker:
        errorChecker = defaultChecker
    func = getattr(library, functionName)
    func.restype = resultType
    func.argtypes = argTypes
    func.errcheck = errorChecker
    return func


class API:

    def __init__(self, dll):
        self.dll = dll
        self.initialize()

    def initialize(self):
        self.loadFunctions()

    def functionFactory(self, library, functionName, resultType, argTypes, errorChecker = None):
        #print(library, functionName, resultType, argTypes, errorChecker)
        #if not errorChecker:
        #    errorChecker = defaultChecker
        #print(library, type(library))
        func = getattr(library, functionName)
        func.restype = resultType
        func.argtypes = argTypes
        return func

    def loadFunctions(self):
        for fun in self.FUNCTIONS:

            inst = self.functionFactory(self.dll, fun.function_name, fun.result_type, fun.arg_types, fun.error_checker)
            setattr(self, fun.function_name, inst)
            if fun.alias:
                setattr(self, fun.alias, inst)






"""
DLL_NAME = "./test_daq.so"

dll = ctypes.CDLL(DLL_NAME)

xcp = XCP(dll)
daq = DAQ(dll)
print(xcp.Xcp_Init())
"""

