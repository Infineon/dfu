# Device Firmware Update (DFU) Middleware Library

## Overview

 The purpose of the DFU middleware library is to provide an SDK for updating
 firmware images. The middleware allows creating these types of projects:

- An application loader receives the program and switch to
   the new application.
- A loadable application to be transferred and programmed.

 A project can contain the features of both types.

## Features

- Reads firmware images from a host through a number of transport interfaces,
  e.g. USB, UART, I2C, SPI
  Supports dynamic switching (during runtime) of the communication interfaces
- Provides ready-for-use transport interface templates based on HAL drivers for
  PDL drivers for CAT1 devices and CAT2 devices
- Supported flows: Basic bootloader and MCUBoot compatibility
- Device support: CAT1A, CAT2 (Basic bootloader flow),
  CAT1C (MCUBoot compatibility flow)
- Programs a firmware image to the specified address in internal flash,
  XIP region or any external memory that supports the DFU API
- Copies applications
- Validates applications
- Updates safely - updates at a temporary location, validates, and if valid,
  overwrites the working image
- Switches applications - passes parameters in RAM when switching
  applications
- Supports encrypted image files - transfers encrypted images without decrypting
  in the middle
- Supports many application images - the number of applications is limited by
  the metadata size; each image can be an application loader, for example,
  512-byte metadata supports up to 63 applications
- Supports customization
- Supports the CRC-32 checksum to validate data.

## DFU Specific Instructions

The DFU middleware requires configuration files which should be enabled by
components in the Makefile. The application loader requires DFU_USER component,
transport components (e.g. for loading with UART - DFU_UART).
The loadable application does not require specific component and dfu_user.h file
includes with DFU middleware. The configuration files is copies in the application
and could be updated based on the specific use cases.
For a build with a the basic bootloader flow, use the dfu linker scripts located
in the linker_script directory.

## Quick Start

The [Quick Start section of the DFU Middleware API Reference Guide](https://infineon.github.io/dfu/html/index.html#section_dfu_quick_start)
describes step-by-step instructions to set up a DFU application.

## More information

For more information, refer to the following links:

- [DFU Middleware Library Release Notes](./RELEASE.md)
- [DFU Middleware Library API Reference](https://infineon.github.io/dfu/html/index.html)
- [AN213924](https://www.infineon.com/an213924) DFU SDK User Guide
- [CE213903](https://www.infineon.com/ce213903) DFU SDK Basic Communication Code Examples
- [CAT1 Peripheral Driver Library API Reference](https://infineon.github.io/mtb-pdl-cat1/pdl_api_reference_manual/html/index.html)
- [CAT2 Peripheral Driver Library API Reference](https://infineon.github.io/mtb-pdl-cat2/pdl_api_reference_manual/html/index.html)
- [USB Device Middleware Library API Reference](https://infineon.github.io/usbdev/usbfs_dev_api_reference_manual/html/index.html)
- [Code Examples for ModusToolbox Software](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software)
- [ModusToolbox Device Configurator Tool Guide](https://www.infineon.com/ModusToolboxDeviceConfig)
- [ModusToolbox Device Firmware Update Host Tool](https://www.infineon.com/ModusToolboxDFUHostTool)
- [ModusToolbox USB Configurator Tool Guide](https://www.infineon.com/ModusToolboxUSBConfig)
- [PSoC 6 BLE Pioneer Kit](https://www.infineon.com/CY8CKIT-062-BLE)
- [PSoC 6 WiFi-BT Pioneer Kit](https://www.infineon.com/CY8CKIT-062-WiFi-BT)
- [PSoC 6 Wi-Fi BT Prototyping Kit](https://www.infineon.com/cy8cproto-062-4343w)
- [XMC7200 evaluation kit](https://www.infineon.com/KIT_XMC72_EVK)
- [PSoC 6 MCU Datasheets](https://www.infineon.com/cms/en/search.html#!view=downloads&term=psoc6&doc_group=Data%20Sheet)
- [PSoC 6 MCU Technical Reference Manuals](https://www.infineon.com/cms/en/search.html#!view=downloads&term=psoc6&doc_group=Additional%20Technical%20Information)
- [PSoC 4 product page](https://www.infineon.com/cms/en/product/microcontroller/32-bit-psoc-arm-cortex-microcontroller/psoc-4-32-bit-arm-cortex-m0-mcu)
- [Infineon Technologies AG](https://www.infineon.com)

---
© Cypress Semiconductor Corporation (an Infineon company), 2023.
