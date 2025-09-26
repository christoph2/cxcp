/*
 * BlueParrot XCP - Arduino BLE Transport (Peripheral)
 *
 * Provides a BLE Peripheral with two characteristics:
 * - RX (Write Without Response): Host writes XCP request frames
 * - TX (Notify): Device notifies XCP response frames
 *
 * Frame format on RX:
 *   [len_lo, len_hi, ctr_lo, ctr_hi, payload...]
 * Where len is payload size (without header), ctr is a simple counter (ignored here).
 *
 * Outgoing TX uses raw XCP response data payload (no header) like other TLs.
 *
 * Boards: e.g. Arduino Nicla Sense ME (nRF52), Nano 33 BLE, etc.
 */

#include "xcp_config.h"

#if (XCP_TRANSPORT_LAYER == XCP_ON_BT) && defined(ARDUINO)

    #include <Arduino.h>
    #include <ArduinoBLE.h>

    /*!!! START-INCLUDE-SECTION !!!*/
    #include "xcp.h"
    #include "xcp_util.h"
/*!!! END-INCLUDE-SECTION !!!*/

/* Nordic UART Service (NUS)-style UUIDs */
static const char* BLE_SVC_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* BLE_RX_UUID  = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; /* Write (w/o response) */
static const char* BLE_TX_UUID  = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; /* Notify */

/* BLE objects */
static BLEService        XcpBleService(BLE_SVC_UUID);
static BLECharacteristic XcpBleRx(BLE_RX_UUID, BLEWriteWithoutResponse, XCP_COMM_BUFLEN);
static BLECharacteristic XcpBleTx(BLE_TX_UUID, BLENotify, XCP_COMM_BUFLEN);

/* Simple RX buffer for one incoming frame */
static uint8_t  XcpBle_RxBuf[XCP_COMM_BUFLEN];
static uint16_t XcpBle_RxLen = 0U;

/* Optional: connection state */
static bool XcpBle_CentralConnected = false;
/* Staged frame for processing in RxHandler (mirrors CAN TL pattern) */
static volatile bool XcpBle_FrameReceived = false;
static uint8_t       XcpBle_FrameBuf[XCP_COMM_BUFLEN];
static uint16_t      XcpBle_FrameLen = 0U;

static inline uint16_t u16_le(const uint8_t* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static bool XcpBle_HandleWrite(void) {
    int avail = XcpBleRx.valueLength();
    if (avail <= 0) {
        return false;
    }
    if ((size_t)avail > sizeof(XcpBle_RxBuf)) {
        /* Drop over-sized write */
        return false;
    }

    int rd = XcpBleRx.readValue(XcpBle_RxBuf, (size_t)avail);
    if (rd <= 0) {
        return false;
    }

    /* Expect: len(2) + ctr(2) + payload(len) */
    if (rd < 4) {
        return false;
    }
    uint16_t len = u16_le(&XcpBle_RxBuf[0]);
    /* uint16_t ctr = u16_le(&XcpBle_RxBuf[2]); -- currently unused */

    if ((uint16_t)(rd - 4) != len) {
        /* Malformed frame; length mismatch */
        return false;
    }
    if (len > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE) {
        /* Payload exceeds TL buffer limit */
        return false;
    }

    /* Stage frame for processing in RxHandler (consistent with CAN driver) */
    XcpBle_FrameLen = len;
    XcpUtl_MemCopy(XcpBle_FrameBuf, &XcpBle_RxBuf[4], len);
    XcpBle_FrameReceived = true;
    return true;
}

void XcpTl_Init(void) {
    /* Begin BLE stack */
    if (!BLE.begin()) {
        /* If BLE can't start, nothing else we can do here on Arduino */
        return;
    }

    /* Set advertised name and service */
    BLE.setLocalName("XCP-BLE");
    BLE.setDeviceName("XCP-BLE");
    BLE.setAdvertisedService(XcpBleService);

    /* Configure characteristics */
    XcpBleTx.setValue((const uint8_t*)"", 0);
    XcpBleService.addCharacteristic(XcpBleRx);
    XcpBleService.addCharacteristic(XcpBleTx);

    /* Add service and start advertising */
    BLE.addService(XcpBleService);
    BLE.advertise();

    XcpBle_CentralConnected = false;
    XcpBle_RxLen            = 0U;
}

void XcpTl_DeInit(void) {
    BLE.stopAdvertise();
    BLE.end();
    XcpBle_CentralConnected = false;
}

void XcpTl_MainFunction(void) {
    /* Poll BLE events */
    BLEDevice central = BLE.central();

    if (central && central.connected()) {
        if (!XcpBle_CentralConnected) {
            XcpBle_CentralConnected = true;
        }
        /* Handle inbound writes to RX characteristic */
        if (XcpBleRx.written()) {
            (void)XcpBle_HandleWrite();
        }
    } else {
        if (XcpBle_CentralConnected) {
            XcpBle_CentralConnected = false;
        }
    }
    /* Process staged frame if available (like CAN TL) */
    if (XcpTl_FrameAvailable(0, 0) > 0) {
        XcpTl_RxHandler();
    }
}

void XcpTl_Send(uint8_t const * buf, uint16_t len) {
    if (!buf || len == 0U) {
        return;
    }
    /* BLE Characteristic notifications are bounded by MTU/char size.
       We chunk if needed. */
    const uint16_t mtu  = (uint16_t)XCP_COMM_BUFLEN; /* Using characteristic buffer size as cap */
    uint16_t       sent = 0U;

    XCP_TL_ENTER_CRITICAL();
    while (sent < len) {
        uint16_t chunk = (uint16_t)((len - sent) > mtu ? mtu : (len - sent));
        /* Best effort: setValue triggers a notify for BLENotify characteristics */
        if (!XcpBleTx.setValue((const uint8_t*)(buf + sent), chunk)) {
            /* If notify fails, abort this send to avoid busy loop */
            break;
        }
        sent = (uint16_t)(sent + chunk);
        /* Give BLE stack a chance; optional small delay */
        /* delay(1); */
    }
    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_SaveConnection(void) {
    /* Not needed on BLE peripheral */
}

void XcpTl_ReleaseConnection(void) {
    /* Not needed on BLE peripheral */
}

void XcpTl_PrintConnectionInformation(void) {
    /* On Arduino, print via Serial if available */
    #if defined(SERIAL_PORT_MONITOR) || defined(Serial)
    if (Serial) {
        Serial.println("XCP over BLE Peripheral (NUS)");
    }
    #endif
}

void XcpTl_RxHandler(void) {
    /* Consume the staged frame and dispatch to XCP core */
    if (!XcpBle_FrameReceived) {
        return;
    }
    XcpBle_FrameReceived = false;
    XcpUtl_MemCopy(Xcp_CtoIn.data, XcpBle_FrameBuf, XcpBle_FrameLen);
    Xcp_CtoIn.len = XcpBle_FrameLen;
    Xcp_DispatchCommand(&Xcp_CtoIn);
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {
    (void)sec;
    (void)usec;
    return (int16_t)(XcpBle_FrameReceived ? 1 : 0);
}

#endif /* (XCP_TRANSPORT_LAYER == XCP_ON_BT) && defined(ARDUINO) */
