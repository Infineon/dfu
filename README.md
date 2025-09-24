# Device Firmware Update (DFU) Middleware Library

## Overview

The purpose of the DFU middleware library is to provide an SDK for updating
firmware images. The middleware allows creating these types of projects:
- The application loader receives an image and programs it into memory.
- A loadable application is transferred and programmed.

A project can contain the features of both types.

## Features

- Reads firmware images from a host through a number of transport interfaces, e.g. USB, UART, I2C, SPI, CANFD
- Supports dynamic switching (during runtime) of the communication interfaces
- Provides ready-for-use transport interface templates based on HAL/PDL drivers for PSOC Control C3 and PSE84 devices
- Supported flow: MCUBoot compatibility
- Device support: PSOC Control C3 and PSE84
- Programs a firmware image to the specified address in internal flash, XIP region or any external memory that supports the DFU API
- Validates applications
- Supports encrypted image files - transfers encrypted images without decrypting in the middle
- Supports customization
- Supports the CRC-32 checksum to validate data.
- Supports extend of the host command/response protocol with custom commands.

## DFU Specific Instructions

The DFU middleware requires configuration files which should be enabled by
components in the Makefile. The application loader requires DFU_USER component,
transport components (e.g. for loading with UART - DFU_UART).
The loadable application does not require specific component and dfu_user.h file
includes with DFU middleware. The configuration files is copies in the application
and could be updated based on the specific use cases.

## Quick Start

The [Quick Start section of the DFU Middleware API Reference Guide](https://infineon.github.io/dfu/html/index.html#section_dfu_quick_start)
describes step-by-step instructions to set up a DFU application.

## More information

For more information, refer to the following links:

- [DFU Middleware Library Release Notes](./RELEASE.md)
- [DFU Middleware Library API Reference](https://infineon.github.io/dfu/html/index.html)
- [AN213924](https://www.infineon.com/an213924) DFU SDK User Guide
- [CE213903](https://www.infineon.com/ce213903) DFU SDK Basic Communication Code Examples
- [Peripheral Driver Library API Reference](https://infineon.github.io/mtb-pdl-cat1/pdl_api_reference_manual/html/index.html), Note: PDL CAT1 is used for PSOC Control C3 device only.
- [Code Examples for ModusToolbox Software](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software)
- [ModusToolbox Device Configurator Tool Guide](https://www.infineon.com/ModusToolboxDeviceConfig)
- [ModusToolbox Device Firmware Update Host Tool](https://www.infineon.com/ModusToolboxDFUHostTool)
- [Infineon Technologies AG](https://www.infineon.com)

---
© Cypress Semiconductor Corporation (an Infineon company), 2025.
