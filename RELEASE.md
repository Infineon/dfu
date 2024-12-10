# Device Firmware Update (DFU) Middleware Library 6.0

## What's Included?

For a complete description of the DFU middleware, refer to [README.md](./README.md)
and the [DFU API Reference](https://infineon.github.io/dfu/html/index.html).
The revision history of the DFU middleware is also available in the [API Reference Changelog](https://infineon.github.io/dfu/html/index.html#group_dfu_changelog).

New in this release:
* Migration DFU middleware to HAL-Next flow
* Adding support of PSOC Control C3 device
* Temporary removed support of devices not compatible with the  HAL-Next flow (PSOC 4, PSOC 6, XMC7000)

The DFU middleware 5.2.0 release to be used with the classic HAL flow and devices dropped from the 6.0 version (PSOC 4, PSOC 6, XMC7000).

## Defect Fixes

No fixes

## Known Issues

No known issues

## Supported Software and Tools

This version of the DFU middleware was validated for compatibility with the following Software and Tools (add and remove information as needed):

| Software and Tools                        | Version |
| :---------------------------------------  | :----:  |
| ModusToolbox Software Environment         | 3.3.0   |
|  - ModusToolbox Device Configurator       | 5.10    |
|  - Device Firmware Update Host Tool       | 2.50    |
| GCC Compiler                              | 11.3.1  |
| IAR Compiler                              | 9.50.2  |
| ARM Compiler 6                            | 6.22    |
| mtb-pdl-cat1                              | 3.13.0  |
| mtb-hal-psc3                              | 1.0.0   |


Usage of the MCUBoot flow requires DFU Host Tool 2.0 or higher.

## More information

* [README.md](./README.md)

---
© Cypress Semiconductor Corporation (an Infineon company), 2023-2024.
