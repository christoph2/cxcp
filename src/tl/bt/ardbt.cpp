#include "xcp_config.h"

#if (XCP_TRANSPORT_LAYER == XCP_ON_BTH) && defined(ARDUINO)

    #include <Arduino.h>

/*!!! START-INCLUDE-SECTION !!!*/
    #include "xcp.h"
/*!!! END-INCLUDE-SECTION !!!*/

    #if (XCP_ON_BTH_ARDUINO_DRIVER == XCP_ON_BTH_DRIVER_AUTO)
        #if defined(ESP32)
            #define XCP_ON_BTH_ACTIVE_DRIVER XCP_ON_BTH_DRIVER_ESP32_SPP
        #elif defined(ARDUINO_ARCH_MBED)
            #define XCP_ON_BTH_ACTIVE_DRIVER XCP_ON_BTH_DRIVER_ARDUINO_BLE
        #elif defined(__has_include)
            #if __has_include(<ArduinoBLE.h>)
                #define XCP_ON_BTH_ACTIVE_DRIVER XCP_ON_BTH_DRIVER_ARDUINO_BLE
            #else
                #error "XCP_ON_BTH requires either ESP32 BluetoothSerial or ArduinoBLE support."
            #endif
        #else
            #error "XCP_ON_BTH requires either ESP32 BluetoothSerial or ArduinoBLE support."
        #endif
    #else
        #define XCP_ON_BTH_ACTIVE_DRIVER XCP_ON_BTH_ARDUINO_DRIVER
    #endif

    #if XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ESP32_SPP
        #include <BluetoothSerial.h>
        #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
            #error "XCP_ON_BTH on ESP32 requires Classic Bluetooth (CONFIG_BT_ENABLED + CONFIG_BLUEDROID_ENABLED)."
        #endif
    #elif XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ARDUINO_BLE
        #include <ArduinoBLE.h>
    #else
        #error "Unknown XCP_ON_BTH Arduino backend selected."
    #endif

typedef enum tagXcpBth_RxStateType {
    XCP_BTH_RX_HEADER,
    XCP_BTH_RX_PAYLOAD
} XcpBth_RxStateType;

typedef struct tagXcpBth_RxContextType {
    XcpBth_RxStateType state;
    uint8_t            header[XCP_ETH_HEADER_SIZE];
    uint8_t            payload[XCP_MAX_CTO];
    uint16_t           headerIndex;
    uint16_t           payloadIndex;
    uint16_t           payloadLength;
    bool               frameReady;
} XcpBth_RxContextType;

static bool                 XcpBth_Connected = false;
static XcpBth_RxContextType XcpBth_RxContext = { XCP_BTH_RX_HEADER, { 0U }, { 0U }, 0U, 0U, 0U, false };

static void XcpBth_ResetReceiver(void) {
    XcpBth_RxContext.state         = XCP_BTH_RX_HEADER;
    XcpBth_RxContext.headerIndex   = 0U;
    XcpBth_RxContext.payloadIndex  = 0U;
    XcpBth_RxContext.payloadLength = 0U;
    XcpBth_RxContext.frameReady    = false;
}

static void XcpBth_PushByte(uint8_t byteValue) {
    if (XcpBth_RxContext.frameReady) {
        return;
    }

    if (XcpBth_RxContext.state == XCP_BTH_RX_HEADER) {
        XcpBth_RxContext.header[XcpBth_RxContext.headerIndex++] = byteValue;
        if (XcpBth_RxContext.headerIndex == XCP_ETH_HEADER_SIZE) {
            XcpBth_RxContext.payloadLength = XCP_MAKEWORD(XcpBth_RxContext.header[1], XcpBth_RxContext.header[0]);
            if ((XcpBth_RxContext.payloadLength == 0U) ||
                (XcpBth_RxContext.payloadLength > (uint16_t)sizeof(XcpBth_RxContext.payload))) {
                XcpBth_ResetReceiver();
                return;
            }
            XcpBth_RxContext.state        = XCP_BTH_RX_PAYLOAD;
            XcpBth_RxContext.payloadIndex = 0U;
        }
    } else {
        XcpBth_RxContext.payload[XcpBth_RxContext.payloadIndex++] = byteValue;
        if (XcpBth_RxContext.payloadIndex == XcpBth_RxContext.payloadLength) {
            XcpBth_RxContext.frameReady = true;
        }
    }
}

static void XcpBth_PushBuffer(uint8_t const *buffer, uint16_t length) {
    uint16_t idx;

    if (buffer == nullptr) {
        return;
    }
    for (idx = 0U; idx < length; ++idx) {
        XcpBth_PushByte(buffer[idx]);
        if (XcpBth_RxContext.frameReady) {
            break;
        }
    }
}

    #if XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ESP32_SPP
static BluetoothSerial XcpBt_Serial;

static void XcpBt_UpdateConnectionState(void) {
    bool connected = XcpBt_Serial.hasClient() && XcpBt_Serial.connected(0);

    if (connected && !XcpBth_Connected) {
        XcpBth_Connected = true;
        XcpTl_SaveConnection();
        XcpBth_ResetReceiver();
    } else if (!connected && XcpBth_Connected) {
        XcpBth_Connected = false;
        XcpTl_ReleaseConnection();
        XcpBth_ResetReceiver();
    }
}

static void XcpBt_PumpRx(void) {
    while ((XcpBt_Serial.available() > 0) && !XcpBth_RxContext.frameReady) {
        int byteRead = XcpBt_Serial.read();
        if (byteRead < 0) {
            break;
        }
        XcpBth_PushByte((uint8_t)byteRead);
    }
}

    #elif XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ARDUINO_BLE
static BLEService        XcpBleService(XCP_ON_BTH_BLE_SERVICE_UUID);
static BLECharacteristic XcpBleRx(XCP_ON_BTH_BLE_RX_UUID, BLEWrite | BLEWriteWithoutResponse, XCP_COMM_BUFLEN);
static BLECharacteristic XcpBleTx(XCP_ON_BTH_BLE_TX_UUID, BLENotify, XCP_COMM_BUFLEN);

static void XcpBle_UpdateConnectionState(void) {
    bool connected = BLE.connected();

    if (connected && !XcpBth_Connected) {
        XcpBth_Connected = true;
        XcpTl_SaveConnection();
        XcpBth_ResetReceiver();
    } else if (!connected && XcpBth_Connected) {
        XcpBth_Connected = false;
        XcpTl_ReleaseConnection();
        XcpBth_ResetReceiver();
        BLE.advertise();
    }
}

static void XcpBle_PumpRx(void) {
    uint8_t rxBuffer[XCP_COMM_BUFLEN];
    int     bytesRead;

    if (!XcpBleRx.written()) {
        return;
    }

    bytesRead = XcpBleRx.readValue(rxBuffer, (int)sizeof(rxBuffer));
    if (bytesRead > 0) {
        XcpBth_PushBuffer(rxBuffer, (uint16_t)bytesRead);
    }
}
    #endif

void XcpTl_Init(void) {
    #if XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ESP32_SPP
    if (!XcpBt_Serial.begin(XCP_ON_BTH_DEVICE_NAME)) {
        DBG_PRINT("XcpTl_Init: BluetoothSerial.begin failed\n");
        return;
    }
    if ((XCP_ON_BTH_PIN[0] != '\0') && !XcpBt_Serial.setPin(XCP_ON_BTH_PIN, (uint8_t)strlen(XCP_ON_BTH_PIN))) {
        DBG_PRINT("XcpTl_Init: BluetoothSerial.setPin failed\n");
    }
    #elif XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ARDUINO_BLE
    if (!BLE.begin()) {
        DBG_PRINT("XcpTl_Init: ArduinoBLE.begin failed\n");
        return;
    }

    XcpBleService.addCharacteristic(XcpBleRx);
    XcpBleService.addCharacteristic(XcpBleTx);
    BLE.addService(XcpBleService);
    BLE.setLocalName(XCP_ON_BTH_DEVICE_NAME);
    BLE.setDeviceName(XCP_ON_BTH_DEVICE_NAME);
    BLE.setAdvertisedService(XcpBleService);
    XcpBleTx.writeValue((const uint8_t *)"", 0);
    BLE.advertise();
    #endif

    XcpBth_Connected = false;
    XcpBth_ResetReceiver();
}

void XcpTl_DeInit(void) {
    #if XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ESP32_SPP
    XcpBt_Serial.end();
    #elif XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ARDUINO_BLE
    BLE.stopAdvertise();
    BLE.end();
    #endif

    XcpBth_Connected = false;
    XcpBth_ResetReceiver();
}

void XcpTl_MainFunction(void) {
    #if XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ESP32_SPP
    XcpBt_UpdateConnectionState();
    #elif XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ARDUINO_BLE
    BLE.poll();
    XcpBle_UpdateConnectionState();
    #endif

    if (!XcpBth_Connected) {
        return;
    }

    #if XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ESP32_SPP
    XcpBt_PumpRx();
    #elif XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ARDUINO_BLE
    XcpBle_PumpRx();
    #endif

    if (XcpTl_FrameAvailable(0U, 0U) > 0) {
        XcpTl_RxHandler();
    }
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    if ((buf == nullptr) || (len == 0U) || !XcpBth_Connected) {
        return;
    }

    XCP_TL_ENTER_CRITICAL();
    #if XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ESP32_SPP
    (void)XcpBt_Serial.write(buf, len);
    #elif XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ARDUINO_BLE
    (void)XcpBleTx.writeValue(buf, (int)len);
    #endif
    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_SaveConnection(void) {
}

void XcpTl_ReleaseConnection(void) {
}

void XcpTl_PrintConnectionInformation(void) {
    #if XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ESP32_SPP
    DBG_PRINT("XCPonBth -- BluetoothSerial active\n");
    #elif XCP_ON_BTH_ACTIVE_DRIVER == XCP_ON_BTH_DRIVER_ARDUINO_BLE
    DBG_PRINT("XCPonBth -- ArduinoBLE active (%s)\n", XCP_ON_BTH_DEVICE_NAME);
    #endif
}

void XcpTl_RxHandler(void) {
    if (!XcpBth_RxContext.frameReady) {
        return;
    }

    Xcp_CtoIn.len = XcpBth_RxContext.payloadLength;
    memcpy(Xcp_CtoIn.data, XcpBth_RxContext.payload, XcpBth_RxContext.payloadLength);
    Xcp_DispatchCommand(&Xcp_CtoIn);
    XcpBth_ResetReceiver();
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {
    (void)sec;
    (void)usec;
    return (int16_t)(XcpBth_RxContext.frameReady ? 1 : 0);
}

#endif /* (XCP_TRANSPORT_LAYER == XCP_ON_BTH) && defined(ARDUINO) */
