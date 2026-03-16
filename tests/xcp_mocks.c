
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "xcp.h"

uint32_t start_marker;
uint32_t voltage1;
uint32_t voltage2;
uint32_t voltage3;
uint32_t voltage4;
// Measurements.
uint16_t triangle_wave;
uint8_t  sq0_wave;
uint8_t  sq1_wave;
float    sine_wave;
uint32_t end_marker;

static uint8_t  last_send[256];
static uint16_t last_send_len;

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
    printf("XcpHw_Init\n");
    printf("start_marker: %p\n", &start_marker);
    voltage1 = 0xaaaaaaaa;
    voltage2 = 0xbbbbbbbb;
    voltage3 = 0xccccccc;
    voltage4 = 0xddddddd;
    printf("voltage1: %p 0x%08" PRIx32 "\n", (void *)&voltage1, voltage1);
    triangle_wave = 0xee;
    sq0_wave      = 0xff;
    sq1_wave      = 0x99;
    sine_wave     = 0x88;
}

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

void Hexdump(uint8_t const *buf, uint16_t sz) {
    uint16_t idx;

    for (idx = UINT16(0); idx < sz; ++idx) {
        printf("%02X ", buf[idx]);
    }
    printf("\n\r");
}

void XcpTl_ResetLastSend(void) {
    XCP_TL_ENTER_CRITICAL();
    last_send_len = 0;
    XCP_TL_LEAVE_CRITICAL();
}

uint16_t XcpTl_GetLastSend(uint8_t *buf, uint16_t max_len) {
    uint16_t copy_len;
    XCP_TL_ENTER_CRITICAL();
    copy_len = (last_send_len < max_len) ? last_send_len : max_len;
    if ((buf != NULL) && (copy_len > 0U)) {
        memcpy(buf, last_send, copy_len);
    }
    XCP_TL_LEAVE_CRITICAL();
    return last_send_len;
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    XCP_TL_ENTER_CRITICAL();
    last_send_len = (len < (uint16_t)sizeof(last_send)) ? len : (uint16_t)sizeof(last_send);
    memcpy(last_send, buf, last_send_len);
    printf("Send: %d bytes [%p]\n", len, buf);
    Hexdump(buf, len);
    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
}

char *data_start() {
    return (char *)&start_marker;
}

char *data_end() {
    return (char *)&end_marker;
}
