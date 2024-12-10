/***************************************************************************//**
* \file cy_dfu.h
* \version 6.0
*
* Provides API declarations for the DFU Middleware.
*
********************************************************************************
* \copyright
* (c) (2016-2024), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation. All rights reserved.
********************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation or one of its
* affiliates ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#if !defined(CY_DFU_H)
#define CY_DFU_H

/**
* \mainpage
*
* \section section_mainpage_overview Overview
*
* The purpose of the DFU middleware library is to provide an SDK for updating
* firmware images. The middleware allows creating these types of projects:
*
* - The application loader receives an image and programs it into memory.
* - A loadable application is transferred and programmed.
*
* A project can contain the features of both types.
*
********************************************************************************
* \section section_dfu_general General Description
********************************************************************************
*
* Include cy_dfu.h to get access to all functions and other declarations in this
* library.
*
* The DFU SDK has the following features:
* - Reads firmware images from a host through a number of transport interfaces,
*   e.g. UART, I2C, SPI, CANFD
* - Supports dynamic switching (during runtime) of the communication interfaces
* - Provides ready-for-use transport interface templates based on HAL/PDL drivers
*   for CAT1 devices
* - Supported flow: MCUBoot compatibility
* - Device support: CAT1B (PSOC Control C3)
* - Programs a firmware image to the specified address in internal flash,
*   XIP region or any external memory that supports the DFU API
* - Validates applications
* - Supports encrypted image files - transfers encrypted images without
*   decrypting in the middle
* - Supports customization
* - Supports the CRC-32 checksum to validate data.
* - Supports extend of the host command/response protocol with custom commands.
*
********************************************************************************
* \section section_dfu_quick_start Quick Start Guide
********************************************************************************
********************************************************************************
* \subsection subsection_dfu_qsg_mcuboot DFU Transport (MCUBoot compatible) flow
********************************************************************************
*
* \subsubsection subsubsection_qsg_mcuboot_description Description
* The DFU supports the usage of the MCUBoot as a bootloader and provides a transport layer
* for transferring a new application image to the slot.
* Set macro CY_DFU_FLOW to CY_DFU_MCUBOOT_FLOW to enable this flow.
*
* \subsubsection subsubsection_qsg_mcuboot_s1 STEP1: Projects preparation.
*
* 1. Create a ModusToolbox&trade; application for the CAT1B device.\n
*   For example:
*   - the KIT_PSC3M5_EVK can be used as CAT1B;
*
*   Create a new application in the ModusToolbox&trade; IDE using an appropriate BSP
*   and an empty application as a template (Empty App). Name it "DFU_App0". For details, refer to the
*   ModusToolbox&trade; 3.x IDE Quick Start Guide.
*
*   \note The ModusToolbox&trade; PSOC Edge Protect Bootloader application
*   must be used as a base project for the KIT_PSC3M5_EVK application loader.
*   Please refer to the project documentation for set-up and configuration.
*
* 2. Include the DFU middleware into the project using the ModusToolbox&trade; Library
*    Manager.
*
* 3. Add the DFU transport components to project's Makefile to enable the transport interface(s).
*    In our case, I2C is used:
*   \code COMPONENTS += DFU_I2C \endcode
*
* 4. Add the DFU User component:
*   \code COMPONENTS += DFU_USER \endcode
*
* 5. Update project's Makefile to use MCUBoot flow:
*   \code DEFINES += CY_DFU_FLOW=CY_DFU_MCUBOOT_FLOW \endcode
*
* \subsubsection subsubsection_qsg_mcuboot_s2 STEP2: Add DFU logic to main.c
*
* 1. Include the required headers:
*    \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_INCLUDE
* 2. Initialize the variables:
*    \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_INIT_VAR
* 3. Call the DFU initialization function:
*    \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_INIT_FUNC
* 4. Initialize the DFU transport layer. Refer to the \ref group_dfu_ucase_i2c section.
* 5. Start the I2C transport:
*    \snippet snippet/source/COMPONENT_DFU_I2C/i2c_transport_snippet.c DFU_I2C_START
* 6. Update the main loop with the Host Command/Response protocol processing:
*  \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_CMD_PROCESS
*
* \note To use DFU logging, initialize the retarget-io middleware
*
* \subsubsection subsubsection_qsg_mcuboot_s3 STEP3: Build and Program Loader Application
* Connect your kit to the computer. Build and program the device.
* \note The CY_DFU_PRODUCT warning displays if default values are used and they need to be
*       changed. CY_DFU_PRODUCT can be defined in the Makefile.
*
* \subsubsection subsubsection_qsg_mcuboot_s4 STEP4: Create a loadable application (Application 1).
* 1. Create a ModusToolbox&trade; application for the same devices as in STEP1. Use an empty
*   application as a template (Empty App). Name it "DFU_App1".
*
* 2. Update the project post build steps to generate HEX files with an offset
*   to the memory region of the loadable application.
*
* - Added Makefile variables for generating the HEX file.
*
* ~~~ makefile
* BINARY_PATH=./build/$(TARGET)/$(CONFIG)/$(APPNAME)
* HEX_TOOL=$(MTB_TOOLCHAIN_GCC_ARM__BASE_DIR)/bin/arm-none-eabi-objcopy
* HEX_TOOL_OPTIONS=-O ihex
* APP_OFFSET=0x00030000
* ~~~
*
* - Add a post build step to put the loadable application at the upgradable area offset
*
* ~~~ makefile
* # Custom post-build commands to run.
* POSTBUILD=\
* cp -f $(BINARY_PATH).hex $(BINARY_PATH)_raw.hex;\
* rm -f $(BINARY_PATH).hex;\
* $(HEX_TOOL) --change-addresses=$(APP_OFFSET) $(HEX_TOOL_OPTIONS) $(BINARY_PATH).elf $(BINARY_PATH).hex;
* ~~~
*
* 3. Load the application to the device using the DFU Host Tool. Refer to the DFU Host
*   Tool user guide for the details of using the HEX file as an input.
* \note Only DFU Host Tool v2.0 or later support the HEX file as an input.
*
********************************************************************************
* \section section_dfu_design Design Considerations
********************************************************************************
*
* Supported transports:
*
* <table class="doxtable">
*   <tr><th>Devices</th><th>I2C (HAL-Next)</th><th>UART (HAL-Next)</th><th>SPI (HAL-Next)</th><th>CANFD (PDL)</th></tr>
*   <tr>
*     <td>CAT1B (PSOC Control C3)</td>
*     <td>Supported</td>
*     <td>Supported</td>
*     <td>Supported</td>
*     <td>Supported</td>
*   </tr>
* </table>
*
********************************************************************************
* \subsection group_dfu_ucase_hal_next Firmware Update via transports based on HAL-Next
********************************************************************************
*
* The next transports support HAL-Next flow:
* - I2C
* - UART
* - SPI
*
* To configure transport based on HAL-Next flow:
* - Add to **COMPONENTS** varialbe in the Makefile **DFU_<transport name>** (Example: **DFU_UART**,
* **DFU_I2C**, etc)
* - Configure the communication protocol in Device Configurator. The Device Configurator
* will generate appropriate configuration structures and macro for the protocol
* initialization by PDL and HAL APIs. The DFU middleware does not re-configure
* any protocol settings like baudrate, data width, address, etc.
* - Configure the HW using PDL initialization APIs. For some transport,
* configure the interrupt too.
* - Setup HAL driver by appropriate HAL APIs
* - Create the DFU transport callback function. Typically, this function enables
* or disables HW by PDL APIs. For some cases, this function makes
* full HW initialization instead of only enabling/disabling. This callback will
* be automatically called by DFU middleware during the protocol selection.
* - Create transport DFU initialization structure. This structure has
* pointers to the HAL driver object and DFU transport callback.
*
* \note Check the next subsections for code snippets with transports configuration
*
********************************************************************************
* \subsubsection group_dfu_ucase_i2c Firmware Update via I2C
********************************************************************************
*
* The I2C specific configuration options:
* - The size of the buffer for sending and receiving data is configured by macro.
* By default the 128 bytes are selected but the size can be re-configured in the
* Makefile. Add DFU_I2C_TX_BUFFER_SIZE and DFU_I2C_RX_BUFFER_SIZE to the
* define variable and assign new values:
* \code DEFINES+=DFU_I2C_TX_BUFFER_SIZE=64 DFU_I2C_RX_BUFFER_SIZE=64 \endcode
* - The I2C transport requires configured interrupt routine
* - To add transport to build, add **DFU_I2C** to **COMPONENTS** variable in the
* Makefile: \code COMPONENTS+=DFU_I2C \endcode
*
* **The Example of I2C transport configuration:**
* - Include the required header files:
*   -# PDL/HAL drivers specific headers
*   -# cybsp.h - to use generated code from the Device Configurator
*   -# DFU core and transport headers.
* \snippet snippet/source/COMPONENT_DFU_I2C/i2c_transport_snippet.c DFU_I2C_HN_TRANSPORT_INCLUDE
* - Define the global variables - first is the HAL driver object to be provided
* to transport and the second is PDL driver context.
* \snippet snippet/source/COMPONENT_DFU_I2C/i2c_transport_snippet.c DFU_I2C_HN_TRANSPORT_VAR_DEF
* - Create interrupt handler for I2C HW
* \snippet snippet/source/COMPONENT_DFU_I2C/i2c_transport_snippet.c DFU_I2C_HN_TRANSPORT_ISR
* - Create I2C transport callback
* \snippet snippet/source/COMPONENT_DFU_I2C/i2c_transport_snippet.c DFU_I2C_HN_TRANSPORT_CALLBACK
* - Configure I2C HW using PDL initialization APIs, then set up the HAL driver and
* configure the I2C interrupt
* \snippet snippet/source/COMPONENT_DFU_I2C/i2c_transport_snippet.c DFU_I2C_HN_TRANSPORT_CONF
*
* \note Select the I2C HW and configure it in the Device Configurator,
* then, write the next name for the DFU_I2C example.
*
********************************************************************************
* \subsubsection group_dfu_ucase_uart Firmware Update via UART
********************************************************************************
*
* The UART specific configuration options:
* - To add transport to the build, add the **DFU_UART** to **COMPONENTS** variable in the
* Makefile: \code COMPONENTS+=DFU_UART \endcode
*
* **The Example of UART transport configuration:**
* - Include the required header files:
*   -# cybsp.h to use generated code from the Device Configurator
*   -# cybsp.h to use generated code from Device-Configurator
*   -# DFU core and transport headers.
* \snippet snippet/source/COMPONENT_DFU_UART/uart_transport_snippet.c DFU_UART_HN_TRANSPORT_INCLUDE
* - Define the global variables - first is the HAL driver object to be provided
* to transport and the second is PDL driver context.
* \snippet snippet/source/COMPONENT_DFU_UART/uart_transport_snippet.c DFU_UART_HN_TRANSPORT_VAR_DEF
* - Create UART transport callback
* \snippet snippet/source/COMPONENT_DFU_UART/uart_transport_snippet.c DFU_UART_HN_TRANSPORT_CALLBACK
* - Configure UART HW using PDL initialization APIs, then set up the HAL driver and
* configure the UART interrupt
* \snippet snippet/source/COMPONENT_DFU_UART/uart_transport_snippet.c DFU_UART_HN_TRANSPORT_CONF
*
* \note Select the UART HW and configure it in the Device Configurator,
* write  the next name for the DFU_UART example.
*
********************************************************************************
* \subsubsection group_dfu_ucase_spi Firmware Update via SPI
********************************************************************************
*
* The SPI specific configuration options:
* - The SPI transport requires configured interrupt routine
* - To add transport to the build, add the **DFU_SPI** to **COMPONENTS** variable in the
* Makefile: \code COMPONENTS+=DFU_SPI \endcode
*
* **The Example of SPI transport configuration:**
* - Include the required header files:
*   -# PDL/HAL drivers specific headers
*   -# cybsp.h to use generated code from the Device Configurator
*   -# DFU core and transport headers.
* \snippet snippet/source/COMPONENT_DFU_SPI/spi_transport_snippet.c DFU_SPI_HN_TRANSPORT_INCLUDE
* - Define the global variables - first is the HAL driver object to be provided
* to transport and the second is PDL driver context.
* \snippet snippet/source/COMPONENT_DFU_SPI/spi_transport_snippet.c DFU_SPI_HN_TRANSPORT_VAR_DEF
* - Create interrupt handler for SPI HW
* \snippet snippet/source/COMPONENT_DFU_SPI/spi_transport_snippet.c DFU_SPI_HN_TRANSPORT_ISR
* - Create SPI transport callback
* \snippet snippet/source/COMPONENT_DFU_SPI/spi_transport_snippet.c DFU_SPI_HN_TRANSPORT_CALLBACK
* - Configure SPI HW using PDL initialization APIs, then set up the HAL driver and
* configure the SPI interrupt
* \snippet snippet/source/COMPONENT_DFU_SPI/spi_transport_snippet.c DFU_SPI_HN_TRANSPORT_CONF
*
* \note Select the SPI HW and configure it in the Device Configurator,
* write the next name for the DFU_SPI example.
*
********************************************************************************
* \subsection group_dfu_ucase_canfd Firmware Update via CAN FD transport
********************************************************************************
*
* Specific steps for the CAN FD transport support:
* - Add the CAN FD transport components to the project Makefile:
*    \code COMPONENTS+=DFU_CANFD \endcode
* 
* - The CAN FD interrupt priority can be configured using the DFU_CANFD_IRQ_PRIORITY macro,
* for example:
*    \code DEFINES+=DFU_CANFD_IRQ_PRIORITY=3 \endcode
*
********************************************************************************
* \subsubsection group_dfu_mtb_cfg Use of the Device-Configurator&trade; tools for CAN-FD HW initialization
********************************************************************************
*
* To set up the CAN FD personality in the ModusToolbox&trade; Device Configurator
* for the CAN FD DFU transport for PSOC Control C3, see the screenshots
* below. For other devices, verify the CAN Rx and CAN Tx pins connections.
* 
* <b> General settings </b> - set the personality alias and CAN FD mode:
*      Parameter name         | Value                                |
*      -----------------------|--------------------------------------
*      Personality alias name | DFU_CANFD
*      CAN FD Mode            | Enabled
*
* \image html dfu_canfd1.png
* \n
*
* <b> Bitrate settings </b> - configure prescaler, time segments and syncronization jump width:
*      Parameter name         | Value                                |
*      -----------------------|--------------------------------------
*      Nominal Prescaler      | Set according to nominal bitrate setting in the DFU Host Tool
*      Nominal Time Segment 1 | ^
*      Nominal Time Segment 1 | ^
*      Nominal Syncronization Jump Width | ^
*      Data Prescaler         | Set according to data bitrate setting in the DFU Host Tool
*      Data Time Segment 1    | ^
*      Data Time Segment 1    | ^
*      Data Syncronization Jump Width | ^
* 
* \image html dfu_canfd2.png
* \n
* 
* <b> ID Filter settings </b> - configure standard or extended frame ID filter:
*      Parameter name         | Value                                |
*      -----------------------|--------------------------------------
*      Number of SID Filters  | 1 if standard frame is used, 0 otherwise
*      Number of XID Filters  | 1 if extended frame is used, 0 otherwise
*      Standard Filter Element Configuration | Store into Rx Buffer or as Debug Message
*      SFID1/EFID1            | As configured in the DFU Host Tool
*      Store the Received Message | Store Message into an Rx Buffer
*      Rx Bufer Element       | 0
* 
* \image html dfu_canfd3.png
* \n
* 
* <b> Global Filter & Rx Buffers settings </b> - configure global filter and Rx buffer:
*      Parameter name         | Value                                |
*      -----------------------|--------------------------------------
*      Accept Non-matcing Frames Standard | Reject
*      Accept Non-matcing Frames Extended | Reject
*      Reject Remote Frames Standard | Enabled
*      Reject Remote Frames Extended | Enabled
*      Rx Bufer Data Field Size | 64 Byte Data Field
*      Number of Rx Buffers   | 1
* 
* \image html dfu_canfd4.png
* \n
* 
* <b> Tx Buffers & Tx Buffer #0 settings </b> - configure Tx buffer:
*      Parameter name         | Value                                |
*      -----------------------|--------------------------------------
*      Tx Bufer Data Field Size | 64 Byte Data Field
*      Number of Tx Buffers   | 1
*      XTD                    | As configured in the DFU Host Tool
*      Identifier             | ^
*      BRS                    | ^
*      FDF                    | CAN FD Format
* 
* \image html dfu_canfd5.png
* \note DLC and Data will be set by the middleware according to the specific transaction.
*
********************************************************************************
* \subsection group_dfu_ucase_checksum Change checksum types
********************************************************************************
*
* DFU supports two types of checksums:
*   - transport packet checksum
*   - application image checksum.
*
* For a packet, DFU supports 2 types of checksums: Basic summation and
* CRC-16CCITT. The basic summation checksum is computed by adding all the bytes
* (excluding  the checksum) and then taking the 2's complement. CRC-16CCITT -
* the 16-bit CRC using the CCITT algorithm. The packet checksum type is
* selected with a macro \ref CY_DFU_OPT_PACKET_CRC in dfu_user.h file:
* 0 - basic summation (default),
* 1 - for CRC-16.
*
********************************************************************************
* \section group_dfu_changelog Changelog
********************************************************************************
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td rowspan="1">6.0.0</td>
*     <td>Migrate DFU middleware to the HAL Next flow</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td rowspan="4">5.2.0</td>
*     <td>Added USB HID transport based on the emUSB-Device middleware for the CAT1A device</td>
*     <td>Extending the current feature</td>
*   </tr>
*   <tr>
*     <td>Added CANFD transport based on the PDL driver for the CAT1C device</td>
*     <td>Extending the current feature</td>
*   </tr>
*   <tr>
*     <td>Minor updates in the templates</td>
*     <td>Improved the templates usability</td>
*   </tr>
*   <tr>
*     <td>Fixed address validation for CAT1C device</td>
*     <td>Bugfix</td>
*   </tr>
*   <tr>
*     <td rowspan="3">5.1.0</td>
*     <td>Added USB CDC transport based on the emUSB-Device middleware for the CAT1A device</td>
*     <td>Extending the current feature</td>
*   <tr>
*     <td>Minor updates in the templates</td>
*     <td>Improved the templates usability</td>
*   </tr>
*   <tr>
*     <td>Corrected the name of the UART object used in the cyhal_uart_set_baud() function</td>
*     <td>Now, works correctly the custom baud rate configuring in the UART transport</td>
*   </tr>
*   <tr>
*     <td rowspan="5">5.0.0</td>
*     <td>Add support of the MCUBoot flow.</td>
*     <td>New functionality.</td>
*   </tr>
*   <tr>
*     <td> Add support of the transport switching at the run time. </td>
*     <td> New functionality.
*     </td>
*   </tr>
*   <tr>
*     <td> CAT1 device flash read/write operation and I2C/SPI/UART transport
*          templates updated to use mtb-hal-cat1 drivers instead of mtb-pdl-cat1.
*     </td>
*     <td> Enhance code portability. </td>
*   </tr>
*   <tr>
*     <td> Removed Cy_DFU_Complete function as not used. </td>
*     <td> Code cleanup. </td>
*   </tr>
*   <tr>
*     <td> Removed CAT1A BLE transport templates. </td>
*     <td> BLESS stack is not supported in the MTB 3.0. </td>
*   </tr>
*   <tr>
*     <td rowspan="3">4.20</td>
*     <td>Added USB CDC transport configuration for the CAT2 PDL.</td>
*     <td>Add support for the USB interface for the PMG1 device family.</td>
*   </tr>
*   <tr>
*     <td>Updated timeout time for the CAT1A SPI transport. </td>
*     <td>Fixed the DFU Host Tool timeout error for the CAT1A SPI transport
*         caused by the incorrect function call
*         (transport_spi.c file, SPI_SpiCyBtldrCommRead() function).
*     </td>
*   </tr>
*   <tr>
*     <td>Minor documentation update.</td>
*     <td>Documentation improvement.</td>
*   </tr>
*   <tr>
*     <td rowspan="3">4.10</td>
*     <td>Added PSOC 4 devices support.
*     </td>
*     <td>Extended device support.
*     </td>
*   </tr>
*   <tr>
*     <td>Added MISRA-C:2012 compliance.</td>
*     <td>MISRA standard compliance.</td>
*   </tr>
*   <tr>
*     <td>Updated SPI communication timeout granularity.</td>
*     <td>Fixed SPI communication issue.</td>
*   </tr>
*   <tr>
*     <td rowspan="6">4.0</td>
*     <td>Updated the linker scripts to use the single pre-compiled CM0p image.
*         The upgradeable part of the image is the CM4 application.</td>
*     <td>Support ModusToolbox v2.0 build flow.</td>
*   </tr>
*   <tr>
*     <td>Added the ARM compiler version 6 support (version 5 is not supported).</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>Added the USB interface (virtual COM port) transport template.</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>Removed the Secure Application Formats support.</td>
*     <td>Secure Application Formats is not supported in ModusToolbox v2.0 build
*         flow.</td>
*   </tr>
*   <tr>
*     <td>Fixed the return value for the SYNC command processing.</td>
*     <td>The SYCN command returned fail after successful execution.</td>
*   </tr>
*   <tr>
*     <td>Updated the major and minor version defines to follow the naming
*         convention.</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td rowspan="2">3.10</td>
*     <td>Remove the function prototype from the MDK linker script include file.</td>
*     <td>Fix the linker error for the MDK compiler.</td>
*   </tr>
*   <tr>
*     <td>Add BLE transport templates.</td>
*     <td>Add BLE middleware support.</td>
*   </tr>
*   <tr>
*     <td rowspan="2">3.0</td>
*     <td>Bootloader SDK is renamed to the DFU (Device Firmware Update) SDK.
*         All API prefixes and file names are renamed accordingly. \n
*         Added BWC macros to simplify migration.
*     </td>
*     <td> Avoid the confusion with the device boot-up and OS load.</td>
*   </tr>
*   <tr>
*     <td>Flattened the organization of the driver source code into the single
*         source directory and the single include directory.
*     </td>
*     <td>Driver library directory-structure simplification.</td>
*   </tr>
*   <tr>
*     <td rowspan="2">2.20</td>
*     <td> Add check of application number in Set Application Metadata command
*          processing routine.
*     </td>
*     <td>Prevent incorrect usage of the Set Application Metadata command.
*     </td>
*   </tr>
*   <tr>
*     <td>Minor documentation updates</td>
*     <td>Documentation improvement</td>
*   </tr>
*   <tr>
*     <td>2.10</td>
*     <td> Moved address and golden image checks from cy_dfu.c to
*       \ref Cy_DFU_WriteData() in dfu_user.c, so the checks can be
*       customized based on application needs.
*     </td>
*     <td>Allows receiving an update for the running app use case.
*         Improvements made based on usability feedback.
*         Documentation update and clarification.
*     </td>
*   </tr>
*   <tr>
*     <td>2.0</td>
*     <td>
*       <ul>
*         <li>Use the shared RAM for application switching
*             instead of the BACKUP register.</li>
*         <li>Add support of secure application verification.</li>
*         <li>Add support of I2C/SPI/BLE transport protocols.</li>
*         <li>Linker scripts updated for PSOC6 Rev *A devices.</li>
*         <li>Made CRC default application checksum.</li>
*       </ul>
*     </td>
*     <td>To increase functionality.</td>
*   </tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version.</td>
*     <td></td>
*   </tr>
* </table>
*
********************************************************************************
* \section group_dfu_more_info More Information
********************************************************************************
*
* For more information, refer to the links in the
* [README.md](https://github.com/Infineon/dfu/blob/master/README.md#more-information)
*
* \defgroup group_dfu_macro         Macros
* \{
* \defgroup group_dfu_macro_config  User Config Macros
* \}
* \defgroup group_dfu_functions     Functions
* \defgroup group_dfu_globals       Global Variables
* \defgroup group_dfu_data_structs  Data Structures
* \defgroup group_dfu_enums         Enumerated Types
*/

#if !defined(__GNUC__) && !defined(__ARMCC_VERSION) && !defined(__ICCARM__)
    #error "Unsupported compiler, use either GNU, ARM or IAR C compilers"
#endif

#include <stdint.h>
#include "dfu_user.h"
#include "cy_dfu_bwc_macro.h"

#if defined(CY_DFU_OPT_CRYPTO_HW) && (CY_DFU_OPT_CRYPTO_HW != 0)
    #include "cy_crypto.h"
#endif

#include "cy_syslib.h"


#ifdef __cplusplus
extern "C"{
#endif

/**
* \addtogroup group_dfu_macro
* \{
*/

/** The DFU SDK major version */
#define CY_DFU_SDK_MW_VERSION_MAJOR       (6)

/** The DFU SDK minor version */
#define CY_DFU_SDK_MW_VERSION_MINOR       (0)

/**
* \defgroup group_dfu_macro_state DFU State
* \{
* The state of updating. \n
* This is a set of values that the DFU state variable can hold. \n
* When Cy_DFU_Continue() and Cy_DFU_Complete() return, the state parameter
* indicates whether the update has finished successfully or what is the unsuccessful
* state.
*/
#define CY_DFU_STATE_NONE          (0U) /**< Updating has not yet started, no Enter packet received */
#define CY_DFU_STATE_UPDATING      (1U) /**< Updating is in process             */
#define CY_DFU_STATE_FINISHED      (2U) /**< Updating has finished successfully */
#define CY_DFU_STATE_FAILED        (3U) /**< Updating has finished with an error   */
/** \} group_dfu_macro_state */

#define CY_DFU_PACKET_MIN_SIZE     (0x07U) /**< The smallest valid DFU packet size */

/**
* \defgroup group_dfu_macro_commands DFU Commands
* \{
*/
#define CY_DFU_CMD_ENTER           (0x38U) /**< DFU command: Enter DFU           */
#define CY_DFU_CMD_EXIT            (0x3BU) /**< DFU command: Exit DFU            */
#define CY_DFU_CMD_PROGRAM_DATA    (0x49U) /**< DFU command: Program Data               */
#define CY_DFU_CMD_VERIFY_DATA     (0x4AU) /**< DFU command: Verify Data                */
#define CY_DFU_CMD_ERASE_DATA      (0x44U) /**< DFU command: Erase Data                 */
#define CY_DFU_CMD_VERIFY_APP      (0x31U) /**< DFU command: Verify Application         */
#define CY_DFU_CMD_SEND_DATA       (0x37U) /**< DFU command: Send Data                  */
#define CY_DFU_CMD_SEND_DATA_WR    (0x47U) /**< DFU command: Send Data without Response */
#define CY_DFU_CMD_SYNC            (0x35U) /**< DFU command: Sync DFU            */
#define CY_DFU_CMD_SET_APP_META    (0x4CU) /**< DFU command: Set Application Metadata   */
#define CY_DFU_CMD_GET_METADATA    (0x3CU) /**< DFU command: Get Metadata               */
#define CY_DFU_CMD_SET_EIVECTOR    (0x4DU) /**< DFU command: Set EI Vector              */

#define CY_DFU_USER_CMD_START      (0x50U) /**< DFU user commands: min value */
#define CY_DFU_USER_CMD_END        (0xFFU) /**< DFU user commands: max value */



/** \} group_dfu_macro_commands */

/**
* \defgroup group_dfu_macro_ioctl Read/Write Data IO Control Values
* \{
* The values of the ctl parameter to the \ref Cy_DFU_ReadData() and \ref Cy_DFU_WriteData() functions.
* - Bit 0:
*   * 0, Normal read or write operations.
*   * 1, Erase a memory page for write operations.
*        Compare a memory page with the data in the buffer for read operation.
* - Bit 1:
*   * 0, Read or write with raw data
*   * 1, Data received from/to be sent to the DFU Host.
*        May require encryption/decryption or any other special treatment.
         E.g. read/write a data from/to an address with an offset.
* - Bit 2: Reserved.
* - Bit 3: Reserved.
* - Bit 4 - 31: Unused in DFU SDK. Up to the user to specify it.
*/

#define CY_DFU_IOCTL_READ          (0x00U) /**< Read data into the buffer                         */
#define CY_DFU_IOCTL_COMPARE       (0x01U) /**< Compare read data with the data in the buffer */

#define CY_DFU_IOCTL_WRITE         (0x00U) /**< Write the buffer to communication */
#define CY_DFU_IOCTL_ERASE         (0x01U) /**< Erase memory page             */

#define CY_DFU_IOCTL_BHP           (0x02U) /**< Data from/to DFU Host. It may require decryption. */

/** \} group_dfu_macro_ioctl */

/**
* \defgroup group_dfu_macro_response_size Response Size
* \{
*/

#define CY_DFU_RSP_SIZE_0          (0U)    /**< Data size for most DFU commands responses */
#define CY_DFU_RSP_SIZE_VERIFY_APP (1U)    /**< Data size for 'Verify Application' DFU command response */

/** \} group_dfu_macro_response_size */

/** DFU SDK PDL ID */
#define CY_DFU_ID                  CY_PDL_DRV_ID(0x06U)


/** \} group_dfu_macro */


/**
* \addtogroup group_dfu_enums
* \{
*/

/** Used to return the statuses of most DFU SDK APIs */
typedef enum
{
    /** Correct status, No error */
    CY_DFU_SUCCESS         =                                   0x00U,
    /** Verification failed */
    CY_DFU_ERROR_VERIFY    = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x02U,
    /** The length of the received packet is outside of the expected range */
    CY_DFU_ERROR_LENGTH    = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x03U,
    /** The data in the received packet is invalid */
    CY_DFU_ERROR_DATA      = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x04U,
    /** The command is not recognized */
    CY_DFU_ERROR_CMD       = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x05U,
    /** The checksum does not match the expected value */
    CY_DFU_ERROR_CHECKSUM  = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x08U,
    /** The wrong address */
    CY_DFU_ERROR_ADDRESS   = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0AU,
    /** The write to external memory device failed */
    CY_DFU_ERROR_WRITE_EXT = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0BU,
    /** The read from external memory device failed */
    CY_DFU_ERROR_READ_EXT  = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0CU,
    /** The command timed out */
    CY_DFU_ERROR_TIMEOUT   = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x40U,
    /** One or more of input parameters are invalid */
    CY_DFU_ERROR_BAD_PARAM = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x50U,
    /** An unknown DFU error, this shall not happen */
    CY_DFU_ERROR_UNKNOWN   = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0FU
} cy_en_dfu_status_t;

/** Used to select one of the transport interface for the update session */
typedef enum
{
    CY_DFU_I2C     = 0x01U, /**< I2C transport interface */
    CY_DFU_UART    = 0x02U, /**< UART transport interface */
    CY_DFU_SPI     = 0x03U, /**< SPI transport interface */
    CY_DFU_CANFD   = 0x06U, /**< CAN FD transport interface */
} cy_en_dfu_transport_t;


/** \} group_dfu_enums */


/**
* \addtogroup group_dfu_data_structs
* \{
*/
struct cy_stc_dfu_params_s;
/** The type for custom command handlers */
typedef cy_en_dfu_status_t (*Cy_DFU_CustomCommandHandler) (uint32_t command, uint8_t  *packetData, uint32_t dataSize,
                                                            uint32_t *rspSize, struct cy_stc_dfu_params_s *params,
                                                            bool *noResponse);


/**
 * Working parameters for some DFU SDK APIs to be initialized before calling DFU API.
 * */
typedef struct cy_stc_dfu_params_s
{
    /**
    * The pointer to a buffer that keeps data to read or write to an NVM.
    * It is required to be 4-byte aligned.
    */
    uint8_t  *dataBuffer;
    /**
     * An offset within \c dataBuffer to put a next chunk of data
     */
    uint32_t  dataOffset;
    /**
    * The pointer to a buffer that keeps packets sent and received with the Transport API.
    * It is required to be 4-byte aligned.
    */
    uint8_t  *packetBuffer;
    /**
     * The time (in milliseconds) for which the
     * communication interface waits to receive a new data packet
     * from Host in \ref Cy_DFU_Continue(). A typical value is 20 ms.
     */
    uint32_t  timeout;
    /**
     * Set with the Set App Metadata DFU command.
     * Used to determine an appId of a DFU image
     */
    uint32_t  appId;
    /**
     * Internal, flags if Verify Application is called before Exit
     */
    uint32_t  appVerified;

    /**
    * The initial value to the ctl parameter for
    * \ref Cy_DFU_ReadData and \ref Cy_DFU_WriteData.
    * The DFU SDK functions call the Read/Write Data functions like this: \n
    * Cy_DFU_ReadData(addr, length, CY_DFU_IOCTL_COMPARE, params).
    */
    uint32_t  initCtl;

#if (defined(CY_DFU_OPT_SET_EIVECTOR) && (CY_DFU_OPT_SET_EIVECTOR != 0)) || defined(CY_DOXYGEN)
    /**
    * The pointer to the Encryption Initialization Vector buffer.
    * Must be 0-, 8-, or 16-byte long and 4-byte aligned.
    * This may be used in \ref Cy_DFU_ReadData and \ref Cy_DFU_WriteData
    * to encrypt or decrypt data when the CY_DFU_IOCTL_BHP flag is set in the
    * ctl parameter.
    */
    uint8_t *encryptionVector;
#endif /* (CY_DFU_OPT_SET_EIVECTOR != 0) || defined(CY_DOXYGEN) */

#if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN)
    Cy_DFU_CustomCommandHandler handlerCmd; /**< User handler for the custom commands.*/
#endif /* #if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN) */

} cy_stc_dfu_params_t;

/**
* Only used inside DFU Command_Enter().
* \note A public definition because the user may want to redefine
* the DFU packet.
*/
typedef struct
{
    uint32_t enterSiliconId;             /**< The silicon ID for a device */
    uint8_t  enterRevision;              /**< Silicon Revision for a device */
    uint8_t  enterDFUVersion[3];         /**< The DFU SDK version */
} cy_stc_dfu_enter_t;
/** \} group_dfu_data_structs */


/**
* \addtogroup group_dfu_globals
* \{
*/

/** \cond INTERNAL */

/**
* \defgroup group_dfu_globals_external_elf_symbols External ELF file symbols
* \{
* CyMCUElfTools adds these symbols to a generated ELF file. \n
* Their values are either defined in the linker script (GCC, IAR)
* or in the assembly code (ARM):
* (see section \ref group_dfu_config_linker_scripts).
* They may be used by CyMCUElfTool as parameters for generating a .cyacd2 file.
* Also, use the DFU SDK APIs to refer link-time known values to the compile time.
*/

/**
 * Metadata address.
 * DFU uses this symbol to access metadata.
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_boot_metadata_addr;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * Metadata row size.
 * The DFU uses this symbol to access metadata.
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_boot_metadata_length;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * Product ID.
 * CyMCUElfTool uses this value to place in the .cyacd2 header.
 * The DFU uses this value to verify if an image is compatible with the device.
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_product_id;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * Checksum Algorithm of the DFU Host Command/Response Protocol packet.
 * Possible values
 * - 0 For the Basic Summation algorithm
 * - 1 For the CRC-16 algorithm
 * \note Must be aligned with \ref CY_DFU_OPT_PACKET_CRC
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_checksum_type;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * Current application number
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_app_id;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * CPU1 vector table address, if present
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_app_core1_start_addr;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/** \} group_dfu_globals_external_elf_symbols */

/** \endcond*/

/** \} group_dfu_globals */

/**
* \addtogroup group_dfu_functions
* \{
*/

cy_en_dfu_status_t Cy_DFU_Init(uint32_t *state, cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_Continue(uint32_t *state, cy_stc_dfu_params_t *params);

uint32_t Cy_DFU_DataChecksum(const uint8_t *address, uint32_t length, cy_stc_dfu_params_t *params);

/** \cond INTERNAL */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
/**
* \defgroup group_dfu_functions_meta Metadata Management
* \{
*   DFU functions for operation over meta data.
*/

cy_en_dfu_status_t Cy_DFU_GetAppMetadata(uint32_t appId, uint32_t *verifyAddress, uint32_t *verifySize);
cy_en_dfu_status_t Cy_DFU_ValidateMetadata(uint32_t metadataAddress, cy_stc_dfu_params_t *params);
#if (CY_DFU_METADATA_WRITABLE != 0) || defined(CY_DOXYGEN)
    cy_en_dfu_status_t Cy_DFU_SetAppMetadata(uint32_t appId, uint32_t verifyAddress,
                                                       uint32_t verifySize, cy_stc_dfu_params_t *params);
#endif /* (CY_DFU_METADATA_WRITABLE != 0) || defined(CY_DOXYGEN) */
/** \} group_dfu_functions_meta */


/**
* \defgroup group_dfu_functions_app Application Management
* \{
*   DFU functions for the application management
*/
void Cy_DFU_ExecuteApp(uint32_t appId);
void Cy_DFU_OnResetApp0(void);
uint32_t Cy_DFU_GetRunningApp(void);
cy_en_dfu_status_t Cy_DFU_SwitchToApp(uint32_t appId);
cy_en_dfu_status_t Cy_DFU_CopyApp(uint32_t destAddress, uint32_t srcAddress, uint32_t length,
                                            uint32_t rowSize, cy_stc_dfu_params_t *params);
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */
/** \endcond*/

/**
* \defgroup group_dfu_functions_app Application Management
* \{
*   DFU functions for the application management
*/
cy_en_dfu_status_t Cy_DFU_ValidateApp(uint32_t appId, cy_stc_dfu_params_t *params);
/** \} group_dfu_functions_app */

/**
* \defgroup group_dfu_functions_mem Memory Operations
* \{
*   DFU functions for memory operations
*   These IO functions have to be re-implemented in the user's code.
*/
cy_en_dfu_status_t Cy_DFU_ReadData (uint32_t address, uint32_t length, uint32_t ctl,
                                              cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_WriteData(uint32_t address, uint32_t length, uint32_t ctl,
                                              cy_stc_dfu_params_t *params);
/** \} group_dfu_functions_mem */


/**
* \defgroup group_dfu_functions_transport Transport Management
* \{
*   DFU functions for the communication interface.
*   These communication functions have to be re-implemented in the user's code.
*/
cy_en_dfu_status_t Cy_DFU_TransportRead (uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout);
cy_en_dfu_status_t Cy_DFU_TransportWrite(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout);
void Cy_DFU_TransportReset(void);
void Cy_DFU_TransportStart(cy_en_dfu_transport_t transport);
void Cy_DFU_TransportStop(void);
/** \} group_dfu_functions_transport */
/**
* \defgroup group_dfu_functions_custom_cmd Custom commands
* \{
*
*   The DFU protocol provides a set of pre-defined commands. The user can also
*   add custom commands and register the single handler for all custom commands
*   at the application level. This allows to adjust use case scenarios
*   per the product needs. The feature is enabled with \ref CY_DFU_OPT_CUSTOM_CMD
*   set to non-zero value in the dfu_user.h or project Makefile.
*
*   \note Custom commands only extend the functionality of the DFU command protocol
*   and must be issued after entering the updating state (\ref CY_DFU_STATE_UPDATING).
*
*   The user commands area preserved in the DFU command protocol:
*   - \ref CY_DFU_USER_CMD_START
*   - \ref CY_DFU_USER_CMD_END
*
*  An example of the custom commands usage:
*
*   1. Add a set of the custom commands to the project.
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMANDS
*
*   2. Define the function to handle the custom commands.
*   \note A single function is used as the handler for all custom commands.
*
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMAND_DECLARATION
*
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMAND_HANDLER
*
*   3. Register the function to handle custom commands as a callback in the DFU core before use.
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMAND_REGISTER
*
*   4. Release the callback function when custom command handling is no longer required.
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMAND_UNREGISTER
*/

#if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN)
cy_en_dfu_status_t Cy_DFU_RegisterUserCommand(cy_stc_dfu_params_t *params, Cy_DFU_CustomCommandHandler handler);
cy_en_dfu_status_t Cy_DFU_UnRegisterUserCommand(cy_stc_dfu_params_t *params);
#endif /* #if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN) */
/** \} group_dfu_functions_custom_cmd */

/** \} group_dfu_functions */


/***************************************
*  Internal declarations
****************************************/
/** \cond INTERNAL */

#define CY_DFU_SILICON_ID      (0U)
#define CY_DFU_SILICON_REV     (0U)

/* Cypress Basic Application Format (CyBAF) */
#define CY_DFU_BASIC_APP           (0U)
/* Cypress Secure Application Format (CySAF) - NOT SUPPORTED */
#define CY_DFU_CYPRESS_APP         (1U)
/* Simplified Secure Application Format (SSAF) - NOT SUPPORTED */
#define CY_DFU_SIMPLIFIED_APP      (2U)

/* Set the application format. Only CyBAF is supported. */
#define CY_DFU_APP_FORMAT          (CY_DFU_BASIC_APP)

#define CY_DFU_VERIFY_FAST         (0U)    /* Verification includes only
                                            * application check */
#define CY_DFU_VERIFY_FULL         (1U)    /* Verification includes application,
                                            * key, and TOC checks */

/* Set the verification type for CySAF and SSAF.
 * NOT SUPPORTED - only CyBAF is supported */
#define CY_DFU_SEC_APP_VERIFY_TYPE  (CY_DFU_VERIFY_FAST)

/*
* These defines are obsolete and kept for backward compatibility only.
* They will be removed in the future versions.
*/
#define CY_DFU_SDK_VERSION_MAJOR  (CY_DFU_SDK_MW_VERSION_MAJOR)
#define CY_DFU_SDK_VERSION_MINOR  (CY_DFU_SDK_MW_VERSION_MINOR)
/** \endcond*/

#ifdef __cplusplus
}
#endif

#endif /* !defined(CY_DFU_H) */


/* [] END OF FILE */
