#include "xcp_config.h"

#if (XCP_TRANSPORT_LAYER == XCP_ON_BTH) && defined(ARDUINO)

    #include <Arduino.h>

    #if defined(ESP32)
        #include <BluetoothSerial.h>

    /*!!! START-INCLUDE-SECTION !!!*/
        #include "xcp.h"
    /*!!! END-INCLUDE-SECTION !!!*/

        #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
            #error "XCP_ON_BTH on ESP32 requires Classic Bluetooth (CONFIG_BT_ENABLED + CONFIG_BLUEDROID_ENABLED)."
        #endif

static BluetoothSerial XcpBt_Serial;
static bool            XcpBt_Connected = false;

typedef enum tagXcpBt_RxStateType {
    XCP_BT_RX_HEADER,
    XCP_BT_RX_PAYLOAD
} XcpBt_RxStateType;

typedef struct tagXcpBt_RxContextType {
    XcpBt_RxStateType state;
    uint8_t           header[XCP_ETH_HEADER_SIZE];
    uint8_t           payload[XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE];
    uint16_t          headerIndex;
    uint16_t          payloadIndex;
    uint16_t          payloadLength;
    bool              frameReady;
} XcpBt_RxContextType;

static XcpBt_RxContextType XcpBt_RxContext = { XCP_BT_RX_HEADER, { 0U }, { 0U }, 0U, 0U, 0U, false };

static void XcpBt_ResetReceiver(void) {
    XcpBt_RxContext.state         = XCP_BT_RX_HEADER;
    XcpBt_RxContext.headerIndex   = 0U;
    XcpBt_RxContext.payloadIndex  = 0U;
    XcpBt_RxContext.payloadLength = 0U;
    XcpBt_RxContext.frameReady    = false;
}

static void XcpBt_UpdateConnectionState(void) {
    bool connected = XcpBt_Serial.hasClient() && XcpBt_Serial.connected(0);

    if (connected && !XcpBt_Connected) {
        XcpBt_Connected = true;
        XcpTl_SaveConnection();
        XcpBt_ResetReceiver();
    } else if (!connected && XcpBt_Connected) {
        XcpBt_Connected = false;
        XcpTl_ReleaseConnection();
        XcpBt_ResetReceiver();
    }
}

static void XcpBt_PumpRx(void) {
    while ((XcpBt_Serial.available() > 0) && !XcpBt_RxContext.frameReady) {
        int byteRead = XcpBt_Serial.read();
        if (byteRead < 0) {
            break;
        }

        if (XcpBt_RxContext.state == XCP_BT_RX_HEADER) {
            XcpBt_RxContext.header[XcpBt_RxContext.headerIndex++] = (uint8_t)byteRead;
            if (XcpBt_RxContext.headerIndex == XCP_ETH_HEADER_SIZE) {
                XcpBt_RxContext.payloadLength = XCP_MAKEWORD(XcpBt_RxContext.header[1], XcpBt_RxContext.header[0]);
                if ((XcpBt_RxContext.payloadLength == 0U) || (XcpBt_RxContext.payloadLength > XCP_MAX_CTO)) {
                    XcpBt_ResetReceiver();
                    continue;
                }
                XcpBt_RxContext.state        = XCP_BT_RX_PAYLOAD;
                XcpBt_RxContext.payloadIndex = 0U;
            }
        } else {
            XcpBt_RxContext.payload[XcpBt_RxContext.payloadIndex++] = (uint8_t)byteRead;
            if (XcpBt_RxContext.payloadIndex == XcpBt_RxContext.payloadLength) {
                XcpBt_RxContext.frameReady = true;
            }
        }
    }
}

void XcpTl_Init(void) {
    if (!XcpBt_Serial.begin(XCP_ON_BTH_DEVICE_NAME)) {
        DBG_PRINT("XcpTl_Init: BluetoothSerial.begin failed\n");
        return;
    }
    if ((XCP_ON_BTH_PIN[0] != '\0') && !XcpBt_Serial.setPin(XCP_ON_BTH_PIN)) {
        DBG_PRINT("XcpTl_Init: BluetoothSerial.setPin failed\n");
    }
    XcpBt_Connected = false;
    XcpBt_ResetReceiver();
}

void XcpTl_DeInit(void) {
    XcpBt_Serial.end();
    XcpBt_Connected = false;
    XcpBt_ResetReceiver();
}

void XcpTl_MainFunction(void) {
    XcpBt_UpdateConnectionState();
    if (!XcpBt_Connected) {
        return;
    }
    XcpBt_PumpRx();
    if (XcpTl_FrameAvailable(0U, 0U) > 0) {
        XcpTl_RxHandler();
    }
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    if ((buf == nullptr) || (len == 0U) || !XcpBt_Connected) {
        return;
    }

    XCP_TL_ENTER_CRITICAL();
    (void)XcpBt_Serial.write(buf, len);
    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_SaveConnection(void) {
}

void XcpTl_ReleaseConnection(void) {
}

void XcpTl_PrintConnectionInformation(void) {
    DBG_PRINT("XCPonBth -- BluetoothSerial active\n");
}

void XcpTl_RxHandler(void) {
    if (!XcpBt_RxContext.frameReady) {
        return;
    }

    Xcp_CtoIn.len = XcpBt_RxContext.payloadLength;
    memcpy(Xcp_CtoIn.data, XcpBt_RxContext.payload, XcpBt_RxContext.payloadLength);
    Xcp_DispatchCommand(&Xcp_CtoIn);
    XcpBt_ResetReceiver();
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {
    (void)sec;
    (void)usec;
    return (int16_t)(XcpBt_RxContext.frameReady ? 1 : 0);
}

    #else
        #error "XCP_ON_BTH on Arduino currently requires an ESP32 board with BluetoothSerial."
    #endif

#endif /* (XCP_TRANSPORT_LAYER == XCP_ON_BTH) && defined(ARDUINO) */
