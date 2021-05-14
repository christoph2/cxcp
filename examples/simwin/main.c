
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define _WIN32_WINNT    0x601
#include <Windows.h>

#include "xcp.h"
#include "xcp_hw.h"
#include "flsemu.h"

void XcpOnCan_Init(void);

#define SIZE    (4096)
uint8_t puffer[SIZE];

DWORD XcpHw_UIThread(LPVOID param);
void XcpTl_SetOptions(Xcp_OptionsType const * options);

DWORD AppTask(LPVOID param);
DWORD Xcp_MainTask(LPVOID param);

#define FLS_SECTOR_SIZE (256)

#define FLS_PAGE_ADDR           ((uint16_t)0x8000U)
#define FLS_PAGE_SIZE           ((uint32_t)0x4000U)

uint8_t Xcp_CalRam[4096] = {0};

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
        XcpHw_ErrorMsg("memimnfo::VirtualQuery()", GetLastError());
    } else {
        printf("%p %p %d %08x %08x %08x\n", info.BaseAddress, info.AllocationBase, info.RegionSize / 1024,
               info.AllocationProtect, info.Protect, info.Type);
    }
}
#endif

#define XCP_THREAD  (0)
#define UI_THREAD   (1)
#define APP_THREAD  (2)

#define NUM_THREADS (3)

HANDLE threads[NUM_THREADS];
extern HANDLE quit_event;
HANDLE userTimer;
Xcp_OptionsType Xcp_Options;

typedef struct {
    uint8_t value;
    bool down;
    uint32_t dummy;
} triangle_type;

triangle_type triangle = {0};
uint16_t randomValue;
float sine_wave = 1.0;


int main(int argc, char **argv)
{
    size_t idx;
    Xcp_OptionsType options;

    srand(23);

    XcpHw_ParseCommandLineOptions(argc, argv, &options);
    XcpTl_SetOptions(&options);

    FlsEmu_Init(&FlsEmu_Config);

    Xcp_Init();
    Xcp_DisplayInfo();

    quit_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    userTimer = CreateWaitableTimer(NULL, FALSE, NULL);

    threads[XCP_THREAD] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Xcp_MainTask, &quit_event, 0, NULL);
    threads[UI_THREAD] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XcpHw_UIThread, &quit_event, 0, NULL);
    threads[APP_THREAD] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AppTask, &quit_event, 0, NULL);

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
    for (idx = 0; idx < NUM_THREADS; ++idx) {
        CloseHandle(threads[idx]);
    }

    CloseHandle(userTimer);

    FlsEmu_DeInit();
    XcpHw_Deinit();
    XcpTl_DeInit();

    return 0;
}

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
    XCP_UNREFERENCED_PARAMETER(mta);
    XCP_UNREFERENCED_PARAMETER(length);
    XCP_UNREFERENCED_PARAMETER(access);
    XCP_UNREFERENCED_PARAMETER(programming);

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


/////////////////////////
/////////////////////////
/////////////////////////

const XcpDaq_ODTEntryType XcpDaq_PredefinedOdtEntries[] = {
    XCP_DAQ_DEFINE_ODT_ENTRY(triangle.value),
    XCP_DAQ_DEFINE_ODT_ENTRY(randomValue),
    XCP_DAQ_DEFINE_ODT_ENTRY(sine_wave),
};


const XcpDaq_ODTType XcpDaq_PredefinedOdts[] = {
    {
        2, 0
    }, {
        1, 2,
    }
};


const XcpDaq_ListConfigurationType XcpDaq_PredefinedLists[] = {
    {
        2, 0
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

#define T_LOW_LIMIT     (70.0)
#define T_HIGH_LIMIT    (85.0)
#define T_STEP_SIZE     (1.5)


/** @brief
 *
 * @return Random selection of {-1, 0 , 1} values.
 *
 */
int8_t randomSlope(void)
{
    return (rand() % 3) - 1;
}

double t_env_func(void)
{
    static double t_prev = -273.15;

    t_prev = t_prev + (T_STEP_SIZE * randomSlope());
    t_prev = XCP_MIN(t_prev, T_HIGH_LIMIT);
    t_prev = XCP_MAX(t_prev, T_LOW_LIMIT);

    return t_prev;

#if 0
FUNC (void, Environment_CODE) envRE_func (void)
{
    /* read parameters for simulation of the temperature profile */
    float32 lLowLimit = Rte_Prm_EnvParamsRPP_env_TLowLimit();
    float32 lStepSize = Rte_Prm_EnvParamsRPP_env_TStepSize();

    /* retrieve internal state */
    uint32 lSeed = Rte_IrvIRead_envRE_Seed();
    float32 lTEnv = Rte_IrvIRead_envRE_TEnv();
    float32 direction = (float32)(lSeed % 3) - 1.0;

    /* calc high limit with parameter, store for measurement */
    *Rte_Pim_THighLimit() = lLowLimit + Rte_Prm_EnvParamsRPP_env_THighLimitDistance();

    /* update state for pseudo random number generation */
    lSeed = (8253729 * lSeed + 2396403);

    /* calculate environment temperature */
    lTEnv += lStepSize * direction;

    /* saturating environment temperature at the bounds */

    if (lTEnv < lLowLimit) { lTEnv = lLowLimit; }

    if( lTEnv > *Rte_Pim_THighLimit()) {
        lTEnv = * Rte_Pim_THighLimit();
    }

    /* Store internal state */
    Rte_IrvIWrite_envRE_Seed(lSeed);
    Rte_IrvIWrite_envRE_TEnv(lTEnv);

    /* write output */
    Rte_IWrite_envRE_EnvTemperaturePPP_T(lTEnv);
}
#endif
}

double plant_func(void)
{
//    q_plant = t_n * 1 * (j / k);
//    t_n = q_plant_n / (1 * (j / k));
}

double controller_func(void)
{

}


/////////////////////////
/////////////////////////
/////////////////////////


DWORD Xcp_MainTask(LPVOID param)
{
    HANDLE * quit_event = (HANDLE *)param;
    XCP_FOREVER {

        Xcp_MainFunction();
        XcpTl_MainFunction();
        if (WaitForSingleObject(*quit_event, INFINITE) == WAIT_OBJECT_0) {
            break;
        }
    }
    ExitThread(0);
}

DWORD AppTask(LPVOID param)
{
    static uint32_t currentTS = 0UL;
    static uint32_t previousTS = 0UL;
    static uint16_t ticker = 0;
    HANDLE * quit_event = (HANDLE *)param;

    LARGE_INTEGER liDueTime;
    LONG period;
    HANDLE handles[2] = {*quit_event, userTimer};
    DWORD res;

    //liDueTime.QuadPart=-10 0 000 000;
    liDueTime.QuadPart=-500;
    period = 150;
    SetWaitableTimer(userTimer, &liDueTime, period, NULL, NULL, 0);

    XCP_FOREVER {
        currentTS = XcpHw_GetTimerCounter() / 1000;
        if (currentTS >= (previousTS + 2)) {
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
//                printf("T: %0.2f ", t_env_func());
                //printf("\t\t\tTRI [%u]\n", triangle.value);
                XcpDaq_TriggerEvent(1);
            }
            ticker++;
            previousTS =  XcpHw_GetTimerCounter() / 1000;
        }
        res = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        if (res == WAIT_OBJECT_0) {
            break;
        }
        //if (WaitForSingleObject(*quit_event, 0) == WAIT_OBJECT_0) {
        //    break;
        //}
    }
    ExitThread(0);
}
