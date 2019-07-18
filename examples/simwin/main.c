
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "xcp.h"
#include "xcp_hw.h"
#include "flsemu.h"

void XcpOnCan_Init(void);

#define SIZE    (4096)
uint8_t puffer[SIZE];

void XcpHw_MainFunction(bool * finished);
void XcpTl_SetOptions(XcpHw_OptionsType const * options);

void AppTask(void);

#define FLS_SECTOR_SIZE (256)

#define FLS_PAGE_ADDR           ((uint16_t)0x8000U)
#define FLS_PAGE_SIZE           ((uint32_t)0x4000U)


static FlsEmu_SegmentType S12D512_PagedFlash = {
    "XCPSIM_Flash",
    FLSEMU_KB(512),
    2,
    FLS_SECTOR_SIZE,
    FLS_PAGE_SIZE,
    4,
    0x8000,
    XCP_NULL,
    0,
};

static FlsEmu_SegmentType S12D512_EEPROM = {
    "XCPSIM_EEPROM",
    FLSEMU_KB(4),
    2,
    4,
    FLSEMU_KB(4),
    1,
    0x4000,
    XCP_NULL,
    0,
};


static FlsEmu_SegmentType const * segments[] = {
    &S12D512_PagedFlash,
    &S12D512_EEPROM,
};

static const FlsEmu_ConfigType FlsEmu_Config = {
    2,
    (FlsEmu_SegmentType**)segments,
};

#if 0
void memimnfo(void * ptr)
{
    /*
    ** mainly a debugging/bug reporting aid.
    */

    MEMORY_BASIC_INFORMATION info;
    SYSTEM_INFO si;
    size_t res;

    GetSystemInfo(&si);
    res = VirtualQuery(ptr, &info, sizeof(MEMORY_BASIC_INFORMATION));
    if (!res) {
        Win_ErrorMsg("memimnfo::VirtualQuery()", GetLastError());
    } else {
        printf("%p %p %d %08x %08x %08x\n", info.BaseAddress, info.AllocationBase, info.RegionSize / 1024,
               info.AllocationProtect, info.Protect, info.Type);
    }
}
#endif


int main(void)
{
    bool finished;
    XcpHw_OptionsType options;

    XcpHw_GetCommandLineOptions(&options);
    XcpTl_SetOptions(&options);

    FlsEmu_Init(&FlsEmu_Config);

    Xcp_Init();
    XcpTl_DisplayInfo();

    srand(23);

    for (;;) {
        Xcp_MainFunction();
        XcpTl_MainFunction();

        XcpHw_MainFunction(&finished);
        if (finished) {
            break;
        }
        AppTask();
        XcpDaq_MainFunction();
    }

    FlsEmu_DeInit();

    XcpTl_DeInit();

    return 0;
}


bool Xcp_HookFunction_GetSeed(uint8_t resource, Xcp_1DArrayType * result)
{
    static const uint8_t seed[] = {0x11, 0x22, 0x33, 0x44};
    result->length = 4;
    result->data = (uint8_t*)&seed;

    return XCP_TRUE;
}


bool Xcp_HookFunction_Unlock(uint8_t resource, Xcp_1DArrayType const * key)
{
    static const uint8_t secret[] = {0x55, 0x66, 0x77, 0x88};

//    printf("\tKEY [%u]: ", key->length);
//    Xcp_Hexdump(key->data, key->length);

    return XcpUtl_MemCmp(&secret, key->data, XCP_ARRAY_SIZE(secret));
}


bool Xcp_HookFunction_CheckMemoryAccess(Xcp_MtaType mta, Xcp_MemoryAccessType access, bool programming)
{
    return XCP_TRUE;
}

Xcp_MemoryMappingResultType Xcp_HookFunction_AddressMapper(Xcp_MtaType * dst, Xcp_MtaType const * src)
{
    return FlsEmu_MemoryMapper(dst, src);
}

#if 0
bool Xcp_HookFunction_GetId(uint8_t idType)
{
    if (idType == 4) {
        Xcp_SetMta(Xcp_GetNonPagedAddress(a2lFile.view.mappingAddress));
        Xcp_Send8(8, 0xff, 0, 0, 0, XCP_LOBYTE(XCP_LOWORD(a2lFile.size)), XCP_HIBYTE(XCP_LOWORD(a2lFile.size)), XCP_LOBYTE(XCP_HIWORD(a2lFile.size)), XCP_HIBYTE(XCP_HIWORD(a2lFile.size)));
        return XCP_TRUE;
    }
    return XCP_FALSE;
}
#endif // 0

typedef struct {
    uint8_t value;
    bool down;
} triangle_type;

triangle_type triangle = {0};
uint16_t randomValue;

/////////////////////////
/////////////////////////
/////////////////////////

const XcpDaq_ODTEntryType XcpDaq_PredefinedOdtEntries[] = {
    {
        (uint32_t)&triangle, sizeof(triangle_type)
    },
    {
        (uint32_t)&randomValue, sizeof(uint16_t)
    },
};


const XcpDaq_ODTType XcpDaq_PredefinedOdts[] = {
    {
        2, 0
    }
};


const XcpDaq_ListConfigurationType XcpDaq_PredefinedLists[] = {
    {
        1, 0
    }
};

#if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
XcpDaq_ListStateType XcpDaq_PredefinedListsState[XCP_DAQ_PREDEFINDED_LIST_COUNT];
const XcpDaq_ListIntegerType XcpDaq_PredefinedListCount = XCP_DAQ_PREDEFINDED_LIST_COUNT;
#endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */


XCP_DAQ_BEGIN_EVENTS
    XCP_DAQ_DEFINE_EVENT("EVT 100ms",
        XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS,
        100
    ),
    XCP_DAQ_DEFINE_EVENT("EVT sporadic",
        XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS,
        0
    ),
    XCP_DAQ_DEFINE_EVENT("EVT 10ms",
        XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS,
        10
    ),
XCP_DAQ_END_EVENTS

/////////////////////////
/////////////////////////
/////////////////////////

void AppTask(void)
{
    static uint32_t currentTS = 0UL;
    static uint32_t previousTS = 0UL;
    static uint16_t ticker = 0;

    currentTS = XcpHw_GetTimerCounter() / 1000;
    if (currentTS >= (previousTS + 10)) {
        //printf("T [%u::%lu]\n", currentTS, XcpHw_GetTimerCounter() / 1000);

        if ((ticker % 3) == 0) {
            if (triangle.down) {
                triangle.value--;
                if (triangle.value == 0) {
                    triangle.down = XCP_FALSE;
                }
            } else {
                triangle.value++;
                if (triangle.value >= 75) {
                    triangle.down = XCP_TRUE;
                }
            }
            randomValue = (uint16_t)rand();

            //printf("\t\t\tTRI [%u]\n", triangle.value);
            XcpDaq_TriggerEvent(1);
        }
        ticker++;
        previousTS =  XcpHw_GetTimerCounter() / 1000;
    }
}
