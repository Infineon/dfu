# Device Firmware Update (DFU) Middleware Library 6.2

## What's Included?

For a complete description of the DFU middleware, refer to [README.md](./README.md)
and the [DFU API Reference](https://infineon.github.io/dfu/html/index.html).
The full revision history of the DFU middleware is listed in the Changelog section below.

New in this release:

* PMBus Transport Interface Support
* PSOC™ Control C3P(M)7/8 MCUs
* Initial Host Bridge Mode Support (I2C and UART)

## Defect Fixes

No fixes

## Known Issues

No known issues

## Supported Software and Tools

This version of the DFU middleware was validated for compatibility with the following Software and Tools:

| Software and Tools                        | Version |
| :---------------------------------------  | :----:  |
| ModusToolbox tools package                | 3.8.0   |
| GCC Compiler                              | 14.2.1  |
| IAR Compiler                              | 9.70.2  |
| ARM Compiler 6                            | 6.22    |
| mtb-dsl-psc3m8                            | 1.0.0   |
| mtb-dsl-psc3m6                            | 0.5.0   |

Usage of the MCUBoot flow requires DFU Host Tool 2.0 or higher.

## Changelog

| Version | Changes | Reason for Change |
| :-- | :-- | :-- |
| 6.2.100 | Enabled emUSB transport on PSOC&trade; Control devices | Extending the current feature |
| 6.2.0 | Added PMBus transport templates | New transport interface support |
| 6.2.0 | Added initial Host Bridge mode support for I2C and UART | New functionality |
| 6.1.0 | Add support of PSOC Edge E84 MCUs devices | New device support |
| 6.0.0 | Migrate DFU middleware to the HAL Next flow |  |
| 5.2.0 | Added USB HID transport based on the emUSB-Device middleware for the CAT1A device | Extending the current feature |
| 5.2.0 | Added CANFD transport based on the PDL driver for the CAT1C device | Extending the current feature |
| 5.2.0 | Minor updates in the templates | Improved the templates usability |
| 5.2.0 | Fixed address validation for CAT1C device | Bugfix |
| 5.1.0 | Added USB CDC transport based on the emUSB-Device middleware for the CAT1A device | Extending the current feature |
| 5.1.0 | Minor updates in the templates | Improved the templates usability |
| 5.1.0 | Corrected the name of the UART object used in the cyhal_uart_set_baud() function | Now, works correctly the custom baud rate configuring in the UART transport |
| 5.0.0 | Add support of the MCUBoot flow. | New functionality. |
| 5.0.0 | Add support of the transport switching at the run time. | New functionality. |
| 5.0.0 | CAT1 device flash read/write operation and I2C/SPI/UART transport templates updated to use mtb-hal-cat1 drivers instead of mtb-pdl-cat1. | Enhance code portability. |
| 5.0.0 | Removed Cy_DFU_Complete function as not used. | Code cleanup. |
| 5.0.0 | Removed CAT1A BLE transport templates. | BLESS stack is not supported in the MTB 3.0. |
| 4.20 | Added USB CDC transport configuration for the CAT2 PDL. | Add support for the USB interface for the PMG1 device family. |
| 4.20 | Updated timeout time for the CAT1A SPI transport. | Fixed the DFU Host Tool timeout error for the CAT1A SPI transport caused by the incorrect function call (transport_spi.c file, SPI_SpiCyBtldrCommRead() function). |
| 4.20 | Minor documentation update. | Documentation improvement. |
| 4.10 | Added PSoC 4 devices support. | Extended device support. |
| 4.10 | Added MISRA-C:2012 compliance. | MISRA standard compliance. |
| 4.10 | Updated SPI communication timeout granularity. | Fixed SPI communication issue. |
| 4.0 | Updated the linker scripts to use the single pre-compiled CM0p image. The upgradeable part of the image is the CM4 application. | Support ModusToolbox v2.0 build flow. |
| 4.0 | Added the ARM compiler version 6 support (version 5 is not supported). |  |
| 4.0 | Added the USB interface (virtual COM port) transport template. |  |
| 4.0 | Removed the Secure Application Formats support. | Secure Application Formats is not supported in ModusToolbox v2.0 build flow. |
| 4.0 | Fixed the return value for the SYNC command processing. | The SYNC command returned fail after successful execution. |
| 4.0 | Updated the major and minor version defines to follow the naming convention. |  |
| 3.10 | Remove the function prototype from the MDK linker script include file. | Fix the linker error for the MDK compiler. |
| 3.10 | Add BLE transport templates. | Add BLE middleware support. |
| 3.0 | Bootloader SDK is renamed to the DFU (Device Firmware Update) SDK. All API prefixes and file names are renamed accordingly. Added BWC macros to simplify migration. | Avoid the confusion with the device boot-up and OS load. |
| 3.0 | Flattened the organization of the driver source code into the single source directory and the single include directory. | Driver library directory-structure simplification. |
| 2.20 | Add check of application number in Set Application Metadata command processing routine. | Prevent incorrect usage of the Set Application Metadata command. |
| 2.20 | Minor documentation updates | Documentation improvement |
| 2.10 | Moved address and golden image checks from cy_dfu.c to Cy_DFU_WriteData() in dfu_user.c, so the checks can be customized based on application needs. | Allows receiving an update for the running app use case. Improvements made based on usability feedback. Documentation update and clarification. |
| 2.0 | Use the shared RAM for application switching instead of the BACKUP register; Add support of secure application verification; Add support of I2C/SPI/BLE transport protocols; Linker scripts updated for PSoC6 Rev *A devices; Made CRC default application checksum. | To increase functionality. |
| 1.0 | Initial version. |  |

## More information

* [README.md](./README.md)

---
(c) (2023-2026), Infineon Technologies AG, or an affiliate of Infineon
Technologies AG.  All rights reserved.
