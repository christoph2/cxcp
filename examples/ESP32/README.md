| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- |


# XCP on ESP32 example

(See the README.md file in the upper level `examples` directory for more information about examples.)

The application boots the XCP core, brings up the selected network interface, and starts the XCP transport and protocol tasks on FreeRTOS.

## How to use the example

Connect an XCP master to the ESP32 using the configured transport settings and port. The default setup uses UDP on port `5555`.

Refer to the [upper level README](../README.md#host-tools) for more information about suitable host tools.

## Hardware Required

This example can be run on any commonly available ESP32 development board.

## Configure the project

```
idf.py menuconfig
```

Set following parameters under Example Configuration Options:

* Set `IP version` of the example to be IPV4 or IPV6.

* Set `Port` number that the XCP slave will listen on.

* Optionally enable `XCP discovery multicast`.

* Tune the XCP core, transport, and terminal FreeRTOS task stack sizes and priorities.

Configure Wi-Fi or Ethernet under "Example Connection Configuration" menu. See "Establishing Wi-Fi or Ethernet Connection" section in [examples/protocols/README.md](../../README.md) for more details.

## Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.


## Troubleshooting

Start server first, to receive data sent from the client (application).
