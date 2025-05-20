
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

void XcpTl_Init(void) {

}

void XcpHw_Init(void) {

}

#if 0
void XcpDaq_TransmitDtos(void) {
}
#endif

void XcpHw_AcquireLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
}

void XcpHw_ReleaseLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
}

void XcpTl_SaveConnection() {

}

void XcpTl_ReleaseConnection() {

}

void XcpHw_Sleep(uint64_t usec) {
    //    delayMicroseconds(usec);
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    XCP_TL_ENTER_CRITICAL();

    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
}