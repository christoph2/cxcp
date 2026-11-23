#include <stdint.h>
#include <stdio.h>

#include "xcp.h"
#include "xcp_eth.h"

Xcp_OptionsType      Xcp_Options = { .ipv6 = false, .tcp = false, .port = 0u };
XcpTl_ConnectionType XcpTl_Connection;

void XcpUtl_MemSet(void *dest, uint8_t fill_char, uint32_t len) {
    uint8_t *bytes = (uint8_t *)dest;
    uint32_t i;
    for (i = 0u; i < len; ++i) {
        bytes[i] = fill_char;
    }
}

void XcpHw_ErrorMsg(char * const function, int errorCode) {
    fprintf(stderr, "%s failed (%d)\n", function, errorCode);
}

bool XcpTl_HandleTransportMulticastPacket(
    const uint8_t *buf, size_t len, const XcpTl_MacAddress *mac_address, const uint8_t source_ipv4[4]
) {
    (void)buf;
    (void)len;
    (void)mac_address;
    (void)source_ipv4;
    return false;
}

void XcpTl_Init(void);
void XcpTl_DeInit(void);

int main(void) {
    unsigned int iteration;

    for (iteration = 0u; iteration < 3u; ++iteration) {
        XcpTl_Init();
        if (XcpTl_Connection.discoverySocket < 0) {
            XcpTl_DeInit();
            fprintf(stderr, "discovery listener unavailable; lifecycle test cannot exercise its worker\n");
            return 77;
        }

        XcpTl_DeInit();
        if ((XcpTl_Connection.discoverySocket != -1) || (XcpTl_Connection.boundSocket != -1)) {
            fprintf(stderr, "XcpTl_DeInit did not invalidate its sockets\n");
            return 1;
        }
    }
    return 0;
}
