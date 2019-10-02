/***************************************************************************//**
* \file cy_dfu.h
* \version 4.0
*
* Provides API declarations for the DFU SDK.
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_DFU_H)
#define CY_DFU_H
    
/**
* \mainpage Cypress Device Firmware Update (DFU) Middleware Library 4.0
*
* \section section_mainpage_overview Overview
*
* The purpose of the DFU middleware library is to provide an SDK for updating 
* firmware images. The middleware allows creating two types of projects:
*
* 1. An application loader receives the program and switch to
*    the new application.
* 2. A loadable application to be transferred and programmed.
*
* A project can contain features of both first and second type.
*
********************************************************************************
* \section section_dfu_general General Description
********************************************************************************
*
* Include cy_dfu.h to get access to all functions and other declarations in this 
* library.
*
* The DFU SDK has the following features:
* - Read firmware images from a host through a number of transport interfaces,
*   e.g. BLE, USB, UART, I2C, SPI.
* - Program a firmware image to the specified address in internal flash,
*   XIP region, or any external memory that supports the DFU API.
* - Copy applications.
* - Validate applications.
* - Safe Update: updates at a temporary location, validates, and if valid,
*   overwrites a working image.
* - Switches applications. Passes parameters in RAM when switching
*   applications.
* - Supports encrypted image files.
*   Transfers encrypted images without decrypting in the middle.
* - Supports many application images, the number of which is limited by 
*   the metadata size. Each image can be an application loader. For example, 
*   512-byte metadata supports up to 63 applications.
* - Supports customization.
* - Supports the CRC-32 checksum to validate data.
*
********************************************************************************
* \section section_dfu_quick_start Quick Start Guide
********************************************************************************
*
* The DFU SDK is used to design updating applications of
* arbitrary flexibility and complexity. Cypress DFU middleware can be used in 
* various software environments. Refer to the \ref section_dfu_toolchain.
* To quick start, use the Code Examples.
* Cypress Semiconductor continuously extends its portfolio of code examples
* at <a href="http://www.cypress.com"><b>Cypress Semiconductor website</b></a>
* and <a href="https://github.com/cypresssemiconductorco">
* <b>Cypress Semiconductor GitHub</b></a>.
*
* The Quick Start Guide (QSG) assumes ModusToolbox 2.0 is installed
* with all needed tools.
*   
* The following steps are to set up and build a basic DFU loader and loadable 
* applications. The DFU loader application uses I2C transport interface.
* The steps assume that the user builds an applications for CY8CKIT-062-WIFI-BT 
* based on a starter Hello_World ModusToolbox project.
*
* \subsection ssection_dfu_step_0 STEP 0: Projects preparation.
*
* 1. Create a project for CY8CKIT-062-WIFI-BT with the DFU loader application
*    using the Hello_World starter application. 
*    Name it QSG_DFU_App0_I2C. See the ModusToolbox 2.0 IDE Quick Start 
*    Guide for the detail steps.
* 2. Create a project for the DFU loadable application in the same way and name 
*    it QSG_DFU_App1_Hello_World
* 3. Include the DFU middleware into each project using the ModusToolbox Library 
*    Manager or download it from GitHub and copy it to the project manually.
* 4. Include a DFU header in main.c of each project to get access to DFU API: 
*    \snippet dfu/snippet/main.c snipped_cy_dfu_include
*
* \subsection ssection_dfu_step_1 STEP 1: Setup Loader Application QSG_DFU_App0_I2C
*
* 1. Copy the configuration files **dfu_user.c** and **dfu_user.h** from the
*    libs\\dfu\\config directory and put them near main.c.
* 2. Copy the transport files from the \\config directory and put them
*    near main.c. In our case, I2C requires **transport_i2c.h** and 
*    **transport_i2c.c**.
* 3. Copy the app0 linker script files from
*    libs\\dfu\\linker_scripts\\TOOLCHAIN_<COMPILER>\\
*    in the project root. For the GCC ARM compiler, 
*    copy libs\\dfu\\linker_scripts\\TOOLCHAIN_GCC_ARM\\dfu_cm4_app0.ld
* 4. Configure SCB for I2C communication using ModusToolbox Device Configurator. 
*    For CY8CKIT-062-WIFI-BT use SCB 3, which is connected to the KitProg.
*    SCB parameter name     | value
*    -----------------------|--------------
*    Personality            | I2C
*    Name                   | **DFU_I2C**
*    Mode                   | Slave
*    Data Rate (kbps)       | 100
*    Slave Address (7-bit)  | 12
*    \warning SCB personality must be **I2C** and name must be **DFU_I2C**. 
*
*    \image html dfu_basic_i2c.png
*    See \ref group_dfu_mtb_cfg
*
* \subsection ssection_dfu_step_2 STEP 2: Update Loader QSG_DFU_App0_I2C main.c
*
* 1. Include a DFU reset handler to start the appropriate application after
*    a reset:
*    \snippet dfu/snippet/main.c snipped_cy_dfu_onreset 
* 2. Initialize the variables and call the DFU initialization function:
*    \snippet dfu/snippet/main.c snipped_cy_dfu_init
* 3. Initialize the DFU transport layer:
*    \snippet dfu/snippet/main.c snipped_cy_dfu_init_comm
* 4. Update the main loop with the Host Command/Response protocol processing:
*    \snippet dfu/snippet/main.c snipped_cy_dfu_command_process
* 5. Update the main loop with a routine to switch to the loaded
*    QSG_DFU_App1_Hello_World application.
*
*    For example, to switch by pressing the CY8CKIT-062-WIFI-BT button SW2: 
*    - Enable pin 0[4] in Device Configurator with name **PIN_SW2**
*    - Set/check pin configuration:
*      Parameter name      | value
*      --------------------|--------------------------------------
*      Driver Mode         | Resistive Pull-Up. Input buffer on
*      Initial Drive State | High(1)
*      \image html dfu_qsg_btn.png
*    - Add the following routine:
*    \snippet dfu/snippet/main.c snipped_cy_dfu_switch_app1 
*
* \subsection ssection_dfu_step_3 STEP 3: Build and Program Loader QSG_DFU_App0_I2C
*
* 1. Update the project Makefile to use the DFU linker script for the
*    appropriate toolchain dfu_cm4_app0.*. Set LINKER_SCRIPT to the linker
*    script copied to the project root.
*    \code LINKER_SCRIPT=dfu_cm4_app0.ld \endcode  
* 2. Add a post-build step to sign the ELF file or sign it manually after
*    the build: \<MCUELFTOOL\> --sign \<app\>.elf --output \<app_signed\>.elf
*    --hex \<app_signed\>.hex.\n  
*    For macOS/Linux platform:
*    \code POSTBUILD=$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool --sign $(CY_CONFIG_DIR)/$(APPNAME).elf --hex $(CY_CONFIG_DIR)/$(APPNAME).hex\endcode
*    For Windows platform:
*    \code POSTBUILD="$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool.exe" --sign $(CY_CONFIG_DIR)/$(APPNAME).elf --hex $(CY_CONFIG_DIR)/$(APPNAME).hex\endcode
* 3. Connect DVK CY8CKIT-062-WIFI-BT. Build and program the device. 
* 4. Observe the device red LED blinking. 
*
*  \anchor loadable_DFU_app_steps 
* \subsection ssection_dfu_step_4 STEP 4: Setup Loadable QSG_DFU_App1_Hello_World
*
* 1. Copy the configuration file **dfu_user.h** from the libs\\dfu\\config
*    directory and put them near main.c in the project root.
*    \warning Do not copy **dfu_user.c** to avoid duplication of the metadata
*    structures 
* 2. Copy the app1 linker script files from
*    \\libs\\dfu\\linker_scripts\\TOOLCHAIN_<COMPILER>\\
*    in the project root. For GCC ARM compiler copy
*    \\libs\\dfu\\linker_scripts\\TOOLCHAIN_GCC_ARM\\dfu_cm4_app1.ld
*
* \subsection ssection_dfu_step_5 STEP 5: Update Loadable QSG_DFU_App1_Hello_World main.c
*
* 1. Update the main.c file with the .cy_app_signature section
*    \snippet dfu/snippet/main.c snipped_cy_dfu_app_signature
* 2. Initialize the peripheral in the main function: 
*    \snippet dfu/snippet/main.c snipped_cy_dfu_app1_cfginit
* 3. Change the behavior of the basic application. For example, change the
*    Blinky LED  to green or change the UART output.
* 4. Update the main loop with a routine to switch to the loader 
*    QSG_DFU_App0_I2C application to load another application.
*
*    For example, to switch by pressing the CY8CKIT-062-WIFI-BT button SW2: 
*    - Enable pin 0[4] in Device Configurator with name **PIN_SW2**
*    - Set/check pin configuration:
*      Parameter name      | value
*      --------------------|--------------------------------------
*      Driver Mode         | Resistive Pull-Up. Input buffer on
*      Initial Drive State | High(1)
*      \image html dfu_qsg_btn.png
*    - Add the following routine:
*      \snippet dfu/snippet/main.c snipped_cy_dfu_switch_app0 
*
* \subsection ssection_dfu_step_6 STEP 6: Build and Program Patch
*
* 1. Update the project Makefile to use the DFU linker script for the
*    appropriate toolchain dfu_cm4_app1.*. Set the LINKER_SCRIPT variable with
*    the path to the copied linker script.
*    \code LINKER_SCRIPT=dfu_cm4_app1.ld \endcode
* 2. Add the post build step to run CyMCUElfTool to generate a patch file in
*    the *.cyacd2 format (see CyMCUElfTool User Guide):
*     + Update the application ELF with a CRC checksum:
*         \<MCUELFTOOL\> --sign app.elf CRC --output app_crc.elf
*     + Generate a patch file:
*         \<MCUELFTOOL\> -P app_crc.elf --output app.cyacd2
*
*    Generate a *.cyacd2 file in the project root.\n  
*    For macOS/Linux platform:
*    \code POSTBUILD=$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool --sign $(CY_CONFIG_DIR)/$(APPNAME).elf CRC --output $(APPNAME)_crc.elf && \
*         $(CY_MCUELFTOOL_DIR)/bin/cymcuelftool -P $(APPNAME)_crc.elf --output $(APPNAME)_crc.cyacd2\endcode
*    For Windows platform:
*    \code POSTBUILD="$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool.exe" --sign $(CY_CONFIG_DIR)/$(APPNAME).elf CRC --output $(APPNAME)_crc.elf && \
*         "$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool.exe" -P $(APPNAME)_crc.elf --output $(APPNAME)_crc.cyacd2\endcode
* 3. Build a project.
* 4. Open the DFU Host Tool. Connect to the device. Select the generated .cyacd2
*     in the project root  and program it to the device.
*    \image html dfu_qsg_hti2c.png
* 5. QSG_DFU_App1_Hello_World application should start after successful program.
*    Observe the kit change behavior based on changes 
*    \ref ssection_dfu_step_5 (green LED blinking, UART output).
*    Also, switching to the QSG_DFU_App1_Hello_World application could be done
*    manually by pressing PIN_SW2 (pin 0[4]).
* 6. Switch back to the loader QSG_DFU_App0_I2C application. 
*    Press the kit button PIN_SW2 (pin 0[4]) and observe that the kit red LED
*    blinking.
*
********************************************************************************
* \section section_dfu_configuration Configuration Considerations
********************************************************************************
*
********************************************************************************
* \subsection group_dfu_config_linker_scripts Linker scripts
********************************************************************************
*
* The DFU SDK projects linker scripts are a bit different from the default
* startup linker scripts.
*
* The DFU linker scripts are:
*
* 1. dfu_cm4_app0.{ld, icf, scat}, dfu_cm4_app1.{ld, icf, scat} for ARM GCC,
*    IAR, and ARM compilers.
* 2. dfu_common.h and dfu_elf_symbols.c for the ARM compiler. 
*
*    These files define symbols for the memory layout for each application 
*    inside the device.
* 
* \par Memory layout of linker scripts (dfu_cm4_{app0, app1}.ld)
*
* This part of the GCC linker script files must have the same memory layout
* across all the application projects in the designed device.
* Any changes made to any application must be copied to other
* applications <i>dfu_cm4_{app0, app1}.ld</i> files.
*
* Memory regions:
* * <i>flash_cm0p</i> - code and data
*   of the default application CM0+ CPU.
* * <i>flash_app{X}</i> - code and data
*   of the user application {X}.
* * <i>flash_boot_meta</i> - for the DFU SDK
*   metadata. Cypress DFU SDK code examples place DFU SDK metadata
*   inside this region.
* * <i>sflash_user_data</i>, <i>eFuse</i>, <i>flash_toc</i>, <i>em_eeprom</i>,
*   <i>xip</i> - These regions not used by typical DFU SDK code examples.
*   They are kept because they may be used in user code.
* * <i>ram_common</i> - shared between DFU SDK applications.
*   The user may place it anywhere inside the RAM,
*   So, one app sets some values there, switches to another app.
*   Then app may read or update the values.
* * <i>ram_app{X}</i> - data, stack, heap etc. for the user app{X}.
* 
* ELF file symbols:
* CyMCUElfTool uses special ELF file symbols besides command-line arguments for
* its configuration. These symbols are defined in each linker script.
* 1. __cy_memory_{N}_start - Defines the start address of the memory region.
*    __cy_memory_{N}_length - Defines the length of the memory region.
*    __cy_memory_{N}_row_size - Defines the row size of the memory region.
*
*    CyMCUElfTool uses these symbols to determine which memory regions should be
*    placed into the output files. I.e. without these symbols, some data
*    like XIP may be absent in the output file.
*    These symbols are critical for .cyacd2 file generation, CyMCUElfTool
*    must know the row size of all the data being exported to the .cyacd2
*    file. The updating is done by rows, and a row size may vary across
*    the memory regions.
*
*    E.g. Internal flash of PSoC6 devices start at address 0x1000_0000 and
*    the length and row size may de device-dependent, but let's assume it is
*    512KB and 512 bytes. The memory symbols for the internal flash will be:
*    <code>
*        __cy_memory_0_start    = 0x10000000;
*        __cy_memory_0_length   = 512 * 1024;
*        __cy_memory_0_row_size = 512;
*    </code>
*
*    The number _{N}_ in the memory symbol indicates that there may be multiple
*    memories.
* 2. __cy_boot_metadata_addr and __cy_boot_metadata_length.
*    These symbols are used by the DFU SDK internally to access the
*    metadata.
* 3. __cy_product_id - used by CyMCUElfTool to be placed in the .cyacd2 header.
*    This value is used by the updating Host and DFU SDK firmware to
*    confirm that the .cyacd2 file being updated is compatible with
*    the device.
*
*    E.g. The user may have two different devices with the same PSoC6 chip:
*    * A coffee machine, with Product ID - 0x1000_0001.
*    * A nuclear power plant control device with Product ID - 0x1000_0002.
*    The user of a coffee machine tries to update firmware for a nuclear
*    power plant control device, and the DFU Host will indicate that
*    the device rejected this firmware because of the wrong Product ID.
* 4. __cy_app{N}_verify_start, __cy_app{N}_verify_length.
*    These symbols are used by the dfu_user.c file to initialize the
*    metadata. Their value is automatically updated by the linker when the
*    user updates the memory layout (memory regions).
*
*    If the user decided to use a different mechanism for SDK metadata
*    initialization, then these symbols can be removed.
* 5. __cy_boot_signature_size. \anchor __cy_boot_signature_size
*    Used by the DFU SDK linker scripts only. It helps avoiding the magic
*    number for a signature size to be scattered throughout all the linker 
*    scripts.
*    E.g.
*    * For the CRC-32C application signature, the value of this symbol is 4 
*      (bytes).
*    * For RSASSA-PCKS-1-v1.5 with RSA 2048, the value is 256 (bytes).
* 6. __cy_checksum_type.
*    The checksum type for the DFU transport packet verification used by 
*    CyMCUElfTool to generate a updating file. Must be aligned with 
*    \ref CY_DFU_OPT_PACKET_CRC
*
* \par File dfu_cm4_app0.ld
*
* This file is a linker script for the app0 for DFU SDK applications. 
*
* It is similar to the default startup GCC's CM4_dual linker script but with
* some differences:
* 1. Memory regions are separated between the CM0+ CPU and CM4 CPU
*    application 0 and CM4 CPU application 1 described above.
* 2. DFU-specific ELF file symbols described above.
* 3. __cy_app_id.
*    These ELF file symbols are used by CyMCUElfTool to set an application ID in 
*    the .cyacd2 file header.
* 4. __cy_app_verify_start, __cy_app_verify_length.
*    These two symbols are used by CyMCUElfTool to generate an application
*    signature. The first symbol provides a value of the start of signed memory 
*    and the second - the length of signed memory.
* 5. Section ".cy_boot_noinit".
*    Used to place data to share between the applications. 
*    See the description of the ram_common memory region.
* 6. Section ".cy_boot_metadata".
*    Contains the DFU SDK metadata. This section name is necessary only
*    for CyMCUElfTool to sign the section with the CRC-32C checksum 
*    of this section data.
*    If no CRC-32C at the end of the metadata is required, then the section can
*    be renamed.
* 7. Section .cy_app_signature.
*    This section is used to place an application signature.
*    The signature is used by the DFU SDK to verify that the application is
*    valid. Typically, CRC, SHA or any other hash of the application code and
*    data is placed here.
*    CyMCUElfTool updates this section in the post-build step.
*    The memory for which the signature is calculated is defined by the
*    following ELF file symbols:
*    __cy_app_verify_start, __cy_app_verify_length.
*
* \par File dfu_cm4_app1.ld
*
* Used to create linker scripts for application \#2, .. \#N  
* It is similar to dfu_cm4_app0.ld linker script with some differences:
* - Region alias for flash and ram are flash_app1 and ram_app1
* - Application ID __cy_app_id = 1
* - Removed section for CM0+ CPU as it is allocated only once in scope
*   of linker script dfu_cm4_app1.ld 
*
* \par Files dfu_cm4_{app0, app1}.{icf, scat}
*
* These files are the linker scripts for the IAR and ARM
* compilers for the DFU SDK applications.
*
* Their difference from the default startup linker scripts are similar to
* the DFU SDK GCC's linker scripts described above.
*
********************************************************************************
* \subsection group_dfu_mtb_cfg Use of ModusToolbox's tools for HW initialization
********************************************************************************
*
* For a setup of the SCB I2C personality in ModusToolbox Device Configurator 
* for the I2C DFU transport for CY8CKIT-062-WIFI-BT, see the screenshot below. 
* For other kits, verify the I2C pins and SCB block selection.
* \note The personality alias name must be DFU_I2C
*
* \image html dfu_basic_i2c.png
*
* For a setup of the SCB SPI personality in ModusToolbox Device Configurator 
* for the SPI DFU transport for CY8CKIT-062-WIFI-BT, see the screenshot below. 
* For other kits, verify the SPI pins and SCB block selection.
* \note The personality alias name must be DFU_SPI
*
* \image html dfu_basic_spi.png
*
* For setup SCB UART personality in ModusToolbox Device Configurator for UART 
* DFU transport for CY8CKIT-062-WIFI-BT see screenshot below. For other kit
* please verify UART pins and SCB block selection.
* \note Personality alias name must be DFU_UART
*
* \image html dfu_basic_uart.png
*
* For a setup of the USB device personality in ModusToolbox Device Configurator
* for the USB CDC DFU transport for CY8CKIT-062-WIFI-BT, see the screenshot 
* below. For other kits, verify the USB pins.
* \note The personality alias name must be DFU_USB_CDC
*
* \image html dfu_usb_cdc.png
*
* For a setup of the BLE device personality in ModusToolbox Device Configurator 
* for the BLE DFU transport, see the screenshot below. 
*
* \image html dfu_ble.png
*
********************************************************************************
* \section section_dfu_design Design Considerations
********************************************************************************
* 
********************************************************************************
* \subsection group_dfu_ucase_uart Firmware Update via UART
********************************************************************************
* 
* See \ref section_dfu_quick_start for steps how to set up a DFU project that 
* upgrades an application via a UART transport interface.
*
********************************************************************************
* \subsection group_dfu_ucase_i2c Firmware Update via I2C
********************************************************************************
* 
* See \ref section_dfu_quick_start for basic steps how to setup a DFU project.
* Specific steps for the I2C transport support:
* - Include transport_i2c.c and transport_i2c.h in the project build flow. For
*   example, copy from the \\config directory to the directory with the main.c 
*   file. Ensure that other transport files are not included in the build flow.
* - Select and configure the SCB block using ModusToolbox Device 
*   Configurator 
*   see \ref group_dfu_mtb_cfg or manually using the configuration
*   structures.
* - Add the post-build step to sign the ELF file or sign it manually after
*   the build 
* \code 
*       <MCUELFTOOL> --sign app.elf --output app_signed.elf\endcode
* - Build and program a project into the device.
* - Open DFU Host Tool. Select the I2C interface. Set ab I2C address and speed
*   according to the SCB I2C setup in the previous step.
* - Select the *.cyacd2 application image and upload to the device     
*
********************************************************************************
* \subsection group_dfu_ucase_spi Firmware Update via SPI
********************************************************************************
* 
* See \ref section_dfu_quick_start for basic steps how to set up a DFU project.
* Specific steps for the SPI transport support:
* - Include transport_spi.c and transport_spi.h in the project build flow. For
*   example, copy them from the \\config directory to the directory with main.c 
*   file. Ensure that other transport files are not included in the build flow.
* - Select and configure the SCB block. This could be done using ModusToolbox 
*   Device Configurator 
*   see \ref group_dfu_mtb_cfg or manually using the configuration structures.
* - Add the post-build step to sign the ELF file or sign it manually after
*   the build 
* \code 
*       <MCUELFTOOL> --sign app.elf --output app_signed.elf\endcode
* - Build and program a project into the device.
* - Open DFU Host Tool. Select the SPI interface. Set SPI mode, shift the
*   direction and speed according to the SCB SPI setup in the previous step.
* - Select *.cyacd2 application image and upload to the device     
*
********************************************************************************
* \subsection group_dfu_ucase_usbcdc Firmware Update via USB CDC
********************************************************************************
* 
* See \ref section_dfu_quick_start for basic steps how to setup a DFU project.
* Specific steps for the USB CDC transport support:
* - Include transport_usb_cdc.c and transport_usb_cdc.h in the project build
*   flow. For example, copy them from the \\config directory to the directory
*   with the main.c file.  Ensure that other transport files are not included
*   in the build flow.
* - Enable and configure the USB Device block using ModusToolbox 
*   Device Configurator
*   see \ref group_dfu_mtb_cfg or manually using the configuration structures.
* - Generate USB descriptors and USB Middleware structures using USB 
*   Configurator. Open the USB configuration file (cycfg_usb_cdc.cyusbdev)
*   in the DFU \\config folder, then click Save to generate configuration files 
*   (cycfg_usbdev.c and cycfg_usbdev.h). These files must be included in the 
*   build flow (see USB Middleware API Reference \ref group_dfu_more_info).  
* - Add the post-build step to sign the ELF file or sign it manually after
*   the build 
* \code 
*       <MCUELFTOOL> --sign app.elf --output app_signed.elf\endcode
* - Build and program a project into the device. Connect your Host to the USB
*   device
* - Open DFU Host Tool. Select the UART interface, because Host recognizes a USB
*   device as a virtual UART (the name should be DFU USB CDC transport). UART
*   settings: baud rate - 115200, data bits - 8, stop bits - 1, parity - None.   
* - Select the *.cyacd2 application image and upload to the device.  
*
********************************************************************************
* \subsection group_dfu_ucase_ble Firmware Update via BLE (Over-the-Air)
********************************************************************************
* 
* See \ref section_dfu_quick_start for basic steps how to set up a DFU project.
* Also, see code example [CE216767](http://www.cypress.com/ce216767)
* Specific steps for the USB BLE transport support:
* - Include transport_ble.c and transport_ble.h in the project build
*   flow. For example, copy them from the \\config directory to the directory
*   with the main.c file.  Ensure that other transport files are not included in 
*   the build flow.
* - Enable and configure the BLE Device block using ModusToolbox
*   Device Configurator 
*   see  \ref group_dfu_mtb_cfg or manually using the configuration structures.
* - Generate BLE Middleware configuration structures. Open the BLE configuration
*   file (cycfg_ble.cybt) in Bluetooth Configurator. The file is located in 
*   the DFU \\config folder. Then click Save to generate configuration files 
*   (cycfg_ble.c and cycfg_ble.h). These files must be included in the build
*   flow (BLE Middleware API Reference \ref group_dfu_more_info).  
* - Add the post build step to sign the ELF file or sign it manually after the 
*   build \code <MCUELFTOOL> --sign app.elf --output app_signed.elf \endcode
* - Build and program the project into the device.
* - Open CySmart. There are two versions: for Windows PC platforms and mobile 
*   application  see \ref group_dfu_more_info). Scan for devices and select your
*   BLE device in the list (should be OTA DFU). 
* - Click Update Firmware -> Application only update. Select the *.cyacd2 
*   application image and upload to the device     
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
* (excluding  the checksum) and then taking the 2’s complement. CRC-16CCITT ‒
* the 16-bit CRC using the CCITT algorithm. The packet checksum type is
* selected with a macro \ref CY_DFU_OPT_PACKET_CRC in dfu_user.h file:
* 0 - basic summation (default), 
* 1 - for CRC-16.
*
* For an application image, DFU supports 2 types of checksums: CRC-32 and SHA1. 
* SHA1 is calculated with a crypto hardware block.
* The default application checksum is CRC-32. 
* Steps to set a SHA1 checksum for an application image:
* - Set \ref CY_DFU_OPT_CRYPTO_HW macro to 1 in dfu_user.h file to enable
*   the SHA1 calculation.
* - Symbol \ref __cy_checksum_type = 0x01 in 
*   \ref group_dfu_config_linker_scripts for each application for
*   ARM GCC and IAR compiler. Set macro CY_CHECKSUM_TYPE to 1 in dfu_common.h
*   for the ARM compiler.
* - Symbol \ref __cy_boot_signature_size = 20 in 
*   \ref group_dfu_config_linker_scripts for each application
*   for the ARM GCC and IAR compilers. Set macro CY_BOOT_SIGNATURE_SIZE to 20
*   in dfu_common.h for the ARM compiler.
* - Configure and start crypto a server and crypto client (see PDL API
*   Reference in \ref group_dfu_more_info) in the loader application main
*   routine.
* - Allocate the ".cy_app_signature" section with a 20-byte array in the main
*   of the loading application.
*
********************************************************************************
* \subsection group_dfu_ucase_multiapp Multi-application DFU project
********************************************************************************
*
* DFU design does not limit the number of applications. The number
* of applications is limited by memory size and metadata size. The maximum size 
* of DFU metadata is limited to the size of the flash row, because metadata
* should be in a single flash row. For example, 512-byte metadata supports
* up to 63 applications.
* An arbitrary number of applications can be protected from overwriting. Such
* a protected application is called "Golden Image".
* \ref section_dfu_quick_start For a setup of basic 2 application DFU
* projects. The following steps show how to set up a 3rd application. 
* The same approach can be used to setup 4th - Nth applications.
* - Define the sizes for each of the 3 applications and define the start and
*   size of each memory region (flash, RAM) for each application.
* - Copy the linker script dfu_cm4_app1 from DFU linker_scripts folder 
*   according to the selected compiler and rename it (for example dfu_cm4_app2).
* - Add flash and RAM sections to the 3rd application. Name them flash_app2,
*   ram_app2.
* - Update the size and start address for each section in each linker script
*   based on the defined in the first step allocation.
* - Set __cy_app_id symbol to 2
* - Update the region aliases for flash and ram to use flash_app2 and ram_app2
*   accordingly: 
*   \code 
*   REGION_ALIAS("flash", flash_app2);
*   REGION_ALIAS("ram",     ram_app2);
*   \endcode
* - Add symbols __cy_app2_verify_start and __cy_app2_verify_length for metadata
*   initialization in the same way as for application 0 and 1.
* - Add macro in the dfu_user.h CY_DFU_APP2_VERIFY_START and
*   CY_DFU_APP2_VERIFY_LENGTH in the same way as for application 0 and 1
* - Add to the cy_dfu_metadata array of the dfu_user.c CY_DFU_APP2_VERIFY_START
*   and CY_DFU_APP2_VERIFY_LENGTH to update metadata with 3rd application.
* - Update you build scripts to use the dfu_cm4_app2 linker script.
*
* Protect the application image by setting parameters in the 
* dfu_user.h file of the loader project: \ref CY_DFU_OPT_GOLDEN_IMAGE
* set to 1 to enable the Golden Image functionality.
* \ref CY_DFU_GOLDEN_IMAGE_IDS lists the number of images that to be protected.
*
********************************************************************************
* \subsection group_dfu_ucase_cyacd2 Creation of the CYACD2 file
********************************************************************************
*
* The .cyacd2 file contains downloadable application data created by 
* CyMCUElfTool and used by host programs such as Cypress’ DFU Host Program and 
* CySmart to send applications to the target DFU module
* (see \ref group_dfu_more_info). Refer to the
* [AN213924](http://www.cypress.com/an213924) DFU SDK User Guide for the .cyacd2
* file format. See the \ref loadable_DFU_app_steps "Loadable Application Setup"
* section of the \ref section_dfu_quick_start for the steps to convert
* a general application into a DFU loadable application.
*
* Steps to create a .cyacd2 file with a CRC application signature:
* -# Copy the path to the CyMCUElfTool binary. The path can be found in the
*    folder with ModusToolbox tools (for example
*    /ModusToolbox/tools_2.0/cymcuelftool-1.0/bin/cymcuelftool).
* -# Update the application ELF with a CRC checksum (\<MCUELFTOOL\> - the copied
*     path to the binary):
*    \code <MCUELFTOOL> --sign app.elf CRC --output app_crc.elf \endcode
* -# Generate a .cyacd2 file:
*    \code <MCUELFTOOL> -P app_crc.elf --output app.cyacd2 \endcode
*
* These commands can be added as post build steps to the build Makefile.
* 
* For the SHA1 application signature, use command
* (\ref group_dfu_ucase_checksum):
* \code <MCUELFTOOL> --sign app.elf SHA1 --output app_crc.elf \endcode
*
********************************************************************************
* \section section_dfu_toolchain Supported Software and Tools
********************************************************************************
*
* This version of the DFU Middleware was validated for the compatibility 
* with the following Software and Tools:
* 
* <table class="doxtable">
*   <tr>
*     <th>Software and Tools</th>
*     <th>Version</th>
*   </tr>
*   <tr>
*     <td>ModusToolbox Software Environment</td>
*     <td>2.0</td>
*   </tr>
*   <tr>
*     <td>- ModusToolbox Device Configurator</td>
*     <td>2.0</td>
*   </tr>
*   <tr>
*     <td>- Device Firmware Update Host Tool</td>
*     <td>1.1</td>
*   </tr>
*   <tr>
*     <td>- CyMCUElfTool</td>
*     <td>1.0</td>
*   </tr>
*   <tr>
*     <td>GCC Compiler</td>
*     <td>7.2.1</td>
*   </tr>
*   <tr>
*     <td>IAR Compiler</td>
*     <td>8.32</td>
*   </tr>
*   <tr>
*     <td>ARM Compiler 6</td>
*     <td>6.11</td>
*   </tr>
* </table>
*
********************************************************************************
* \section group_dfu_MISRA MISRA-C Compliance
********************************************************************************
*
* <table class="doxtable">
*   <tr>
*     <th>MISRA Rule</th>
*     <th>Rule Class (Required/Advisory)</th>
*     <th>Rule Description</th>
*     <th>Description of Deviation(s)</th>
*   </tr>
*   <tr>
*     <td>1.1</td>
*     <td>R</td>
*     <td>This rule states that code shall conform to C ISO/IEC 9899:1990
*         standard.</td>
*     <td>DFU middleware supports the ISO:C99 standard.</td>
*   </tr>
*   <tr>
*     <td>2.3</td>
*     <td>A</td>
*     <td>Nested comments are not recognized in the ISO standard.</td>
*     <td>
*         The comments provide the useful WEB link to the additional 
*         documentation.
*     </td>
*   </tr>
*   <tr>
*     <td>3.1</td>
*     <td>R</td>
*     <td>All usage of implementation defined behavior shall be documented</td>
*     <td>The DFU SDK deviates rule 11.3 which triggers rule 3.1.
*         The ANSI symbols Dollar and Back quote are used by PDL
*         headers before being preprocessed.</td>
*   </tr>
*   <tr>
*     <td>5.6</td>
*     <td>A</td>
*     <td>To avoid confusion, no identifier in one name space should have the
*         same spelling as an identifier in another name space, with the
*         exception of structure and union member names</td>
*     <td>This rule is deviated because the generalized implementation approach
*         requires having the same names.</td>
*   </tr>
*   <tr>
*     <td>8.7</td>
*     <td>R</td>
*     <td>Objects shall be defined at block scope if they are only accessed from
*         within a single function. That is, minimize the scope of objects and
*         variables</td>
*     <td>For some communication APIs, an object scope can be reduced, but that
*          is not because of the generalized implementation approach.</td>
*   </tr>
*   <tr>
*     <td>9.2</td>
*     <td>R</td>
*     <td>Braces shall be used to indicate and match the structure in the
*         nonzero initialization of arrays and structures</td>
*     <td>This rule is deviated in the dfu_user.c file,
*         it is manually checked to be valid.</td>
*   </tr>
*   <tr>
*     <td>11.3</td>
*     <td>A</td>
*     <td>Avoid casts between a pointer type and an integral type</td>
*     <td>There are a few casts between the pointer and uint32_t type in
*         the cy_dfu.c file. All of them are manually verified and reviewed
*         to be safe.</td>
*   </tr>
*   <tr>
*     <td>11.4</td>
*     <td>A</td>
*     <td>A cast should not be performed between a pointer to object type and
*         a different pointer to object type.</td>
*     <td>Casts involving pointers are conducted with caution that the pointers
*         are correctly aligned for the type of the object being pointed to.
*     </td>
*   </tr>
*   <tr>
*     <td>11.5</td>
*     <td>R</td>
*     <td>Not performed, the cast that removes any const or volatile 
*           qualification from the type addressed by a pointer.</td>
*     <td>The removal of the volatile qualification inside the function has no 
*           side effects.</td>
*   </tr>
*   <tr>
*     <td>13.7</td>
*     <td>R</td>
*     <td>Boolean operations whose results are invariant shall not be
*         permitted.</td>
*     <td>MISRA-C:2004 is not smart enough to understand the function is weak,
*         thus may return values other than in the default implementation.</td>
*   </tr>
*   <tr>
*     <td>14.1</td>
*     <td>R</td>
*     <td>There shall be no unreachable code. This refers to code which cannot,
*         under any circumstances, be reached.</td>
*     <td>MISRA-C:2004 does not understand that a weak function is supposed to 
*         be overwritten in the customer project. Thus assumes the function 
*         returns always the same fixed value, so any if() branch testing 
*         weak-function return-value may be treated as unreachable code.</td>
*   </tr>
*   <tr>
*     <td>14.2</td>
*     <td>R</td>
*     <td>All non-null statements shall either have at least one side-effect,
*         however executed, or cause control flow to change.</td>
*     <td>MISRA-C:2004 is not smart enough to understand the intention of
*         the (void)unused_param; statement. \n
*         GCC, ARM and IAR compilers understand the intention of this.</td>
*   </tr>
*   <tr>
*     <td>16.7</td>
*     <td>A</td>
*     <td>The object addressed by the pointer parameter is not modified
*         and so the pointer could be of type 'pointer to const'.</td>
*     <td>Some DFU SDK functions are weak, and can be redefined in
*         the user code. They contain non-const pointer parameters intentionally
*         to be more generic .</td>
*   </tr>
*   <tr>
*     <td>17.4</td>
*     <td>R</td>
*     <td>Array indexing shall be the only allowed form of pointer arithmetic
*         </td>
*     <td>There are several instances of the pointer arithmetic in 
*         cy_dfu.c. They cannot be avoided, so are manually checked
*         and reviewed to be safe.</td>
*   </tr>
*   <tr>
*     <td>19.7</td>
*     <td>A</td>
*     <td>A function shall be used in preference to a function-like macro</td>
*     <td>Function-like macros are used for performance reasons.</td>
*   </tr>
*   <tr>
*     <td>19.13</td>
*     <td>A</td>
*     <td>Avoid use of the # and ## preprocessor directives when possible</td>
*     <td>The directive ## of the preprocessor used in the  
*         transport_uart/i2c/spi/usb_cdc.c. The use of ## is intentional and 
*         allows setting the UART/I2C/SPI/USBFS component name in one place and 
*         does not change it throughout the whole transport_uart/i2c/spi/
*         usb_ucd.c file.</td>
*   </tr>
*   <tr>
*     <td>21.1</td>
*     <td>R</td>
*     <td>Minimization of run-time failures shall be ensured by the use of
*         at least one of:
*         * static analysis tools/techniques;
*         * dynamic analysis tools/techniques;
*         * explicit coding of checks to handle run-time faults.</td>
*     <td>Redundant operations are present because of the generalized
*         implementation approach.</td>
*   </tr>
* </table>
*
********************************************************************************
* \section section_dfu_errata Errata
********************************************************************************
*
*   No know issues.
*
********************************************************************************
* \section group_dfu_changelog Changelog
********************************************************************************
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
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
*         <li>Linker scripts updated for PSoC6 Rev *A devices.</li>
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
* * [AN213924](http://www.cypress.com/an213924) DFU SDK User Guide
* * [CE213903](http://www.cypress.com/ce213903) DFU SDK Basic Communication Code Examples
* * [CE216767](http://www.cypress.com/ce216767) DFU SDK BLE OTA Code Example
* * [PSoC 6 Peripheral Driver Library API Reference](https://cypresssemiconductorco.github.io/psoc6pdl/pdl_api_reference_manual/html/index.html)
* * [Cypress USB Device Middleware Library API Reference](https://cypresssemiconductorco.github.io/usbdev/usbfs_dev_api_reference_manual/html/index.html)
* * [BLE Middleware API Reference Guide](https://cypresssemiconductorco.github.io/middleware-ble/ble_api_reference_manual/html/index.html)
* * [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* * [PSoC 6 SDK Examples](https://github.com/cypresssemiconductorco/Code-Examples-for-the-ModusToolbox-PSoC-6-SDK)
* * [ModusToolbox Device Configurator Tool Guide](https://www.cypress.com/ModusToolboxDeviceConfig)
* * [ModusToolbox Device Firmware Update Host Tool](https://www.cypress.com/ModusToolboxDFUHostTool)
* * [ModusToolbox USB Configurator Tool Guide](https://www.cypress.com/ModusToolboxUSBConfig)
* * [ModusToolbox BT Configurator Tool Guide](https://www.cypress.com/ModusToolboxBLEConfig)
* * [CySmart - BLE Test and Debug Tool](http://www.cypress.com/documentation/software-and-drivers/cysmart-bluetooth-le-test-and-debug-tool)
* * [CySmart – Mobile App](https://www.cypress.com/documentation/software-and-drivers/cysmart-mobile-app)
* * [PSoC 6 BLE Pioneer Kit](http://www.cypress.com/CY8CKIT-062-BLE)
* * [PSoC 6 WiFi-BT Pioneer Kit](http://www.cypress.com/CY8CKIT-062-WiFi-BT)
* * [PSoC 6 Wi-Fi BT Prototyping Kit](http://www.cypress.com/cy8cproto-062-4343w)
* * [PSoC 6 MCU Datasheets](http://www.cypress.com/psoc6ds)
* * [PSoC 6 MCU Application Notes](http://www.cypress.com/psoc6an)
* * [PSoC 6 MCU Technical Reference Manuals](http://www.cypress.com/psoc6trm)
* * [Cypress Semiconductor](http://www.cypress.com)
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

#if CY_DFU_OPT_CRYPTO_HW != 0
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
#define CY_DFU_SDK_MW_VERSION_MAJOR       (4)

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
#define CY_DFU_STATE_NONE          (0u) /**< Updating has not yet started, no Enter packet received */
#define CY_DFU_STATE_UPDATING      (1u) /**< Updating is in process             */
#define CY_DFU_STATE_FINISHED      (2u) /**< Updating has finished successfully */
#define CY_DFU_STATE_FAILED        (3u) /**< Updating has finished with an error   */
/** \} group_dfu_macro_state */

#define CY_DFU_PACKET_MIN_SIZE     (0x07u) /**< The smallest valid DFU packet size */

/**
* \defgroup group_dfu_macro_commands DFU Commands
* \{
*/
#define CY_DFU_CMD_ENTER           (0x38u) /**< DFU command: Enter DFU           */
#define CY_DFU_CMD_EXIT            (0x3Bu) /**< DFU command: Exit DFU            */
#define CY_DFU_CMD_PROGRAM_DATA    (0x49u) /**< DFU command: Program Data               */
#define CY_DFU_CMD_VERIFY_DATA     (0x4Au) /**< DFU command: Verify Data                */
#define CY_DFU_CMD_ERASE_DATA      (0x44u) /**< DFU command: Erase Data                 */
#define CY_DFU_CMD_VERIFY_APP      (0x31u) /**< DFU command: Verify Application         */
#define CY_DFU_CMD_SEND_DATA       (0x37u) /**< DFU command: Send Data                  */
#define CY_DFU_CMD_SEND_DATA_WR    (0x47u) /**< DFU command: Send Data without Response */
#define CY_DFU_CMD_SYNC            (0x35u) /**< DFU command: Sync DFU            */
#define CY_DFU_CMD_SET_APP_META    (0x4Cu) /**< DFU command: Set Application Metadata   */
#define CY_DFU_CMD_GET_METADATA    (0x3Cu) /**< DFU command: Get Metadata               */
#define CY_DFU_CMD_SET_EIVECTOR    (0x4Du) /**< DFU command: Set EI Vector              */

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

#define CY_DFU_IOCTL_READ          (0x00u) /**< Read data into the buffer                         */
#define CY_DFU_IOCTL_COMPARE       (0x01u) /**< Compare read data with the data in the buffer */

#define CY_DFU_IOCTL_WRITE         (0x00u) /**< Write the buffer to communication */
#define CY_DFU_IOCTL_ERASE         (0x01u) /**< Erase memory page             */

#define CY_DFU_IOCTL_BHP           (0x02u) /**< Data from/to DFU Host. It may require decryption. */

/** \} group_dfu_macro_ioctl */

/**
* \defgroup group_dfu_macro_response_size Response Size
* \{
*/

#define CY_DFU_RSP_SIZE_0          (0u)    /**< Data size for most DFU commands responses */
#define CY_DFU_RSP_SIZE_VERIFY_APP (1u)    /**< Data size for 'Verify Application' DFU command response */

/** \} group_dfu_macro_response_size */

/** DFU SDK PDL ID */
#define CY_DFU_ID                  CY_PDL_DRV_ID(0x06u)

/** \} group_dfu_macro */

/**
* \addtogroup group_dfu_data_structs
* \{
*/

/** 
 * Working parameters for some DFU SDK APIs to be initialized before calling DFU API.
 * */
typedef struct
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
     * The amount of time (in milliseconds) for which the
     * communication interface should wait to receive a new data packet
     * from Host in \ref Cy_DFU_Continue(). A typical value is 20 ms.
     */
    uint32_t  timeout;
    /**
     * Set with the Set App Metadata DFU command.
     * Used to determine an appId of DFU image
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
    
#if CY_DFU_OPT_SET_EIVECTOR != 0
    /**
    * The pointer to the Encryption Initialization Vector buffer.
    * Must be 0-, 8-, or 16-byte long and 4-byte aligned.
    * This may be used in \ref Cy_DFU_ReadData and \ref Cy_DFU_WriteData
    * to encrypt or decrypt data when the CY_DFU_IOCTL_BHP flag is set in the
    * ctl parameter.
    */
    uint8_t *encryptionVector;
#endif /* CY_DFU_OPT_SET_EIVECTOR != 0 */
    
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
* \addtogroup group_dfu_enums
* \{
*/

/** Used to return the statuses of most DFU SDK APIs */
typedef enum
{
    /** Correct status, No error */
    CY_DFU_SUCCESS         =                                   0x00u,
    /** Verification failed */
    CY_DFU_ERROR_VERIFY    = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x02u,
    /** The length of received packet is outside of expected range */
    CY_DFU_ERROR_LENGTH    = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x03u,
    /** The data in the received packet is invalid */
    CY_DFU_ERROR_DATA      = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x04u,
    /** Command is not recognized */
    CY_DFU_ERROR_CMD       = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x05u,
    /** The checksum does not match the expected value */
    CY_DFU_ERROR_CHECKSUM  = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x08u,
    /** Wrong address */
    CY_DFU_ERROR_ADDRESS   = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0Au,
    /** Command timed out */
    CY_DFU_ERROR_TIMEOUT   = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x40u,
    /** Unknown DFU error, this shall not happen */
    CY_DFU_ERROR_UNKNOWN   = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0Fu
} cy_en_dfu_status_t;


/** \} group_dfu_enums */

/**
* \addtogroup group_dfu_globals
* \{
*/

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
extern uint8_t __cy_boot_metadata_addr;
/**
 * Metadata row size.
 * DFU uses this symbol to access metadata.
 */
extern uint8_t __cy_boot_metadata_length;
/**
 * Product ID.
 * CyMCUElfTool uses this value to place in the .cyacd2 header.
 * DFU uses this value to verify if an image is compatible with the device.
 */
extern uint8_t __cy_product_id;
/**
 * Checksum Algorithm of the DFU Host Command/Response Protocol packet.
 * Possible values 
 * - 0 for Basic Summation algorithm
 * - 1 for CRC-16 algorithm 
 * \note Must be aligned with \ref CY_DFU_OPT_PACKET_CRC
 */
extern uint8_t __cy_checksum_type;
/**
 * Current application number
 */
extern uint8_t __cy_app_id;
/**
 * CPU1 vector table address, if present 
 */
extern uint8_t __cy_app_core1_start_addr;
/** \} group_dfu_globals_external_elf_symbols */

/** \} group_dfu_globals */

/**
* \addtogroup group_dfu_functions
* \{
*/

cy_en_dfu_status_t Cy_DFU_Complete(uint32_t *state, uint32_t timeout);
cy_en_dfu_status_t Cy_DFU_Init(uint32_t *state, cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_Continue(uint32_t *state, cy_stc_dfu_params_t *params);

void Cy_DFU_ExecuteApp(uint32_t appId);
cy_en_dfu_status_t Cy_DFU_SwitchToApp(uint32_t appId);

uint32_t Cy_DFU_DataChecksum(const uint8_t *address, uint32_t length, cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_ValidateMetadata(uint32_t metadataAddress, cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_ValidateApp(uint32_t appId, cy_stc_dfu_params_t *params);

uint32_t Cy_DFU_GetRunningApp(void);

cy_en_dfu_status_t Cy_DFU_GetAppMetadata(uint32_t appId, uint32_t *verifyAddress, uint32_t *verifySize);

#if CY_DFU_METADATA_WRITABLE != 0
    cy_en_dfu_status_t Cy_DFU_SetAppMetadata(uint32_t appId, uint32_t verifyAddress,
                                                       uint32_t verifySize, cy_stc_dfu_params_t *params);
#endif /* if CY_DFU_METADATA_WRITABLE != 0 */

cy_en_dfu_status_t Cy_DFU_CopyApp(uint32_t destAddress, uint32_t srcAddress, uint32_t length,
                                            uint32_t rowSize, cy_stc_dfu_params_t *params);

void Cy_DFU_OnResetApp0(void);

/* These 2 IO functions have to be re-implemented in the user's code */
cy_en_dfu_status_t Cy_DFU_ReadData (uint32_t address, uint32_t length, uint32_t ctl, 
                                              cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_WriteData(uint32_t address, uint32_t length, uint32_t ctl, 
                                              cy_stc_dfu_params_t *params);

/* These 5 communication functions have to be re-implemented in the user's code */
cy_en_dfu_status_t Cy_DFU_TransportRead (uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout);
cy_en_dfu_status_t Cy_DFU_TransportWrite(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout);
void Cy_DFU_TransportReset(void);
void Cy_DFU_TransportStart(void);
void Cy_DFU_TransportStop(void);
/** \} group_dfu_functions */


/***************************************
*  Internal declarations
****************************************/
/** \cond INTERNAL */

#if !defined(CY_PSOC_CREATOR_USED)

    /* Should be 0 in a non-Creator flow */
    #define CY_DFU_SILICON_ID      (0u)
    #define CY_DFU_SILICON_REV     (0u)
#else
    #include "cy_device_headers.h" /* for CY_SILICON_ID            */
    #include <cyfitter.h>          /* for CYDEV_CHIP_REVISION_USED */

    #define CY_DFU_SILICON_ID  CY_SILICON_ID
    #define CY_DFU_SILICON_REV CYDEV_CHIP_REVISION_USED
#endif /* defined CY_DOXYGEN */


/* Cypress Basic Application Format (CyBAF) */
#define CY_DFU_BASIC_APP           (0u)
/* Cypress Secure Application Format (CySAF) - NOT SUPPORTED */
#define CY_DFU_CYPRESS_APP         (1u)
/* Simplified Secure Application Format (SSAF) - NOT SUPPORTED */
#define CY_DFU_SIMPLIFIED_APP      (2u)

/* Set the application format. Only CyBAF is supported. */
#define CY_DFU_APP_FORMAT          (CY_DFU_BASIC_APP)

#define CY_DFU_VERIFY_FAST         (0u)    /* Verification includes only 
                                            * application check */
#define CY_DFU_VERIFY_FULL         (1u)    /* Verification includes application, 
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
