# Device Firmware Update (DFU) Middleware Library

## Overview

 The purpose of the DFU middleware library is to provide an SDK for updating
 firmware images. The middleware allows creating two types of projects:

1. An application loader receives the program and switch to
   the new application.
2. A loadable application to be transferred and programmed.

 A project can contain features of both first and second type.

## Features

* Read firmware images from a host through a number of transport interfaces,
  e.g. BLE, USB, UART, I2C, SPI.
* Program a firmware image to the specified address in internal flash,
  XIP region, or any external memory that supports the DFU API.
* Copy applications.
* Validate applications.
* Safe Update: updates at a temporary location, validates, and if valid,
  overwrites a working image.
* Switches applications. Passes parameters in RAM when switching
  applications.
* Supports encrypted image files.
  Transfers encrypted images without decrypting in the middle.
* Supports many application images, the number of which is limited by
  the metadata size. Each image can be an application loader. For example,
  512-byte metadata supports up to 63 applications.
* Supports customization.
* Supports the CRC-32 checksum to validate data.

## DFU Specific Instructions

The DFU middleware requires configuration files located in the /config
directory. The directory /config is skipped from a build process, so
copy the configuration files manually. The application loader requires
dfu_user.h/c, transport files (e.g. for loading with UART - transport_uart.c/h).
The loadable application requires dfu_user.h file.
For a build, use the dfu linker scripts located in the linker_script directory.

## Quick Start

The [Quick Start section of the DFU Middleware API Reference Guide](https://infineon.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html#section_dfu_quick_start)
describes step-by-step instructions to set up a DFU application.

## More information

For more information, refer to the following links:

* [DFU Middleware Library Release Notes](./RELEASE.md)
* [DFU Middleware Library API Reference](https://infineon.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html)
* [AN213924](http://www.cypress.com/an213924) DFU SDK User Guide
* [CE213903](http://www.cypress.com/ce213903) DFU SDK Basic Communication Code Examples
* [CE216767](http://www.cypress.com/ce216767) DFU SDK BLE OTA Code Example
* [CAT1 Peripheral Driver Library API Reference](https://infineon.github.io/mtb-pdl-cat1/pdl_api_reference_manual/html/index.html)
* [CAT2 Peripheral Driver Library API Reference](https://infineon.github.io/mtb-pdl-cat2/pdl_api_reference_manual/html/index.html)
* [USB Device Middleware Library API Reference](https://infineon.github.io/usbdev/usbfs_dev_api_reference_manual/html/index.html)
* [PSoC 6 BLE Middleware Library API Reference](https://infineon.github.io/bless/ble_api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [Code Examples for ModusToolbox Software](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software)
* [ModusToolbox Device Configurator Tool Guide](https://www.cypress.com/ModusToolboxDeviceConfig)
* [ModusToolbox Device Firmware Update Host Tool](https://www.cypress.com/ModusToolboxDFUHostTool)
* [ModusToolbox USB Configurator Tool Guide](https://www.cypress.com/ModusToolboxUSBConfig)
* [ModusToolbox BT Configurator Tool Guide](https://www.cypress.com/ModusToolboxBLEConfig)
* [CySmart - BLE Test and Debug Tool](http://www.cypress.com/documentation/software-and-drivers/cysmart-bluetooth-le-test-and-debug-tool)
* [CySmart – Mobile App](https://www.cypress.com/documentation/software-and-drivers/cysmart-mobile-app)
* [PSoC 6 BLE Pioneer Kit](http://www.cypress.com/CY8CKIT-062-BLE)
* [PSoC 6 WiFi-BT Pioneer Kit](http://www.cypress.com/CY8CKIT-062-WiFi-BT)
* [PSoC 6 Wi-Fi BT Prototyping Kit](http://www.cypress.com/cy8cproto-062-4343w)
* [PSoC 6 MCU Datasheets](http://www.cypress.com/psoc6ds)
* [PSoC 6 MCU Application Notes](http://www.cypress.com/psoc6an)
* [PSoC 6 MCU Technical Reference Manuals](http://www.cypress.com/psoc6trm)
* [PSoC 4 Technical Reference Manuals](https://www.cypress.com/search/all?f%5B0%5D=meta_type%3Atechnical_documents&f%5B1%5D=resource_meta_type%3A583&f%5B2%5D=field_related_products%3A1314)
* [PSoC 4 MCU Datasheets](https://www.cypress.com/search/all?f%5B0%5D=meta_type%3Atechnical_documents&f%5B1%5D=field_related_products%3A1297&f%5B2%5D=resource_meta_type%3A575)
* [Cypress Semiconductor](http://www.cypress.com)

---
© Cypress Semiconductor Corporation (an Infineon company), 2021.
