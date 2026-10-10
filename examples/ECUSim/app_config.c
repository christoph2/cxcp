#if defined(__linux__) && !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
#endif

#include <stdint.h>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <inttypes.h>
    #include <link.h>
    #include <stdio.h>
#endif

#include "app_config.h"
#include "xcp.h"

#if !defined(ECUSIM_ENABLE_ELF_MAPPER_TRACE)
    #define ECUSIM_ENABLE_ELF_MAPPER_TRACE XCP_OFF
#endif

#define CALRAM_SIZE     (16U * 1024U)
#define CALRAM_ADDRESS  (0xc000U)
#define FLS_SECTOR_SIZE (256U)
#define FLS_PAGE_SIZE   (16U * 1024U)

static uint8_t                    calram[CALRAM_SIZE];
static FlsEmu_PersistentArrayType ECUSim_RamBacking = { XCP_NULL, XCP_NULL, calram, 0 };

static FlsEmu_SegmentType ECUSim_PagedFlash = {
    "ECUSim_Flash", FLSEMU_KB(512), 2, FLS_SECTOR_SIZE, FLS_PAGE_SIZE, 4, 0x8000, XCP_NULL, 0, 0, FLSEMU_FLASH, 0
};

static FlsEmu_SegmentType ECUSim_Eeprom = { "ECUSim_EEPROM", FLSEMU_KB(4), 2, 4, FLSEMU_KB(4),  1,
                                            0x4000,          XCP_NULL,     0, 0, FLSEMU_EEPROM, 0 };

static FlsEmu_SegmentType ECUSim_Ram = { "ECUSim_RAM",   CALRAM_SIZE,        1, 1, CALRAM_SIZE, 1,
                                         CALRAM_ADDRESS, &ECUSim_RamBacking, 0, 0, FLSEMU_RAM,  0 };

static FlsEmu_SegmentType const *segments[] = {
    &ECUSim_PagedFlash,
    &ECUSim_Eeprom,
    &ECUSim_Ram,
};

const FlsEmu_ConfigType FlsEmu_Config = {
    3,
    (FlsEmu_SegmentType **)segments,
};

#if defined(_WIN32)
static Xcp_MemoryMappingResultType ECUSim_PeCoffMemoryMapper(Xcp_MtaType *dst, Xcp_MtaType const *src) {
    HMODULE                 moduleHandle = NULL;
    uint8_t const          *imageBase    = XCP_NULL;
    IMAGE_DOS_HEADER const *dosHeader    = XCP_NULL;
    IMAGE_NT_HEADERS const *ntHeaders    = XCP_NULL;

    uint32_t rva = (uint32_t)src->address;

    if (src->ext != 0U) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR)(uintptr_t)&ECUSim_PeCoffMemoryMapper, &moduleHandle
        )) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    imageBase = (uint8_t const *)moduleHandle;
    dosHeader = (IMAGE_DOS_HEADER const *)imageBase;
    if ((dosHeader == XCP_NULL) || (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    ntHeaders = (IMAGE_NT_HEADERS const *)(imageBase + dosHeader->e_lfanew);
    if ((ntHeaders == XCP_NULL) || (ntHeaders->Signature != IMAGE_NT_SIGNATURE)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    if (rva >= (uint32_t)ntHeaders->OptionalHeader.SizeOfImage) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    dst->address = (uintptr_t)(imageBase + rva);
    dst->ext     = src->ext;

    return XCP_MEMORY_MAPPED;
}
#elif defined(__linux__)
typedef struct {
    uintptr_t imageBase;
    uintptr_t minVaddr;
    uintptr_t maxVaddr;
    uint8_t   found;
} ECUSim_ElfImageInfoType;

    #if ECUSIM_ENABLE_ELF_MAPPER_TRACE == XCP_ON
static uint8_t ECUSim_ElfMapperDebugPrinted = 0U;
    #endif

static int ECUSim_CollectMainElfInfo(struct dl_phdr_info *info, size_t size, void *data) {
    ECUSim_ElfImageInfoType *imageInfo      = (ECUSim_ElfImageInfoType *)data;
    uintptr_t                minVaddr       = UINTPTR_MAX;
    uintptr_t                maxVaddr       = 0U;
    int                      hasLoadSegment = 0;
    uint16_t                 segmentIdx;

    (void)size;

    if ((info->dlpi_name != XCP_NULL) && (info->dlpi_name[0] != '\0')) {
        return 0;
    }

    for (segmentIdx = 0U; segmentIdx < (uint16_t)info->dlpi_phnum; ++segmentIdx) {
        ElfW(Phdr) const *programHeader = &info->dlpi_phdr[segmentIdx];

        if (programHeader->p_type != PT_LOAD) {
            continue;
        }

        if ((uintptr_t)programHeader->p_vaddr < minVaddr) {
            minVaddr = (uintptr_t)programHeader->p_vaddr;
        }

        if ((uintptr_t)programHeader->p_vaddr + (uintptr_t)programHeader->p_memsz > maxVaddr) {
            maxVaddr = (uintptr_t)programHeader->p_vaddr + (uintptr_t)programHeader->p_memsz;
        }

        hasLoadSegment = 1;
    }

    if (hasLoadSegment == 0) {
        return 0;
    }

    imageInfo->imageBase = (uintptr_t)info->dlpi_addr;
    imageInfo->minVaddr  = minVaddr;
    imageInfo->maxVaddr  = maxVaddr;
    imageInfo->found     = 1U;

    return 1;
}

static Xcp_MemoryMappingResultType ECUSim_ElfMemoryMapper(Xcp_MtaType *dst, Xcp_MtaType const *src) {
    ECUSim_ElfImageInfoType imageInfo = { 0U, 0U, 0U, 0U };
    uintptr_t               symbolAddress;

    if (src->ext != 0U) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    (void)dl_iterate_phdr(ECUSim_CollectMainElfInfo, &imageInfo);
    if (imageInfo.found == 0U) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    symbolAddress = (uintptr_t)src->address;
    if ((symbolAddress < imageInfo.minVaddr) || (symbolAddress >= imageInfo.maxVaddr)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    if (symbolAddress > (UINTPTR_MAX - imageInfo.imageBase)) {
        return XCP_MEMORY_NOT_MAPPED;
    }

    dst->address = imageInfo.imageBase + symbolAddress;
    dst->ext     = src->ext;

    #if ECUSIM_ENABLE_ELF_MAPPER_TRACE == XCP_ON
    if (ECUSim_ElfMapperDebugPrinted == 0U) {
        fprintf(
            stderr, "[XCP] ELF mapper: elf=0x%" PRIxPTR ", base=0x%" PRIxPTR ", runtime=0x%" PRIxPTR "\n", symbolAddress,
            imageInfo.imageBase, (uintptr_t)dst->address
        );
        ECUSim_ElfMapperDebugPrinted = 1U;
    }
    #endif

    return XCP_MEMORY_MAPPED;
}
#endif

Xcp_MemoryMappingResultType Xcp_HookFunction_AddressMapper(Xcp_MtaType *dst, Xcp_MtaType const *src) {
    Xcp_MemoryMappingResultType result = FlsEmu_MemoryMapper(dst, src);

    if (result != XCP_MEMORY_NOT_MAPPED) {
        return result;
    }

#if defined(_WIN32)
    return ECUSim_PeCoffMemoryMapper(dst, src);
#elif defined(__linux__)
    return ECUSim_ElfMemoryMapper(dst, src);
#else
    return XCP_MEMORY_NOT_MAPPED;
#endif
}
