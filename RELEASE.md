# Device Firmware Update (DFU) Middleware Library 4.20

## What's Included?

For a complete description of the DFU middleware, refer to [README.md](./README.md)
and the [DFU API Reference](https://infineon.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html).
The revision history of the DFU middleware is also available in the [API Reference Changelog](https://infineon.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html#group_dfu_changelog).
New in this release:

* Added USB CDC transport configuration for the CAT2 PDL.
* Documentation improvement.

## Defect Fixes

* Fixed the DFU Host Tool timeout-error for the CAT1A SPI transport.
See [API Reference Changelog](https://infineon.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html#group_dfu_changelog) for details.

## Known Issues

No known issues

## Supported Software and Tools

This version of the DFU middleware was validated for compatibility with the following Software and Tools (add and remove information as needed):

| Software and Tools                        | Version |
| :---                                      | :----:  |
| ModusToolbox Software Environment         | 2.20    |
| - ModusToolbox Device Configurator        | 2.20    |
| - Device Firmware Update Host Tool        | 1.3     |
| - CyMCUElfTool                            | 1.0     |
| GCC Compiler                              | 9.3.1   |
| IAR Compiler                              | 8.42.2  |
| ARM Compiler 6                            | 6.13    |

## More information

* [README.md](./README.md)

---
© Cypress Semiconductor Corporation (an Infineon company), 2021.
