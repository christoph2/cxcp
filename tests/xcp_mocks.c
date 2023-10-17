
#include <stdint.h>
#include <time.h>

#include "xcp.h"

#if 0
typedef struct {
    uint8_t value;
    bool down;
    uint32_t dummy;
} triangle_type;

triangle_type triangle = {0};
uint16_t randomValue;
float sine_wave = 1.0;

XcpDaq_ODTEntryType XcpDaq_PredefinedOdtEntries[] = {
    XCP_DAQ_DEFINE_ODT_ENTRY(triangle.value),
    XCP_DAQ_DEFINE_ODT_ENTRY(randomValue),
    XCP_DAQ_DEFINE_ODT_ENTRY(sine_wave),
};

const XcpDaq_ODTType XcpDaq_PredefinedOdts[] = {{2, 0}, {1, 2}};

const XcpDaq_ListConfigurationType XcpDaq_PredefinedLists[] = {{2, 0}};

    #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
XcpDaq_ListStateType XcpDaq_PredefinedListsState[XCP_DAQ_PREDEFINDED_LIST_COUNT];
const XcpDaq_ListIntegerType XcpDaq_PredefinedListCount = XCP_DAQ_PREDEFINDED_LIST_COUNT;
    #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */
#endif

XCP_DAQ_BEGIN_EVENTS
XCP_DAQ_DEFINE_EVENT(
    "EVT 100ms", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 100
),
    XCP_DAQ_DEFINE_EVENT(
        "EVT sporadic", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 0
    ),
    XCP_DAQ_DEFINE_EVENT(
        "EVT 10ms", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 10
    ),
    XCP_DAQ_END_EVENTS

    uint32_t XcpHw_GetTimerCounter() {
    //    return clock();
    return 0xaaaa;
}

void XcpHw_TransmitDtos(void) {
}
