#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "xcp.h"

static uint32_t dispatch_count;
static uint8_t  dispatched_command;

Xcp_PduType Xcp_CtoIn;

void XcpHw_AcquireLock(uint8_t lockIdx) {
    (void)lockIdx;
}

void XcpHw_ReleaseLock(uint8_t lockIdx) {
    (void)lockIdx;
}

void Xcp_DispatchCommand(Xcp_PduType const * const pdu) {
    ++dispatch_count;
    dispatched_command = (pdu->len > 0u) ? pdu->data[0] : 0u;
}

void XcpTl_TimeoutInit(uint16_t timeout_value, void (*timeout_function)(void)) {
    (void)timeout_value;
    (void)timeout_function;
}

void XcpTl_TimeoutCheck(void) {
}

void XcpTl_TimeoutStart(void) {
}

void XcpTl_TimeoutStop(void) {
}

void XcpTl_TimeoutReset(void) {
}

void Serial_Init(void) {
}

void Serial_DeInit(void) {
}

uint32_t Serial_Available(void) {
    return 0u;
}

bool Serial_Read(uint8_t *in_byte) {
    (void)in_byte;
    return false;
}

void Serial_WriteByte(uint8_t out_byte) {
    (void)out_byte;
}

void Serial_WriteBuffer(uint8_t const *out_bytes, uint32_t size) {
    (void)out_bytes;
    (void)size;
}

void Serial_MainFunction(void) {
}

static void feed(const uint8_t *bytes, size_t count) {
    size_t i;
    for (i = 0u; i < count; ++i) {
        XcpTl_FeedReceiver(bytes[i]);
    }
}

int main(void) {
#if XCP_SXI_TEST_HEADER_FORMAT == 1
    static const uint8_t zero_len_byte[]  = { 0u };
    static const uint8_t valid_len_byte[] = { 1u, 0xC0u };
#elif (XCP_SXI_TEST_HEADER_FORMAT == 2) || (XCP_SXI_TEST_HEADER_FORMAT == 3) || (XCP_SXI_TEST_HEADER_FORMAT == 5)
    static const uint8_t zero_len_word[]  = { 0u, 0u };
    static const uint8_t valid_len_word[] = { 1u, 0u, 0xC0u };
#elif (XCP_SXI_TEST_HEADER_FORMAT == 4) || (XCP_SXI_TEST_HEADER_FORMAT == 6)
    static const uint8_t zero_len_counter_word[]  = { 0u, 0u, 0u, 0u };
    static const uint8_t valid_len_counter_word[] = { 1u, 0u, 0u, 0u, 0xC0u };
#else
    #error Unsupported SXI test header format
#endif
    const uint8_t *zero_frame;
    const uint8_t *valid_frame;
    size_t         zero_frame_size;
    size_t         valid_frame_size;

#if XCP_SXI_TEST_HEADER_FORMAT == 1
    zero_frame       = zero_len_byte;
    zero_frame_size  = sizeof(zero_len_byte);
    valid_frame      = valid_len_byte;
    valid_frame_size = sizeof(valid_len_byte);
#elif XCP_SXI_TEST_HEADER_FORMAT == 2
    zero_frame       = zero_len_word;
    zero_frame_size  = sizeof(zero_len_word);
    valid_frame      = valid_len_word;
    valid_frame_size = sizeof(valid_len_word);
#elif (XCP_SXI_TEST_HEADER_FORMAT == 3) || (XCP_SXI_TEST_HEADER_FORMAT == 5)
    zero_frame       = zero_len_word;
    zero_frame_size  = sizeof(zero_len_word);
    valid_frame      = valid_len_word;
    valid_frame_size = sizeof(valid_len_word);
#elif (XCP_SXI_TEST_HEADER_FORMAT == 4) || (XCP_SXI_TEST_HEADER_FORMAT == 6)
    zero_frame       = zero_len_counter_word;
    zero_frame_size  = sizeof(zero_len_counter_word);
    valid_frame      = valid_len_counter_word;
    valid_frame_size = sizeof(valid_len_counter_word);
#endif

    XcpTl_Init();
    feed(zero_frame, zero_frame_size);
    feed(valid_frame, valid_frame_size);

    if ((dispatch_count != 1u) || (dispatched_command != 0xC0u)) {
        fprintf(stderr, "expected one valid dispatch after a zero-length frame; got %u\n", dispatch_count);
        return 1;
    }
    return 0;
}
