#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/tl/eth/discovery_validation.h"

static void make_packet(uint8_t *packet, size_t packet_size, uint8_t subcommand, const uint8_t source_ipv4[4]) {
    const size_t payload_size = packet_size - XCP_ETH_HEADER_SIZE;
    memset(packet, 0, packet_size);
    packet[0] = (uint8_t)(payload_size & 0xFFu);
    packet[1] = (uint8_t)((payload_size >> 8u) & 0xFFu);
    packet[4] = 0xF2u;
    packet[5] = subcommand;
    packet[6] = 0x34u;
    packet[7] = 0x12u;
    if (source_ipv4 != NULL) {
        memcpy(packet + 8, source_ipv4, 4u);
    }
}

static int expect(bool actual, bool expected, const char *description) {
    if (actual != expected) {
        fprintf(stderr, "%s: expected %s\n", description, expected ? "valid" : "invalid");
        return 1;
    }
    return 0;
}

int main(void) {
    const uint8_t source_ip[4]    = { 192u, 168u, 1u, 20u };
    const uint8_t zero_source[4]  = { 0u, 0u, 0u, 1u };
    const uint8_t mcast_source[4] = { 224u, 0u, 0u, 1u };
    uint8_t       packet[31];
    int           failures = 0;

    make_packet(packet, 12u, 0xFFu, source_ip);
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 12u, source_ip), true, "valid GET_SLAVE_ID");
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 11u, source_ip), false, "truncated GET_SLAVE_ID");

    packet[0]--;
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 12u, source_ip), false, "declared-size mismatch");
    make_packet(packet, 12u, 0xFDu, source_ip);
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 12u, source_ip), true, "valid extended discovery");
    packet[8]  = 192u;
    packet[9]  = 168u;
    packet[10] = 1u;
    packet[11] = 21u;
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 12u, source_ip), false, "spoofed response address");
    make_packet(packet, 12u, 0xFFu, source_ip);
    packet[6] = packet[7] = 0u;
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 12u, source_ip), false, "zero response port");
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 12u, NULL), false, "missing source address");
    make_packet(packet, 12u, 0xFFu, zero_source);
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 12u, zero_source), false, "unspecified source address");
    make_packet(packet, 12u, 0xFFu, mcast_source);
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 12u, mcast_source), false, "multicast source address");

    make_packet(packet, sizeof(packet), 0xFCu, source_ip);
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, sizeof(packet), source_ip), true, "valid SET_SLAVE_IP");
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, 30u, source_ip), false, "truncated SET_SLAVE_IP");
    failures += expect(XcpTl_ValidateDiscoveryPacket(packet, XCP_COMM_BUFLEN + 1u, source_ip), false, "oversized datagram");

    if (failures != 0) {
        fprintf(stderr, "%d discovery validation check(s) failed\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
