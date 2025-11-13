
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

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"
#include "xcp_tl_timeout.h"
/*!!! END-INCLUDE-SECTION !!!*/

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


const unsigned long READ_TIMEOUT_MS = 2000;  // Timeout when reading a frame

class ClientWrapper {
   public:

    virtual int    available()                                 = 0;
    virtual int    read_bytes(uint8_t* buf, size_t len)        = 0;
    virtual size_t write_bytes(const uint8_t* buf, size_t len) = 0;
    virtual bool   connected()                                 = 0;
    virtual void   stop()                                      = 0;
    virtual bool   is_datagram() const                         = 0;

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

    bool is_datagram() const override {
        return false;
    }

   private:

    EthernetClient m_client;
};

class EthernetUdpClientWrapper : public ClientWrapper {
   public:

    EthernetUdpClientWrapper() : m_udp(nullptr), m_remoteIp(), m_remotePort(0), m_size(0), m_offset(0) {
    }

    void reset(EthernetUDP* udp, IPAddress remoteIp, uint16_t remotePort) {
        m_udp        = udp;
        m_remoteIp   = remoteIp;
        m_remotePort = remotePort;
        m_size       = 0;
        m_offset     = 0;
    }

    int loadFromUdp(int packetSize) {
        if (!m_udp)
            return -1;
        if (packetSize <= 0)
            return -1;
        if (packetSize > (int)XCP_COMM_BUFLEN)
            packetSize = (int)XCP_COMM_BUFLEN;
        int len = m_udp->read(m_buf, (size_t)packetSize);
        if (len <= 0) {
            m_size   = 0;
            m_offset = 0;
            return -1;
        }
        m_size   = (size_t)len;
        m_offset = 0;
        return len;
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

    bool is_datagram() const override {
        return true;
    }

   private:

    EthernetUDP* m_udp;
    IPAddress    m_remoteIp;
    uint16_t     m_remotePort;
    uint8_t      m_buf[XCP_COMM_BUFLEN];
    size_t       m_size;
    size_t       m_offset;
};

class EthernetAdapter : public ArduinoNetworkIf {
   public:

    EthernetAdapter(uint16_t port) : m_port(port), m_use_dhcp(true), m_ip(0, 0, 0, 0) {
    }

    EthernetAdapter(IPAddress ip, uint16_t port) : m_port(port), m_use_dhcp(false), m_ip(ip) {
    }

    bool begin() override {
        Serial.begin(115200);
        while (!Serial) {
            ;  // wait for serial port to connect. Needed for native USB port only
        }
        Serial.println("\nBlueparrot XCP (UDP) on Ethernet");
        Ethernet.init(10);
        delay(1000);  // Give the hardware some time to initialize

        int result = 0;
        if (m_use_dhcp) {
            Serial.println("Attempting to get an IP address using DHCP...");
            result = Ethernet.begin(m_mac_address);  // try DHCP
            if (result == 0) {
                Serial.println("Failed to configure Ethernet using DHCP");
                return false;
            }
        } else {
            Serial.print("Configuring Ethernet with static IP: ");
            Serial.println(m_ip);
            Ethernet.begin(m_mac_address, m_ip);  // use static IP
            result = 1;                           // Assume success for static IP, as begin() doesn't indicate failure
        }

        // Check for a valid IP address
        if (Ethernet.localIP() == IPAddress(0, 0, 0, 0)) {
            Serial.println("Failed to get a valid IP address.");
            return false;
        }
        m_udp.begin(m_port);
        return true;
    }

    ClientWrapper* accept_client() override {
        int packetSize = m_udp.parsePacket();
        if (packetSize > 0) {
            if (packetSize > (int)XCP_COMM_BUFLEN)
                packetSize = (int)XCP_COMM_BUFLEN;
            IPAddress rip   = m_udp.remoteIP();
            uint16_t  rport = m_udp.remotePort();
            m_udpClientWrapper.reset(&m_udp, rip, rport);
            int len = m_udpClientWrapper.loadFromUdp(packetSize);
            if (len <= 0)
                return nullptr;
            return &m_udpClientWrapper;
        }
        return nullptr;
    }

    String getInfo() override {
        IPAddress ip = Ethernet.localIP();
        return String("Ethernet-UDP ") + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
    }

   private:

    uint8_t                  m_mac_address_storage[6] = XCP_ON_ETHERNET_MAC_ADDRESS;
    uint8_t*                 m_mac_address            = m_mac_address_storage;
    bool                     m_use_dhcp;
    IPAddress                m_ip;
    uint16_t                 m_port;
    EthernetUDP              m_udp;
    EthernetUdpClientWrapper m_udpClientWrapper;  // preallocated UDP client wrapper
};
    #endif  // XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_ETHERNET

    #if XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI
class WiFiUdpClientWrapper : public ClientWrapper {
   public:

    WiFiUdpClientWrapper() : m_udp(nullptr), m_remoteIp(), m_remotePort(0), m_size(0), m_offset(0) {
    }

    void reset(WiFiUDP* udp, IPAddress remoteIp, uint16_t remotePort) {
        m_udp        = udp;
        m_remoteIp   = remoteIp;
        m_remotePort = remotePort;
        m_size       = 0;
        m_offset     = 0;
    }

    int loadFromUdp(int packetSize) {
        if (!m_udp)
            return -1;
        if (packetSize <= 0)
            return -1;
        if (packetSize > (int)XCP_COMM_BUFLEN)
            packetSize = (int)XCP_COMM_BUFLEN;
        int len = m_udp->read(m_buf, (size_t)packetSize);
        if (len <= 0) {
            m_size   = 0;
            m_offset = 0;
            return -1;
        }
        m_size   = (size_t)len;
        m_offset = 0;
        return len;
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

    bool is_datagram() const override {
        return true;
    }

   private:

    WiFiUDP*  m_udp;
    IPAddress m_remoteIp;
    uint16_t  m_remotePort;
    uint8_t   m_buf[XCP_COMM_BUFLEN];
    size_t    m_size;
    size_t    m_offset;
};

class WiFiAdapter : public ArduinoNetworkIf {
   public:

    WiFiAdapter(const char* s, const char* p, uint16_t port) : m_port(port), m_ssid(s), m_pass(p) {
    }

    bool begin() override {
        // Put ESP32 in station mode and try to keep things deterministic during connect
        WiFi.persistent(false);   // avoid writing credentials repeatedly to NVS
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);     // keep radio awake during provisioning/UDP XCP

        // Optional static IP configuration (do this BEFORE begin())
        {
#if defined(XCP_ON_ETHERNET_IP_OCTETS)
            IPAddress ip(XCP_ON_ETHERNET_IP_OCTETS);
            if (!WiFi.config(ip)) {
                Serial.println("[WiFi] Failed to apply static IP config");
            }
#elif defined(XCP_ON_ETHERNET_IP)
            IPAddress ip;
            ip.fromString(XCP_ON_ETHERNET_IP);
            if (!WiFi.config(ip)) {
                Serial.println("[WiFi] Failed to apply static IP config");
            }
#endif
        }

        Serial.print("[WiFi] Connecting to SSID: ");
        Serial.println(m_ssid ? m_ssid : "<null>");
        wl_status_t startStatus = WiFi.begin(m_ssid, m_pass);
        (void)startStatus; // not all cores return a meaningful value here

        const unsigned long timeoutMs = 15000; // allow a bit more time on congested APs
        unsigned long       start      = millis();
        wl_status_t         st;
        while ((st = WiFi.status()) != WL_CONNECTED && (millis() - start < timeoutMs)) {
            switch (st) {
                case WL_CONNECT_FAILED:
                    Serial.println("[WiFi] Connect failed (bad SSID/Password?)");
                    break;
                case WL_NO_SSID_AVAIL:
                    Serial.println("[WiFi] SSID not available");
                    break;
                case WL_DISCONNECTED:
                case WL_IDLE_STATUS:
                default:
                    break;
            }
            delay(200);
        }

        bool connected = (WiFi.status() == WL_CONNECTED);
        if (connected) {
            IPAddress ip = WiFi.localIP();
            Serial.print("\nBlueparrot XCP (UDP) on WiFi -- IP: ");
            Serial.println(ip);
            if (!m_udp_active) {
                if (m_udp.begin(m_port)) {
                    m_udp_active = true;
                } else {
                    Serial.println("[WiFi] Failed to start UDP listener");
                }
            }
        } else {
            Serial.println("[WiFi] Not connected after timeout");
        }
        return connected;
    }

    ClientWrapper* accept_client() override {
        // Avoid UDP operations while STA is connecting; ESP-IDF logs: "wifi:sta is connecting, return error"
        if (WiFi.status() != WL_CONNECTED || !m_udp_active) {
            return nullptr;
        }
        int packetSize = m_udp.parsePacket();
        if (packetSize > 0) {
            if (packetSize > (int)XCP_COMM_BUFLEN)
                packetSize = (int)XCP_COMM_BUFLEN;
            IPAddress rip   = m_udp.remoteIP();
            uint16_t  rport = m_udp.remotePort();
            m_udpClientWrapper.reset(&m_udp, rip, rport);
            int len = m_udpClientWrapper.loadFromUdp(packetSize);
            if (len <= 0)
                return nullptr;
            return &m_udpClientWrapper;
        }
        return nullptr;
    }

    void process() override {
        // Keep UDP aligned with link state and perform gentle reconnects.
        static unsigned long lastAttempt = 0;
        wl_status_t          st          = WiFi.status();

        if (st == WL_CONNECTED) {
            if (!m_udp_active) {
                if (m_udp.begin(m_port)) {
                    m_udp_active = true;
                    Serial.println("[WiFi] UDP listener started after reconnect");
                }
            }
            m_reconnect_backoff_ms = 2000; // reset backoff on success
        } else {
            // Ensure UDP is not used while disconnected/connecting
            if (m_udp_active) {
                m_udp.stop();
                m_udp_active = false;
            }
            // Throttled reconnect attempts
            unsigned long now = millis();
            if (now - lastAttempt >= m_reconnect_backoff_ms) {
                lastAttempt = now;
                Serial.println("[WiFi] Attempting reconnect...");
                WiFi.reconnect();
                // Exponential backoff up to 30s
                if (m_reconnect_backoff_ms < 30000) {
                    m_reconnect_backoff_ms = m_reconnect_backoff_ms * 2;
                }
            }
        }
    }

    String getInfo() override {
        if (WiFi.status() == WL_CONNECTED) {
            IPAddress ip = WiFi.localIP();
            return String("WiFi-UDP ") + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
        }
        return String("WiFi-UDP <disconnected>");
    }

   private:

    uint16_t             m_port;
    WiFiUDP              m_udp;
    const char*          m_ssid;
    const char*          m_pass;
    WiFiUdpClientWrapper m_udpClientWrapper;
    bool                 m_udp_active = false;
    unsigned long        m_reconnect_backoff_ms = 2000; // start with 2s backoff
};
    #endif  // XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI

static ArduinoNetworkIf* s_net       = nullptr;
static ClientWrapper*    s_client    = nullptr;
static bool              s_connected = false;

static volatile bool s_timeout_triggered = false;

static void XcpTl_OnTimeout(void) {
    s_timeout_triggered = true;
}

class FrameParser {
   public:

    static int readFrame(ClientWrapper* client, uint8_t* buffer, size_t bufSize, unsigned long timeoutMs) {
        unsigned long start = millis();

        uint8_t lenBuf[2];

        if (!readN(client, lenBuf, 2, start, timeoutMs)) {
            return -1;
        }
        uint16_t len = (uint16_t(lenBuf[0]) << 8) | uint16_t(lenBuf[1]);
        if (len == 0 || len > bufSize) {
            return -1;
        }
        if (!readN(client, buffer, len, start, timeoutMs)) {
            return -1;
        }
        return int(len);
    }

    static bool readN(ClientWrapper* client, uint8_t* out, size_t n, unsigned long startTime, unsigned long timeoutMs) {
        size_t got = 0;
        while (got < n) {
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
                if (millis() - startTime > timeoutMs) {
                    return false;
                }
            }
        }
        return true;
    }
};

static bool ar_read_n(uint8_t* out, size_t n, unsigned long timeout_ms) {
    if (!s_client)
        return false;
    unsigned long start = millis();
    size_t        got   = 0;
    while (got < n) {
        if (s_timeout_triggered) {
            return false;
        }
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
            XcpTl_TimeoutReset();
        } else {
            XcpTl_TimeoutCheck();
            if (millis() - start > timeout_ms) {
                s_timeout_triggered = true;
                return false;
            }
            delay(1);
        }
    }
    return true;
}

static int ar_read_header(uint16_t* len, uint16_t* counter) {
    s_timeout_triggered = false;
    XcpTl_TimeoutStart();
    uint8_t hdr[4];
    if (!ar_read_n(hdr, 4, READ_TIMEOUT_MS)) {
        return 0;
    }

    *len     = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
    *counter = (uint16_t)hdr[2] | ((uint16_t)hdr[3] << 8);
    return 1;
}

static int ar_read_data(uint8_t* data, uint16_t len) {
    return ar_read_n(data, len, READ_TIMEOUT_MS) ? 1 : 0;
}

extern "C" {

    void XcpTl_Init(void) {
        XcpTl_TimeoutInit((uint16_t)READ_TIMEOUT_MS, XcpTl_OnTimeout);
        s_timeout_triggered = false;
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
            s_client = nullptr;
        }
        if (s_net) {
            delete s_net;
            s_net = nullptr;
        }
        s_connected = false;
    }

    static void XcpTl_Accept(void) {
        if (!s_net) {
            return;
        }
        s_net->process();
        ClientWrapper* c = s_net->accept_client();
        if (c) {
            s_client = c;
            if (!s_connected) {
                s_connected = true;
                XcpTl_SaveConnection();
            }
        }
    }

    void XcpTl_MainFunction(void) {
        XcpTl_Accept();
        XcpTl_TimeoutCheck();
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
            if (s_timeout_triggered) {
                s_timeout_triggered = false;
                XcpTl_TimeoutStop();
                Serial.println("Reception timeout (header) — aborting current frame");
                return;
            }
            /* Connection lost or other error */
            if (s_client && s_client->is_datagram()) {
                /* For UDP, keep last endpoint; just stop TL timeout and abort this frame */
                XcpTl_TimeoutStop();
                return;
            }
            XcpTl_ReleaseConnection();
            s_client->stop();
            s_client    = nullptr;
            s_connected = false;
            XcpTl_TimeoutStop();
            return;
        }

        if ((dlc > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE) || (dlc > (uint16_t)XCP_COMM_BUFLEN)) {
            // XcpHw_ErrorMsg((char*)"XcpTl_RxHandler: DLC too large", EINVAL);
            if (s_client && s_client->is_datagram()) {
                /* For UDP, keep endpoint but drop this malformed frame */
                XcpTl_TimeoutStop();
                return;
            }
            XcpTl_ReleaseConnection();
            s_client->stop();
            s_client    = nullptr;
            s_connected = false;
            return;
        }

        Xcp_CtoIn.len = dlc;
        int dres      = ar_read_data(Xcp_CtoIn.data, dlc);
        if (dres <= 0) {
            if (s_timeout_triggered) {
                s_timeout_triggered = false;
                XcpTl_TimeoutStop();
                Serial.println("Reception timeout (data) — aborting current frame");
                return;
            }
            if (s_client && s_client->is_datagram()) {
                /* For UDP, keep last endpoint on short read or error; abort this frame only */
                XcpTl_TimeoutStop();
                return;
            }
            XcpTl_ReleaseConnection();
            s_client->stop();
            s_client    = nullptr;
            s_connected = false;
            XcpTl_TimeoutStop();
            return;
        }
        /* Dispatch the freshly received CTO command. */
        Xcp_DispatchCommand(&Xcp_CtoIn);
        XcpTl_TimeoutStop();
    }

    void XcpTl_Send(uint8_t const * buf, uint16_t len) {
        XCP_TL_ENTER_CRITICAL();
        if (s_connected && s_client) {
    #ifdef XCP_UDP_DEBUG
            Serial.print("XCP_TL_SEND len=");
            Serial.print(len);
            Serial.print(" conn=");
            Serial.print(s_connected ? 1 : 0);
            Serial.print(" datagram=");
            Serial.println(s_client->is_datagram() ? 1 : 0);
    #endif
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
