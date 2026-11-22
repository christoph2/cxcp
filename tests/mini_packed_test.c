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

void XcpTl_ResetLastSend(void);
uint16_t XcpTl_GetLastSend(uint8_t *buf, uint16_t max_len);

bool XcpHw_SetCalPage(uint8_t segment, uint8_t page, uint8_t mode) {
    (void)segment;
    (void)page;
    (void)mode;
    return false;
}

bool XcpHw_GetCalPage(uint8_t segment, uint8_t mode, uint8_t *page) {
    (void)segment;
    (void)mode;
    (void)page;
    return false;
}

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
    run_test("TIMESTAMP_LAST_SAMPLE == 0", XCP_DAQ_PACKED_TIMESTAMP_LAST_SAMPLE == 0);
    run_test("TIMESTAMP_FIRST_SAMPLE == 1", XCP_DAQ_PACKED_TIMESTAMP_FIRST_SAMPLE == 1);
    run_test("DAQ_LIST_PROPERTY_PACKED == 0x10", DAQ_LIST_PROPERTY_PACKED == 0x10);
}

static int last_response_is_error(uint8_t error_code) {
	uint8_t  response[256] = { 0 };
	uint16_t response_len  = XcpTl_GetLastSend(response, (uint16_t)sizeof(response));
	size_t   payload_offset = XCP_TRANSPORT_LAYER_BUFFER_OFFSET;

	return (response_len >= (payload_offset + 2U)) && (response[payload_offset] == XCP_PACKET_IDENTIFIER_ERR) &&
	       (response[payload_offset + 1U] == error_code);
}

static void test_dispatch_rejects_malformed_ctos(void) {
	uint8_t     connect[]        = { XCP_CONNECT, 0x00 };
	uint8_t     empty_request    = 0;
	uint8_t     truncated_mta[]  = { XCP_SET_MTA };
	Xcp_PduType pdu              = { (uint16_t)sizeof(connect), connect };

	Xcp_Init();
	Xcp_DispatchCommand(&pdu);

	XcpTl_ResetLastSend();
	pdu.len  = 0;
	pdu.data = &empty_request;
	Xcp_DispatchCommand(&pdu);
	run_test("dispatch rejects an empty CTO", last_response_is_error((uint8_t)ERR_CMD_SYNTAX));

	XcpTl_ResetLastSend();
	pdu.len  = (uint16_t)sizeof(truncated_mta);
	pdu.data = truncated_mta;
	Xcp_DispatchCommand(&pdu);
	run_test("dispatch rejects a truncated SET_MTA", last_response_is_error((uint8_t)ERR_CMD_SYNTAX));
}

static void test_dispatch_rejects_truncated_variable_ctos(void) {
	static const struct {
		uint16_t    length;
		uint8_t     data[8];
		char const *name;
	} cases[] = {
		{ 2, { XCP_GET_SEED, 0 }, "dispatch rejects a truncated GET_SEED" },
		{ 3, { XCP_DOWNLOAD, 2, 0xAA }, "dispatch rejects a truncated DOWNLOAD payload" },
		{ 8, { XCP_SHORT_DOWNLOAD, 1, 0, 0, 0, 0, 0, 0 }, "dispatch rejects a truncated SHORT_DOWNLOAD payload" },
		{ 1, { XCP_DOWNLOAD_MAX }, "dispatch rejects a truncated DOWNLOAD_MAX" },
		{ 2, { XCP_TRANSPORT_LAYER_CMD, 0xFA }, "dispatch rejects a truncated transport-layer command" },
		{ 2, { WRITE_DAQ_MULTIPLE, 1 }, "dispatch rejects a truncated WRITE_DAQ_MULTIPLE" },
		{ 2, { XCP_LEVEL1_COMMAND, XCP_SET_DAQ_PACKED_MODE }, "dispatch rejects a truncated level-1 command" },
	};
	uint8_t     connect[] = { XCP_CONNECT, 0x00 };
	Xcp_PduType pdu       = { (uint16_t)sizeof(connect), connect };

	Xcp_Init();
	Xcp_DispatchCommand(&pdu);

	for (size_t idx = 0; idx < (sizeof(cases) / sizeof(cases[0])); ++idx) {
		XcpTl_ResetLastSend();
		pdu.len  = cases[idx].length;
		pdu.data = (uint8_t *)cases[idx].data;
		Xcp_DispatchCommand(&pdu);
		run_test(cases[idx].name, last_response_is_error((uint8_t)ERR_CMD_SYNTAX));
	}
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
        (XcpDaq_ListIntegerType)0, XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED, XCP_DAQ_PACKED_TIMESTAMP_FIRST_SAMPLE, (uint16_t)10
    );
    XcpDaq_GetPackedMode((XcpDaq_ListIntegerType)0, &rd_mode, &rd_ts, &rd_cnt);

    run_test("roundtrip: packed_mode == ELEMENT_GROUPED", rd_mode == XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED);
    run_test("roundtrip: ts_mode == TIMESTAMP_FIRST_SAMPLE", rd_ts == XCP_DAQ_PACKED_TIMESTAMP_FIRST_SAMPLE);
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
     * SET_DAQ_PACKED_MODE, daq=0, ELEMENT_GROUPED, timestamp of first sample, samples=4
     */
    uint8_t cmd[] = {
        0xC0,                    /* LEVEL1 CMD indicator */
        XCP_SET_DAQ_PACKED_MODE, /* Sub-command           */
        0x00,
        0x00,                                  /* DAQ list number = 0  */
        XCP_DAQ_PACKED_MODE_ELEMENT_GROUPED,   /* Mode                  */
        XCP_DAQ_PACKED_TIMESTAMP_FIRST_SAMPLE, /* ts_mode */
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
    test_dispatch_rejects_malformed_ctos();
    test_dispatch_rejects_truncated_variable_ctos();

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
