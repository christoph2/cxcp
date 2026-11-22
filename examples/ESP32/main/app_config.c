#include <stdint.h>

#include "esp_random.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include "app_config.h"
#include "xcp.h"

#define CALRAM_SIZE     (16U * 1024U)
#define CALRAM_ADDRESS  (0xc000U)

static uint8_t calram[CALRAM_SIZE];
static FlsEmu_PersistentArrayType ECUSim_RamBacking = {XCP_NULL, XCP_NULL, calram, 0};

static FlsEmu_SegmentType ECUSim_Ram = {
    "ECUSim_RAM", CALRAM_SIZE, 1, 1, CALRAM_SIZE, 1, CALRAM_ADDRESS, &ECUSim_RamBacking, 0, 0, FLSEMU_RAM, 0
};

static FlsEmu_SegmentType const *segments[] = {
    &ECUSim_Ram,
};

const FlsEmu_ConfigType FlsEmu_Config = {
    1,
    (FlsEmu_SegmentType **) segments,
};

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
XCP_DAQ_BEGIN_EVENTS
    XCP_DAQ_DEFINE_EVENT(
        "EVT 10ms", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 10
    ),
    XCP_DAQ_DEFINE_EVENT(
        "EVT sporadic", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 0
    ),
    XCP_DAQ_DEFINE_EVENT(
        "EVT 100ms", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 100
    ),
XCP_DAQ_END_EVENTS
#endif

#if defined(_WIN32)
static Xcp_MemoryMappingResultType ECUSim_PeCoffMemoryMapper(Xcp_MtaType *dst, Xcp_MtaType const *src) {
    HMODULE moduleHandle = NULL;
    uint8_t const *imageBase = XCP_NULL;
    IMAGE_DOS_HEADER const *dosHeader = XCP_NULL;
    IMAGE_NT_HEADERS const *ntHeaders = XCP_NULL;

    uint32_t rva = (uint32_t) src->address;

    if (src->ext != 0U) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    if (!GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR) (uintptr_t) &ECUSim_PeCoffMemoryMapper, &moduleHandle
    )) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    imageBase = (uint8_t const *) moduleHandle;
    dosHeader = (IMAGE_DOS_HEADER const *) imageBase;
    if ((dosHeader == XCP_NULL) || (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    ntHeaders = (IMAGE_NT_HEADERS const *) (imageBase + dosHeader->e_lfanew);
    if ((ntHeaders == XCP_NULL) || (ntHeaders->Signature != IMAGE_NT_SIGNATURE)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    if (rva >= (uint32_t) ntHeaders->OptionalHeader.SizeOfImage) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    dst->address = (uintptr_t) (imageBase + rva);
    dst->ext = src->ext;

    return XCP_MEMORY_MAPPED;
}
#endif

Xcp_MemoryMappingResultType Xcp_HookFunction_AddressMapper(Xcp_MtaType *dst, Xcp_MtaType const *src) {
    if (!FlsEmu_Initialized()) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    Xcp_MemoryMappingResultType result = FlsEmu_MemoryMapper(dst, src);

    if (result != XCP_MEMORY_NOT_MAPPED) {
        return result;
    }

#if defined(_WIN32)
    return ECUSim_PeCoffMemoryMapper(dst, src);
#else
    return XCP_MEMORY_NOT_MAPPED;
#endif
}

bool Xcp_HookFunction_CheckMemoryAccess(Xcp_MtaType mta, uint32_t length, Xcp_MemoryAccessType access, bool programming) {
    FlsEmu_ConfigType const *config = FlsEmu_GetConfig();
    uint8_t                  idx    = 0;

    (void)access;
    (void)programming;

    if (!FlsEmu_Initialized() || (config == XCP_NULL) || (config->segments == XCP_NULL)) {
        return false;
    }

    for (idx = 0; idx < config->numSegments; ++idx) {
        FlsEmu_SegmentType const *segment = config->segments[idx];
        uint64_t                  offset;
        uint64_t                  segmentSize;

        if (segment == XCP_NULL) {
            continue;
        }
        if (mta.address < segment->baseAddress) {
            continue;
        }
        if (segment->type != FLSEMU_RAM) {
            continue;
        }

        offset = (uint64_t)(mta.address - segment->baseAddress);
        if (mta.ext != segment->addressExtension) {
            continue;
        }
        segmentSize = segment->memSize;

        if ((offset < segmentSize) && ((uint64_t)length <= (segmentSize - offset))) {
            return true;
        }
    }

    return false;
}

bool Xcp_HookFunction_GetSeed(uint8_t resource, Xcp_1DArrayType *result) {
    static uint8_t seed[4];

    (void)resource;
    esp_fill_random(seed, sizeof(seed));
    result->length = (uint8_t)sizeof(seed);
    result->data   = seed;
    return true;
}

bool Xcp_HookFunction_Unlock(uint8_t resource, Xcp_1DArrayType const *key) {
    (void)resource;
    (void)key;
    return false;
}
