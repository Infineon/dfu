# Device Firmware Update (DFU) Middleware Library 5.0

## What's Included?

For a complete description of the DFU middleware, refer to [README.md](./README.md)
and the [DFU API Reference](https://infineon.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html).
The revision history of the DFU middleware is also available in the [API Reference Changelog](https://infineon.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html#group_dfu_changelog).
New in this release:

* Added support of the MCUBoot flow
* Added support of the transport switching at the run time

## Defect Fixes

No fixes

## Known Issues

No known issues

## Supported Software and Tools

This version of the DFU middleware was validated for compatibility with the following Software and Tools (add and remove information as needed):

| Software and Tools                        | Version |
| :---------------------------------------  | :----:  |
| ModusToolbox Software Environment         | 3.0     |
|  - ModusToolbox Device Configurator       | 2.20    |
|  - Device Firmware Update Host Tool       | 2.0     |
|  - CyMCUElfTool                           | 1.0     |
| GCC Compiler                              | 10.3.1  |
| IAR Compiler                              | 9.30.1  |
| ARM Compiler 6                            | 6.16    |
| mtb-hal-cat1                              | 2.3.0   |

Usage of the CAT1 HAL flow requires mtb-hal-cat1 2.3.0 or higher.
Usage of the MCUBoot flow requires DFU Host Tool 2.0 or higher.

## More information

* [README.md](./README.md)

---
© Cypress Semiconductor Corporation (an Infineon company), 2023.
