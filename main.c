
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "xcp.h"
#include "xcp_util.h"
#include "flsemu.h"


// For demo purpopses, create an extra constant data section whose name is exactly 8 bytes long (the max)
#pragma const_seg(".t_const") // begin allocating const data in a new section whose name is 8 bytes long (the max)
const char const_string1[] = "This string is allocated in a special const data segment named \".t_const\".";
#pragma const_seg() // resume allocating const data in the normal .rdata section



#pragma bss_seg(push, /*stack1,*/ ".arbeitsseite")
int in_der_arbeitsseite;

#pragma bss_seg(pop/*, stack1*/)
int bss_normal;

void XcpOnCan_Init(void);

#define SIZE    (4096)
uint8_t puffer[SIZE];

void XcpHw_MainFunction(bool * finished);

void AppTask(void);

#define FLS_SECTOR_SIZE (256)
//#define FLS_PAGE_SIZE   (4 * 1024)

#define FLS_PAGE_ADDR           ((uint16_t)0x8000U)
#define FLS_PAGE_SIZE           ((uint32_t)0x10000U)  /* NB: MUST MATCH system allocation granularity!!! */


static FlsEmu_SegmentType S12D512_PagedFlash = {
    "XCPSIM_Flash",
    FLSEMU_KB(512),
    2,
    FLS_SECTOR_SIZE,
    FLS_PAGE_SIZE,
    4,
    0x8000,
};

static FlsEmu_SegmentType S12D512_EEPROM = {
    "XCPSIM_EEPROM",
    FLSEMU_KB(4),
    2,
    4,
    FLSEMU_KB(4),
    1,
    0x4000,
};


static FlsEmu_SegmentType const * segments[] = {
    &S12D512_PagedFlash,
    &S12D512_EEPROM,
};

static const FlsEmu_ConfigType FlsEmu_Config = {
    2,
    segments,
};

int main()
{
    bool finished;
    uint8_t * ptr;
    //int * ptr;

    //XcpOnCan_Init();
    //XcpOw_MapFileOpen(FNAME, &a2lFile);

    FlsEmu_Init(&FlsEmu_Config);
    //FlsEmu_ErasePage(0, 3);

    ptr = FlsEmu_BasePointer(0);
    printf("PTR %p\n", ptr);
    printf("PTR2 %p\n", ptr + 0x200);
    FlsEmu_SelectPage(0, 4);
    FillMemory(ptr + 0x200, 0x80, 0x55);

    //FillMemory(ptr + 0x800, 0x80, 0xff);

    Xcp_Init();
    DBG_PRINT1("Starting XCP task...\n");
    fflush(stdout);

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

    return Xcp_MemCmp(&secret, key->data, XCP_ARRAY_SIZE(secret));
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
            //printf("\t\t\tTRI [%u]\n", triangle.value);
            XcpDaq_TriggerEvent(1);
        }
        ticker++;
        previousTS =  XcpHw_GetTimerCounter() / 1000;
    }
}
