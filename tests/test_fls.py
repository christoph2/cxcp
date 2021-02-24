
import pytest
import ctypes

from hypothesis import (
    assume, event, example, given, infer, note, reject, register_random, reproduce_failure, settings,
    Verbosity
)
from hypothesis.strategies import (
    booleans, integers, lists, text, characters, builds, composite, tuples, randoms
)

from loader import API, Function, Variable

DLL_NAME = "./test_fls.so"

dll = ctypes.CDLL(DLL_NAME)

"""
void FlsEmu_Init(FlsEmu_ConfigType const * config);
void FlsEmu_DeInit(void);
void FlsEmu_Close(uint8_t segmentIdx);
FlsEmu_ConfigType const * FlsEmu_GetConfig(void);
void FlsEmu_OpenCreate(uint8_t segmentIdx);
FlsEmu_ModuleStateType * FlsEmu_GetModuleState(void);
FlsEmu_OpenCreateResultType FlsEmu_OpenCreatePersitentArray(char const * fileName, uint32_t size, FlsEmu_PersistentArrayType * persistentArray);
void * FlsEmu_BasePointer(uint8_t segmentIdx);
void FlsEmu_SelectPage(uint8_t segmentIdx, uint8_t page);
uint32_t FlsEmu_NumPages(uint8_t segmentIdx);
void FlsEmu_ErasePage(uint8_t segmentIdx, uint8_t page);
void FlsEmu_EraseSector(uint8_t segmentIdx, uint32_t address);
void FlsEmu_EraseBlock(uint8_t segmentIdx, uint16_t block);
uint32_t FlsEmu_GetPageSize(void);
uint32_t FlsEmu_GetAllocationGranularity(void);
Xcp_MemoryMappingResultType FlsEmu_MemoryMapper(Xcp_MtaType * dst, Xcp_MtaType const * src);
uint32_t FlsEmu_AllocatedSize(uint8_t segmentIdx);
"""


class Flsemu(API):
    FUNCTIONS = (
        Function("FlsEmu_Init"),
        Function("FlsEmu_DeInit"),
        Function("FlsEmu_Close"),
        Function("FlsMock_Init"),
        Function("FlsMock_Deinit"),
#        Function("XcpDaq_Alloc", Xcp_ReturnType, [XcpDaq_ListIntegerType]),
#        Function("XcpDaq_AllocOdt", Xcp_ReturnType, [XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType]),
    )

def test_me():
    pass

