#include "app_config.h"

uint32_t FlsEmu_GetAllocationGranularity(void) {
    return 1U;
}

void FlsEmu_Close(uint8_t segmentIdx) {
    (void)segmentIdx;
}

FlsEmu_OpenCreateResultType FlsEmu_OpenCreatePersitentArray(
    char const *fileName, uint32_t size, FlsEmu_PersistentArrayType *persistentArray
) {
    (void)fileName;
    (void)size;
    (void)persistentArray;
    return OPEN_ERROR;
}

void FlsEmu_SelectPage(uint8_t segmentIdx, uint8_t page) {
    FlsEmu_ConfigType const *config = FlsEmu_GetConfig();

    if (!FlsEmu_Initialized() || (config == XCP_NULL) || (segmentIdx >= config->numSegments)) {
        return;
    }
    if (page < FlsEmu_NumPages(segmentIdx)) {
        config->segments[segmentIdx]->currentPage = page;
    }
}