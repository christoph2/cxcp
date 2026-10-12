/*
 * BlueParrot XCP – mini_packed_test.c
 *
 * Minimal C unit-test for XCP 1.4 Packed DAQ Mode (SET/GET_DAQ_PACKED_MODE).
 *
 * Kompilieren (MSVC, Ethernet-Transport):
 *   cl /I..\inc /I..\examples\xcpsim /DTP_ETHER
 *      mini_packed_test.c ..\src\xcp.c ..\src\xcp_daq.c ..\src\xcp_util.c
 *      ..\src\xcp_checksum.c xcp_mocks.c xcp_init.c fls_mocks.c checksum_mocks.c
 *      /Fe:mini_packed_test.exe
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "xcp.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

static void run_test(const char *name, int result) {
    printf("[%s] %s\n", result ? "PASS" : "FAIL", name);
    assert(result);
}

/* ------------------------------------------------------------------ */
/* Test 1: Constants sanity                                             */
/* ------------------------------------------------------------------ */
static void test_constants(void) {
    run_test("XCP_SET_DAQ_PACKED_MODE == 0x01", XCP_SET_DAQ_PACKED_MODE == 0x01);
    run_test("XCP_GET_DAQ_PACKED_MODE == 0x02", XCP_GET_DAQ_PACKED_MODE == 0x02);
    run_test("NOT_PACKED == 0", XCP_DAQ_PACKED_MODE_NOT_PACKED == 0);
    run_test("ELEMENT_GROUPED == 1", XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED == 1);
    run_test("EVENT_GROUPED == 2", XCP_DAQ_PACKED_MODE_EVENT_GROUPED == 2);
    run_test("TS_PRESENT == 0x01", XCP_DAQ_PACKED_TS_PRESENT == 0x01);
    run_test("TS_SIZE_2B == 0x02", XCP_DAQ_PACKED_TS_SIZE_2B == 0x02);
    run_test("TS_SIZE_4B == 0x04", XCP_DAQ_PACKED_TS_SIZE_4B == 0x04);
    run_test("DAQ_LIST_PROPERTY_PACKED == 0x10", DAQ_LIST_PROPERTY_PACKED == 0x10);
}

/* ------------------------------------------------------------------ */
/* Test 2: XcpDaq_SetPackedMode / XcpDaq_GetPackedMode roundtrip       */
/* ------------------------------------------------------------------ */
#if XCP_DAQ_ENABLE_PACKED_MODE == XCP_ON
static void test_set_get_roundtrip(void) {
    uint8_t  rd_mode = 0xFF, rd_ts = 0xFF;
    uint16_t rd_cnt = 0xFFFF;

    /* We need at least one dynamic DAQ list allocated first. */
    Xcp_Init();

    /* Fake: connect + allocate one DAQ list with one ODT */
    {
        /* Simulate CONNECT */
        uint8_t     connect_buf[] = { 0xFF, 0x00 };
        Xcp_PduType pdu_connect   = { 2, connect_buf };
        Xcp_DispatchCommand(&pdu_connect);
    }
    {
        /* FREE_DAQ */
        uint8_t     buf[] = { 0xD6, 0x00, 0x00, 0x00 };
        Xcp_PduType p     = { 4, buf };
        Xcp_DispatchCommand(&p);
    }
    {
        /* ALLOC_DAQ count=1 */
        uint8_t     buf[] = { 0xD5, 0x00, 0x01, 0x00 };
        Xcp_PduType p     = { 4, buf };
        Xcp_DispatchCommand(&p);
    }
    {
        /* ALLOC_ODT daq=0 count=1 */
        uint8_t     buf[] = { 0xD4, 0x00, 0x00, 0x00, 0x01 };
        Xcp_PduType p     = { 5, buf };
        Xcp_DispatchCommand(&p);
    }
    {
        /* ALLOC_ODT_ENTRY daq=0 odt=0 entries=1 */
        uint8_t     buf[] = { 0xD3, 0x00, 0x00, 0x00, 0x00, 0x01 };
        Xcp_PduType p     = { 6, buf };
        Xcp_DispatchCommand(&p);
    }

    /* Now call the API directly */
    XcpDaq_SetPackedMode(
        (XcpDaq_ListIntegerType)0, XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED,
        (uint8_t)(XCP_DAQ_PACKED_TS_PRESENT | XCP_DAQ_PACKED_TS_SIZE_4B), (uint16_t)10
    );
    XcpDaq_GetPackedMode((XcpDaq_ListIntegerType)0, &rd_mode, &rd_ts, &rd_cnt);

    run_test("roundtrip: packed_mode == ELEMENT_GROUPED", rd_mode == XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED);
    run_test("roundtrip: ts_mode == TS_PRESENT|TS_SIZE_4B", rd_ts == (XCP_DAQ_PACKED_TS_PRESENT | XCP_DAQ_PACKED_TS_SIZE_4B));
    run_test("roundtrip: sample_count == 10", rd_cnt == 10);
}
#endif /* XCP_DAQ_ENABLE_PACKED_MODE */

/* ------------------------------------------------------------------ */
/* Test 3: SET_DAQ_PACKED_MODE via Xcp_DispatchCommand (0xC0 / 0x01)   */
/* ------------------------------------------------------------------ */
#if XCP_DAQ_ENABLE_PACKED_MODE == XCP_ON
static void test_dispatch_set_packed(void) {
    /*
     * Frame: [0xC0][0x01][daq_lsb][daq_msb][mode][ts_mode][count_lo][count_hi]
     * SET_DAQ_PACKED_MODE, daq=0, ELEMENT_GROUPED, ts=0x05 (present+4B), samples=4
     */
    uint8_t cmd[] = {
        0xC0,                    /* LEVEL1 CMD indicator */
        XCP_SET_DAQ_PACKED_MODE, /* Sub-command           */
        0x00,
        0x00,                                                    /* DAQ list number = 0  */
        XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED,                     /* Mode                  */
        (XCP_DAQ_PACKED_TS_PRESENT | XCP_DAQ_PACKED_TS_SIZE_4B), /* ts_mode */
        0x04,
        0x00 /* sample_count = 4     */
    };
    Xcp_PduType pdu = { (uint16_t)sizeof(cmd), cmd };
    Xcp_DispatchCommand(&pdu);

    {
        uint8_t  rd_mode = 0xFF, rd_ts = 0xFF;
        uint16_t rd_cnt = 0xFFFF;
        XcpDaq_GetPackedMode((XcpDaq_ListIntegerType)0, &rd_mode, &rd_ts, &rd_cnt);
        run_test("dispatch SET: mode == ELEMENT_GROUPED", rd_mode == XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED);
        run_test("dispatch SET: sample_count == 4", rd_cnt == 4);
    }
}
#endif /* XCP_DAQ_ENABLE_PACKED_MODE */

/* ------------------------------------------------------------------ */
/* Test 4: GET_DAQ_PACKED_MODE via Xcp_DispatchCommand (0xC0 / 0x02)   */
/* ------------------------------------------------------------------ */
#if XCP_DAQ_ENABLE_PACKED_MODE == XCP_ON
extern uint8_t Xcp_CtoOutBuffer[]; /* exposed for response inspection */

static void test_dispatch_get_packed(void) {
    /*
     * Frame: [0xC0][0x02][daq_lsb][daq_msb]
     */
    uint8_t     cmd[] = { 0xC0, XCP_GET_DAQ_PACKED_MODE, 0x00, 0x00 };
    Xcp_PduType pdu   = { (uint16_t)sizeof(cmd), cmd };
    Xcp_DispatchCommand(&pdu);
    /* Response byte[0] should be 0xFF (positive response) */
    run_test("dispatch GET: response[0] == 0xFF", Xcp_CtoOutBuffer[XCP_TRANSPORT_LAYER_BUFFER_OFFSET] == 0xFF);
    run_test(
        "dispatch GET: response[1] == ELEMENT_GROUPED",
        Xcp_CtoOutBuffer[XCP_TRANSPORT_LAYER_BUFFER_OFFSET + 1] == XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED
    );
    run_test("dispatch GET: response[3] == 4 (sample_count lo)", Xcp_CtoOutBuffer[XCP_TRANSPORT_LAYER_BUFFER_OFFSET + 3] == 4);
}
#endif /* XCP_DAQ_ENABLE_PACKED_MODE */

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */
int main(void) {
    printf("=== cxcp Packed DAQ Lists – unit tests ===\n\n");

    test_constants();

#if XCP_DAQ_ENABLE_PACKED_MODE == XCP_ON
    test_set_get_roundtrip();
    test_dispatch_set_packed();
    test_dispatch_get_packed();
#else
    printf("[SKIP] XCP_DAQ_ENABLE_PACKED_MODE is OFF – skipping runtime tests.\n");
#endif

    printf("\nAll assertions passed.\n");
    return 0;
}
