/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#if (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET) && (defined(ARDUINO))

    #include <Arduino.h>

    #ifdef ESP32
        #include <WiFi.h>
        #include <WiFiUdp.h>
    #else
        #include <Ethernet.h>
        #include <EthernetUdp.h>
        #include <SPI.h>
    #endif

    /*!!! START-INCLUDE-SECTION !!!*/
    #include "xcp.h"
    #include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

const unsigned long READ_TIMEOUT_MS = 2000;  // Timeout beim Lesen eines Frames
const size_t        MAX_FRAME_SIZE  = 1024;  // maximale erlaubte Frame-Größe

class ClientWrapper {
   public:

    virtual int    available()                                 = 0;
    virtual int    read_bytes(uint8_t* buf, size_t len)        = 0;
    virtual size_t write_bytes(const uint8_t* buf, size_t len) = 0;
    virtual bool   connected()                                 = 0;
    virtual void   stop()                                      = 0;

    virtual ~ClientWrapper() {
    }
};

class ArduinoNetworkIf {
   public:

    virtual bool           begin()         = 0;
    virtual ClientWrapper* accept_client() = 0;

    virtual void process() {
    }

    virtual String getInfo() {
        return String();
    }

    virtual ~ArduinoNetworkIf() {
    }
};

    #if XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_ETHERNET
class EthernetClientWrapper : public ClientWrapper {
   public:

    EthernetClientWrapper(EthernetClient c) : m_client(c) {
    }

    int available() override {
        return m_client.available();
    }

    int read_bytes(uint8_t* buf, size_t len) override {
        return m_client.read(buf, len);
    }

    size_t write_bytes(const uint8_t* buf, size_t len) override {
        size_t nbytes = m_client.write(buf, len);
        return nbytes;
    }

    bool connected() override {
        return m_client && m_client.connected();
    }

    void stop() override {
        if (m_client) {
            m_client.stop();
        }
    }

   private:

    EthernetClient m_client;
};

class EthernetUdpClientWrapper : public ClientWrapper {
   public:

    EthernetUdpClientWrapper(EthernetUDP* udp, IPAddress remoteIp, uint16_t remotePort, const uint8_t* data, size_t size) :
        m_udp(udp), m_remoteIp(remoteIp), m_remotePort(remotePort), m_size(size), m_offset(0) {
        if (m_size > MAX_FRAME_SIZE)
            m_size = MAX_FRAME_SIZE;
        memcpy(m_buf, data, m_size);
    }

    int available() override {
        return (int)(m_size - m_offset);
    }

    int read_bytes(uint8_t* buf, size_t len) override {
        size_t remaining = m_size - m_offset;
        size_t toCopy    = remaining < len ? remaining : len;
        if (toCopy == 0)
            return 0;
        memcpy(buf, m_buf + m_offset, toCopy);
        m_offset += toCopy;
        return (int)toCopy;
    }

    size_t write_bytes(const uint8_t* buf, size_t len) override {
        if (!m_udp)
            return 0;
        if (m_udp->beginPacket(m_remoteIp, m_remotePort) == 0)
            return 0;
        size_t written = m_udp->write(buf, len);
        m_udp->endPacket();
        return written;
    }

    bool connected() override {
        return m_offset < m_size;
    }

    void stop() override {
        // no persistent connection to stop for UDP
    }

   private:

    EthernetUDP* m_udp;
    IPAddress    m_remoteIp;
    uint16_t     m_remotePort;
    uint8_t      m_buf[MAX_FRAME_SIZE];
    size_t       m_size;
    size_t       m_offset;
};

class EthernetAdapter : public ArduinoNetworkIf {
   public:

    // DHCP constructor (backward compatible)
    EthernetAdapter(uint16_t port) : m_port(port), m_use_dhcp(true), m_ip(0, 0, 0, 0) {
    }

    // Static IP constructor
    EthernetAdapter(IPAddress ip, uint16_t port) : m_port(port), m_use_dhcp(false), m_ip(ip) {
    }

    bool begin() override {
        Serial.begin(115200);
        while (!Serial) {
            ;  // wait for serial port to connect. Needed for native USB port only
        }
        if (m_use_dhcp) {
            (void)Ethernet.begin(m_mac_address);  // try DHCP
        } else {
            (void)Ethernet.begin(m_mac_address, m_ip);  // use static IP
        }
        Serial.print("\nBlueparrot XCP (UDP) on Ethernet -- IP: ");
        Serial.println(Ethernet.localIP());
        delay(1000);
        m_udp.begin(m_port);
        return true;
    }

    ClientWrapper* accept_client() override {
        int packetSize = m_udp.parsePacket();
        if (packetSize > 0) {
            if (packetSize > (int)MAX_FRAME_SIZE)
                packetSize = (int)MAX_FRAME_SIZE;
            uint8_t buf[MAX_FRAME_SIZE];
            int     len = m_udp.read(buf, packetSize);
            if (len <= 0)
                return nullptr;
            IPAddress rip   = m_udp.remoteIP();
            uint16_t  rport = m_udp.remotePort();
            return new EthernetUdpClientWrapper(&m_udp, rip, rport, buf, (size_t)len);
        }
        return nullptr;
    }

    String getInfo() override {
        IPAddress ip = Ethernet.localIP();
        return String("Ethernet-UDP ") + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
    }

   private:

    char        m_mac_address_storage[6] = XCP_ON_ETHERNET_MAC_ADDRESS;
    char*       m_mac_address            = m_mac_address_storage;
    bool        m_use_dhcp;
    IPAddress   m_ip;
    uint16_t    m_port;
    EthernetUDP m_udp;
};
    #endif  // XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_ETHERNET

    #if XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI
class WiFiUdpClientWrapper : public ClientWrapper {
   public:

    WiFiUdpClientWrapper(WiFiUDP* udp, IPAddress remoteIp, uint16_t remotePort, const uint8_t* data, size_t size) :
        m_udp(udp), m_remoteIp(remoteIp), m_remotePort(remotePort), m_size(size), m_offset(0) {
        if (m_size > MAX_FRAME_SIZE)
            m_size = MAX_FRAME_SIZE;
        memcpy(m_buf, data, m_size);
    }

    int available() override {
        return (int)(m_size - m_offset);
    }

    int read_bytes(uint8_t* buf, size_t len) override {
        size_t remaining = m_size - m_offset;
        size_t toCopy    = remaining < len ? remaining : len;
        if (toCopy == 0)
            return 0;
        memcpy(buf, m_buf + m_offset, toCopy);
        m_offset += toCopy;
        return (int)toCopy;
    }

    size_t write_bytes(const uint8_t* buf, size_t len) override {
        if (!m_udp)
            return 0;
        if (m_udp->beginPacket(m_remoteIp, m_remotePort) == 0)
            return 0;
        size_t written = m_udp->write(buf, len);
        m_udp->endPacket();
        return written;
    }

    bool connected() override {
        return m_offset < m_size;
    }

    void stop() override {
        // no-op for UDP
    }

   private:

    WiFiUDP*  m_udp;
    IPAddress m_remoteIp;
    uint16_t  m_remotePort;
    uint8_t   m_buf[MAX_FRAME_SIZE];
    size_t    m_size;
    size_t    m_offset;
};

class WiFiAdapter : public ArduinoNetworkIf {
   public:

    WiFiAdapter(const char* s, const char* p, uint16_t port) : m_port(port), m_ssid(s), m_pass(p) {
    }

    bool begin() override {
        WiFi.mode(WIFI_STA);
        WiFi.begin(m_ssid, m_pass);

        unsigned long start = millis();
        while ((WiFi.status() != WL_CONNECTED) && (millis() - start < 10000)) {
            delay(200);
        }
        Serial.print("\nBlueparrot XCP (UDP) on WiFi -- IP: ");
        Serial.println(WiFi.localIP());
        m_udp.begin(m_port);
        return WiFi.status() == WL_CONNECTED;
    }

    ClientWrapper* accept_client() override {
        int packetSize = m_udp.parsePacket();
        if (packetSize > 0) {
            if (packetSize > (int)MAX_FRAME_SIZE)
                packetSize = (int)MAX_FRAME_SIZE;
            uint8_t buf[MAX_FRAME_SIZE];
            int     len = m_udp.read(buf, packetSize);
            if (len <= 0)
                return nullptr;
            IPAddress rip   = m_udp.remoteIP();
            uint16_t  rport = m_udp.remotePort();
            return new WiFiUdpClientWrapper(&m_udp, rip, rport, buf, (size_t)len);
        }
        return nullptr;
    }

    void process() override {
        // reconnect handling
        if (WiFi.status() != WL_CONNECTED) {
            static unsigned long lastAttempt = 0;
            if (millis() - lastAttempt > 5000) {
                lastAttempt = millis();
                WiFi.reconnect();
            }
        }
    }

    String getInfo() override {
        IPAddress ip = WiFi.localIP();
        return String("WiFi-UDP ") + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
    }

   private:

    uint16_t    m_port;
    WiFiUDP     m_udp;
    const char* m_ssid;
    const char* m_pass;
};
    #endif  // XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI

// ======= Frame Parser / Handler =======

class FrameParser {
   public:

    // liest ein Frame: returns size or -1 on error/timeout, fills buffer (must be >= MAX_FRAME_SIZE)
    static int readFrame(ClientWrapper* client, uint8_t* buffer, size_t bufSize, unsigned long timeoutMs) {
        unsigned long start = millis();
        // Schritt 1: lese 2-Byte Länge (Big Endian)
        uint8_t lenBuf[2];

        if (!readN(client, lenBuf, 2, start, timeoutMs)) {
            return -1;
        }
        uint16_t len = (uint16_t(lenBuf[0]) << 8) | uint16_t(lenBuf[1]);
        if (len == 0 || len > bufSize) {
            return -1;
        }
        // Schritt 2: lese len bytes
        if (!readN(client, buffer, len, start, timeoutMs)) {
            return -1;
        }
        return int(len);
    }

    static bool readN(ClientWrapper* client, uint8_t* out, size_t n, unsigned long startTime, unsigned long timeoutMs) {
        size_t got = 0;
        while (got < n) {
            // Wenn Client disconnected -> fail
            if (!client->connected() && client->available() == 0) {
                return false;
            }
            int av = client->available();
            if (av > 0) {
                int toRead = min((size_t)av, n - got);
                int r      = client->read_bytes(out + got, toRead);
                if (r <= 0) {
                    return false;
                }
                got += r;
            } else {
                // keine Daten jetzt, prüfe Timeout
                if (millis() - startTime > timeoutMs) {
                    return false;
                }
                // delay(1);
            }
        }
        return true;
    }
};

// ======= XCP Transport Layer Interface Implementation =======

/* The Arduino Ethernet/WiFi backend must expose the same TL entry points
   as other drivers: XcpTl_Init, XcpTl_MainFunction, XcpTl_RxHandler,
   XcpTl_Send, XcpTl_DeInit, XcpTl_SaveConnection, XcpTl_ReleaseConnection,
   XcpTl_VerifyConnection. */

static ArduinoNetworkIf* s_net       = nullptr;
static ClientWrapper*    s_client    = nullptr;
static bool              s_connected = false;

/* Simple helpers to read exact N bytes from the active client with a timeout. */
static bool ar_read_n(uint8_t* out, size_t n, unsigned long timeout_ms) {
    if (!s_client)
        return false;
    unsigned long start = millis();
    size_t        got   = 0;
    while (got < n) {
        if (!s_client->connected() && s_client->available() == 0) {
            return false;
        }
        int av = s_client->available();
        if (av > 0) {
            int toRead = (int)min((size_t)av, n - got);
            int r      = s_client->read_bytes(out + got, (size_t)toRead);
            if (r <= 0)
                return false;
            got += (size_t)r;
        } else {
            if (millis() - start > timeout_ms)
                return false;
            delay(1);
        }
    }
    return true;
}

static int ar_read_header(uint16_t* len, uint16_t* counter) {
    uint8_t hdr[4];
    if (!ar_read_n(hdr, 4, READ_TIMEOUT_MS)) {
        return 0; /* treat as no data/closed */
    }
    /* XCP on ETH header is 4 bytes, little-endian: LEN (2), CTR (2) */
    *len     = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
    *counter = (uint16_t)hdr[2] | ((uint16_t)hdr[3] << 8);
    return 1;
}

static int ar_read_data(uint8_t* data, uint16_t len) {
    return ar_read_n(data, len, READ_TIMEOUT_MS) ? 1 : 0;
}

extern "C" {

    void XcpTl_Init(void) {
        /* Initialize backend network adapter and start listening. */
    #if XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI
        s_net = new WiFiAdapter(XCP_ON_ETHERNET_WIFI_SSID, XCP_ON_ETHERNET_WIFI_PASSWORD, XCP_ON_ETHERNET_PORT);
    #else
        #if defined(XCP_ON_ETHERNET_IP_OCTETS)
        s_net = new EthernetAdapter(IPAddress(XCP_ON_ETHERNET_IP_OCTETS), XCP_ON_ETHERNET_PORT);
        #elif defined(XCP_ON_ETHERNET_IP)
        {
            IPAddress ip;
            ip.fromString(XCP_ON_ETHERNET_IP);
            s_net = new EthernetAdapter(ip, XCP_ON_ETHERNET_PORT);
        }
        #else
        s_net = new EthernetAdapter(XCP_ON_ETHERNET_PORT);
        #endif
    #endif
        if (s_net) {
            (void)s_net->begin();
        }
        s_connected = false;
    }

    void XcpTl_DeInit(void) {
        if (s_client) {
            s_client->stop();
            delete s_client;
            s_client = nullptr;
        }
        if (s_net) {
            delete s_net;
            s_net = nullptr;
        }
        s_connected = false;
    }

    static void XcpTl_Accept(void) {
        if (!s_connected && s_net) {
            s_net->process();
            ClientWrapper* c = s_net->accept_client();
            if (c) {
                s_client    = c;
                s_connected = true;
                XcpTl_SaveConnection();
            }
        }
    }

    void XcpTl_MainFunction(void) {
        XcpTl_Accept();
        XcpTl_RxHandler();
    }

    extern Xcp_PduType Xcp_CtoIn;
    extern void        Xcp_DispatchCommand(Xcp_PduType const * pdu);

    void XcpTl_RxHandler(void) {
        if (!s_connected || !s_client) {
            return;
        }

        uint16_t dlc     = 0U;
        uint16_t counter = 0U;

        /* Non-blocking: only try if at least header is available. */
        if (s_client->available() < (int)XCP_ETH_HEADER_SIZE) {
            return;
        }

        int hres = ar_read_header(&dlc, &counter);
        if (hres <= 0) {
            /* Connection lost or no data */
            XcpTl_ReleaseConnection();
            s_client->stop();
            delete s_client;
            s_client    = nullptr;
            s_connected = false;
            return;
        }

        if (dlc > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE || dlc > (uint16_t)XCP_COMM_BUFLEN) {
            // XcpHw_ErrorMsg((char*)"XcpTl_RxHandler: DLC too large", EINVAL);
            XcpTl_ReleaseConnection();
            s_client->stop();
            delete s_client;
            s_client    = nullptr;
            s_connected = false;
            return;
        }

        Xcp_CtoIn.len = dlc;
        int dres      = ar_read_data(Xcp_CtoIn.data, dlc);
        if (dres <= 0) {
            XcpTl_ReleaseConnection();
            s_client->stop();
            delete s_client;
            s_client    = nullptr;
            s_connected = false;
            return;
        }

        /* Dispatch the freshly received CTO command. */
        Xcp_DispatchCommand(&Xcp_CtoIn);

        /* For UDP wrappers, the packet is fully consumed now; mark connection free. */
        if (!s_client->connected()) {
            XcpTl_ReleaseConnection();
            s_client->stop();
            delete s_client;
            s_client    = nullptr;
            s_connected = false;
        }
    }

    void XcpTl_Send(uint8_t const * buf, uint16_t len) {
        XCP_TL_ENTER_CRITICAL();
        if (s_connected && s_client) {
            /* Data already contains ETH header (LEN/CTR) + payload. */
            (void)s_client->write_bytes(buf, len);
        }
        XCP_TL_LEAVE_CRITICAL();
    }

    void XcpTl_SaveConnection(void) {
        /* Not using address structs on Arduino; mark connected state. */
        s_connected = true;
    }

    void XcpTl_ReleaseConnection(void) {
        s_connected = false;
    }

    bool XcpTl_VerifyConnection(void) {
        /* Single client only; verify simply returns connection state. */
        return s_connected;
    }

}  // extern "C"

#endif /* (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET) && (defined(ARDUINO)) */
