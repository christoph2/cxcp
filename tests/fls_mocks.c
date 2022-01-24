
#include <assert.h>
#include <string.h>

#include "flsemu.h"

#define FLS_SECTOR_SIZE (256)

#define FLS_PAGE_ADDR           ((uint16_t)0x8000U)
#define FLS_PAGE_SIZE           ((uint32_t)0x1000U)

static FlsEmu_SegmentType Mock_PagedFlash = {
    "Mock_Flash",
    FLSEMU_KB(32),
    2,
    FLS_SECTOR_SIZE,
    FLS_PAGE_SIZE,
    2,
    0x8000,
    XCP_NULL,
    0,
    0,
};

static FlsEmu_SegmentType const * segments[] = {
    &Mock_PagedFlash,
};

static const FlsEmu_ConfigType FlsEmu_Config = {
    1,
    (FlsEmu_SegmentType**)segments,
};


uint32_t FlsEmu_GetAllocationGranularity(void)
{
    return 4096;
}

void FlsEmu_Close(uint8_t segmentIdx)
{

}


FlsEmu_OpenCreateResultType FlsEmu_OpenCreatePersitentArray(char const * fileName, uint32_t size, FlsEmu_PersistentArrayType * persistentArray)
{

}


void FlsEmu_SelectPage(uint8_t segmentIdx, uint8_t page)
{

}

void FlsMock_Init(void)
{
    FlsEmu_Init(&FlsEmu_Config);
}

void FlsMock_Deinit(void)
{
    FlsEmu_DeInit();
}
