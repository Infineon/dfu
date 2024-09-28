# Device Firmware Update (DFU) Middleware Library 5.2

## What's Included?

For a complete description of the DFU middleware, refer to [README.md](./README.md)
and the [DFU API Reference](https://infineon.github.io/dfu/html/index.html).
The revision history of the DFU middleware is also available in the [API Reference Changelog](https://infineon.github.io/dfu/html/index.html#group_dfu_changelog).
New in this release:

* Added USB HID transport based on the emUSB-Device middleware for the CAT1A device
* Added CANFD transport based on the PDL driver for the CAT1C device
* Minor updates in the templates

**Note** Device Firmware Update Host Tool 2.20 does not support CANFD transport. The CANFD support is planned to be added later.

## Defect Fixes

* Fixed address validation for CAT1C device

## Known Issues

No known issues

## Supported Software and Tools

This version of the DFU middleware was validated for compatibility with the following Software and Tools (add and remove information as needed):

| Software and Tools                        | Version |
| :---------------------------------------  | :----:  |
| ModusToolbox Software Environment         | 3.2.0   |
|  - ModusToolbox Device Configurator       | 4.20    |
|  - Device Firmware Update Host Tool       | 2.20    |
|  - CyMCUElfTool                           | 1.0     |
| GCC Compiler                              | 11.3.1  |
| IAR Compiler                              | 9.40.2  |
| ARM Compiler 6                            | 6.16    |
| mtb-hal-cat1                              | 2.6.1   |

Usage of the CAT1 HAL flow requires mtb-hal-cat1 2.3.0 or higher.
Usage of the MCUBoot flow requires DFU Host Tool 2.0 or higher.
Usage of the CDC Trasnport based on the emUSB-Device middleware requires core-make 3.0.2 or higher.

## More information

* [README.md](./README.md)

---
© Cypress Semiconductor Corporation (an Infineon company), 2023-2024.
