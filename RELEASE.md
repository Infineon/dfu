# Device Firmware Update (DFU) Middleware Library 5.1

## What's Included?

For a complete description of the DFU middleware, refer to [README.md](./README.md)
and the [DFU API Reference](https://infineon.github.io/dfu/html/index.html).
The revision history of the DFU middleware is also available in the [API Reference Changelog](https://infineon.github.io/dfu/html/index.html#group_dfu_changelog).
New in this release:

* Added USB CDC transport based on the emUSB-Device middleware for the CAT1A device
* Minor updates in the templates

## Defect Fixes

* Corrected the name of the UART object used in the cyhal_uart_set_baud() function. Now, works correctly the custom baud rate configuring in the UART transport.

## Known Issues

No known issues

## Supported Software and Tools

This version of the DFU middleware was validated for compatibility with the following Software and Tools (add and remove information as needed):

| Software and Tools                        | Version |
| :---------------------------------------  | :----:  |
| ModusToolbox Software Environment         | 3.1.0   |
|  - ModusToolbox Device Configurator       | 2.20    |
|  - Device Firmware Update Host Tool       | 2.0     |
|  - CyMCUElfTool                           | 1.0     |
| GCC Compiler                              | 11.3.1  |
| IAR Compiler                              | 9.30.1  |
| ARM Compiler 6                            | 6.16    |
| mtb-hal-cat1                              | 2.5.4   |

Usage of the CAT1 HAL flow requires mtb-hal-cat1 2.3.0 or higher.
Usage of the MCUBoot flow requires DFU Host Tool 2.0 or higher.
Usage of the CDC Trasnport based on the emUSB-Device middleware requires core-make 3.0.2 or higher.

## More information

* [README.md](./README.md)

---
© Cypress Semiconductor Corporation (an Infineon company), 2023.
