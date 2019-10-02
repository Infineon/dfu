### PSoC 6 Device Firmware Update (DFU) Middleware Library 4.0
 
### What's Included?

For a complete description of the DFU middleware, refer to [README.md](./README.md)
and the [DFU API Reference](https://cypresssemiconductorco.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html).
The revision history of the DFU middleware is also available in the [API Reference Changelog](https://cypresssemiconductorco.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html#group_dfu_changelog).
New in this release:

* Updated the linker scripts to use the single pre-compiled CM0p image.
  The upgradeable part of the image is the CM4 application.
* Added the ModusToolbox 2.0 flow support.
* Added the ARM compiler version 6 support.
* Added the USB interface (virtual COM) transport.

### Defect Fixes

* Fixed the return value for the SYNC command processing.

### Known Issues

No known issues

### Supported Software and Tools

This version of the DFU middleware was validated for compatibility with the following Software and Tools (add and remove information as needed):

| Software and Tools                        | Version |
| :---                                      | :----:  |
| ModusToolbox Software Environment         | 2.0     |
| - ModusToolbox Device Configurator        | 2.0     |
| - Device Firmware Update Host Tool        | 1.1     |
| - CyMCUElfTool                            | 1.0     |
| GCC Compiler                              | 7.2.1   |
| IAR Compiler                              | 8.32    |
| ARM Compiler 6                            | 6.11    |

### More information

* [README.md](./README.md)
* [DFU Middleware API Reference Guide](https://cypresssemiconductorco.github.io/dfu/dfu_sdk_api_reference_manual/html/index.html)
* [AN213924](http://www.cypress.com/an213924) DFU SDK User Guide
* [CE213903](http://www.cypress.com/ce213903) DFU SDK Basic Communication Code Examples
* [CE216767](http://www.cypress.com/ce216767) DFU SDK BLE OTA Code Example
* [PSoC 6 Peripheral Driver Library API Reference](https://cypresssemiconductorco.github.io/psoc6pdl/pdl_api_reference_manual/html/index.html)
* [Cypress USB Device Middleware Library API Reference](https://cypresssemiconductorco.github.io/usbdev/usbfs_dev_api_reference_manual/html/index.html)
* [BLE Middleware API Reference Guide](https://cypresssemiconductorco.github.io/middleware-ble/ble_api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [PSoC 6 SDK Examples](https://github.com/cypresssemiconductorco/Code-Examples-for-the-ModusToolbox-PSoC-6-SDK)
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
* [Cypress Semiconductor](http://www.cypress.com)

---
© Cypress Semiconductor Corporation, 2019.
