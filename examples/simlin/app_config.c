
#include "xcp.h"
#include "app_config.h"

#define CALRAM_SIZE (16 * 1024)

uint8_t calram[CALRAM_SIZE];

triangle_type triangle = {0};
uint16_t randomValue;

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

const FlsEmu_ConfigType FlsEmu_Config = {
    2,
    (FlsEmu_SegmentType**)segments,
};
const XcpDaq_ODTEntryType XcpDaq_PredefinedOdtEntries[] = {
    XCP_DAQ_DEFINE_ODT_ENTRY(triangle),
    XCP_DAQ_DEFINE_ODT_ENTRY(randomValue),
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

/*
 * Customization Functions.
 *
 */
static  uint8_t unlock_key[4] = {0};

#define KEY_INITIAL (0xBC)


bool Xcp_HookFunction_GetSeed(uint8_t resource, Xcp_1DArrayType * result)
{
    uint32_t ts;
    static  uint8_t seed[4] = {0};
    result->length = 4;
    result->data = (uint8_t*)&seed;

    ts = XcpHw_GetTimerCounter();
    seed[0] = XCP_HIWORD(XCP_HIBYTE(ts));
    seed[1] = XCP_HIWORD(XCP_LOBYTE(ts));
    seed[2] = XCP_LOWORD(XCP_HIBYTE(ts));
    seed[3] = XCP_LOWORD(XCP_LOBYTE(ts));
    unlock_key[0] = (seed[0] + seed[3]) ^ KEY_INITIAL;
    unlock_key[1] = seed[1] ^ unlock_key[0];
    unlock_key[2] = seed[2] ^ unlock_key[1];
    unlock_key[3] = seed[3] ^ unlock_key[2];

    return XCP_TRUE;
}


bool Xcp_HookFunction_Unlock(uint8_t resource, Xcp_1DArrayType const * key)
{
    return XcpUtl_MemCmp(&unlock_key, key->data, 4);
}


bool Xcp_HookFunction_CheckMemoryAccess(Xcp_MtaType mta, uint32_t length, Xcp_MemoryAccessType access, bool programming)
{
    return XCP_TRUE;
}

#if 0
Xcp_MemoryMappingResultType Xcp_HookFunction_AddressMapper(Xcp_MtaType * dst, Xcp_MtaType const * src)
{
//    return FlsEmu_MemoryMapper(dst, src);
    if ((src->address >= 0x4000) && (src->address < 0x8000)) {
        dst->address = (uint32_t)(calram - 0x4000) + src->address;
        dst->ext = src->ext;
    }
    return XCP_MEMORY_NOT_MAPPED;
}
#endif

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
