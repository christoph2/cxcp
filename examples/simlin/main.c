
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "xcp.h"
#include "xcp_hw.h"

void XcpOnCan_Init(void);

#define SIZE    (4096)
uint8_t puffer[SIZE];

#define CALRAM_SIZE (16 * 1024)
uint8_t calram[CALRAM_SIZE];

void XcpTl_SetOptions(XcpHw_OptionsType const * options);

void * AppTask(void * param);

extern pthread_t XcpHw_ThreadID[4];

#define XCP_THREAD  (0)
//#define IOCP_THREAD (1)
#define UI_THREAD   (1)
#define APP_THREAD  (2)

#define NUM_THREADS (3)

int main(int argc, char **argv)
{
    size_t idx;
    uint32_t state = 0UL;
    XcpHw_OptionsType options;

    srand(23);

    XcpHw_ParseCommandLineOptions(argc, argv, &options);
    XcpTl_SetOptions(&options);

    Xcp_Init();
    Xcp_DisplayInfo();

    pthread_create(&XcpHw_ThreadID[1], NULL, &AppTask, NULL);

    while (state != 0x01) {
        state = XcpHw_WaitApplicationState(0x01);
//        printf("signaled/main: %u\n", state);
        XcpHw_ResetApplicationState(0x01);
    }

//    pthread_join(XcpHw_ThreadID[0], NULL);

#if 0
    while (XCP_TRUE) {
        XcpTl_MainFunction();
    }
#endif

    XcpHw_Deinit();
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


bool Xcp_HookFunction_CheckMemoryAccess(Xcp_MtaType mta, uint32_t length, Xcp_MemoryAccessType access, bool programming)
{
    return XCP_TRUE;
}

Xcp_MemoryMappingResultType Xcp_HookFunction_AddressMapper(Xcp_MtaType * dst, Xcp_MtaType const * src)
{
//    return FlsEmu_MemoryMapper(dst, src);
    if ((src->address >= 0x4000) && (src->address < 0x8000)) {
        dst->address = (uint32_t)(calram - 0x4000) + src->address;
        dst->ext = src->ext;
    }
    return XCP_MEMORY_NOT_MAPPED;
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
    uint32_t dummy;
} triangle_type;

triangle_type triangle = {0};
uint16_t randomValue;

/////////////////////////
/////////////////////////
/////////////////////////

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

/////////////////////////
/////////////////////////
/////////////////////////

#if 0
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
#endif

void * AppTask(void * param)
{
    static uint32_t currentTS = 0UL;
    static uint32_t previousTS = 0UL;
    static uint16_t ticker = 0;
    uint32_t state;
#if 0
    HANDLE * quit_event = (HANDLE *)param;
    LARGE_INTEGER liDueTime;
    LONG period;
    HANDLE handles[2] = {*quit_event, userTimer};
    DWORD res;

    //liDueTime.QuadPart=-10 0 000 000;
    liDueTime.QuadPart=-50000;
    period = 5000;
    SetWaitableTimer(userTimer, &liDueTime, period, NULL, NULL, 0);
#endif

    XCP_FOREVER {

        state = XcpHw_WaitApplicationState(0x03);
//        printf("signaled/AppTask: %u\n", state);
        XcpHw_ResetApplicationState(state);
        if (state == 1) {
            break;
        }

        currentTS = XcpHw_GetTimerCounter() / 1000;
        if (currentTS >= (previousTS + 10)) {
//            printf("T [%u::%lu]\n", currentTS, XcpHw_GetTimerCounter() / 1000);

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
    return NULL;
}

