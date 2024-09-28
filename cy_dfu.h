/***************************************************************************//**
* \file cy_dfu.h
* \version 5.2
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
* - The application loader receives the program and switch to
*    the new application.
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
*   e.g. USB, UART, I2C, SPI
* - Supports dynamic switching (during runtime) of the communication interfaces
* - Provides ready-for-use transport interface templates based on HAL drivers
*   for CAT1 devices and PDL drivers for CAT2 devices
* - Supported flows: Basic bootloader and MCUBoot compatibility
* - Device support: CAT1A, CAT2 (Basic bootloader flow),
*   CAT1C (MCUBoot compatibility flow)
* - Programs a firmware image to the specified address in internal flash,
*   XIP region or any external memory that supports the DFU API
* - Copies applications
* - Validates applications
* - Updates safely - updates at a temporary location, validates, and if valid,
*   overwrites the working image
* - Switches applications - passes parameters in RAM when switching
*   applications
* - Supports encrypted image files - transfers encrypted images without
*   decrypting in the middle
* - Supports many application images - the number of applications is limited only by
*   the metadata size; each image can be an application loader, for example,
*   512-byte metadata supports up to 63 applications
* - Supports customization
* - Supports the CRC-32 checksum to validate data.
* - Supports extend of the host command/response protocol with custom commands.
*
********************************************************************************
* \section section_dfu_quick_start Quick Start Guide
********************************************************************************
* \subsection subsection_dfu_qsg_basic Basic Bootloader Flow
********************************************************************************
*
* The DFU SDK is used to design updating applications of
* arbitrary flexibility and complexity. Infineon DFU middleware can be used in
* various software environments. For details, refer to RELEASE.md file.
* For a quick start, use the Code Examples.
* The portfolio of Code Examples continuously extends at
* [Infineon GitHub](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software).
*
* The ModusToolbox&trade; Quick Start Guide (QSG) assumes ModusToolbox&trade; 3.x is installed
* with all required tools.
*
* The following steps are to set up and build a basic DFU loader and loadable
* applications. The DFU loader application uses the I2C transport interface.
* The steps assume that the user builds an application for CY8CKIT-062-WIFI-BT
* (CAT1A device) or CY8CKIT-149 kits (CAT2 device)
* based on a starter Hello_World ModusToolbox&trade; project.
*
* \note For other kits or devices, update default linker scripts with the valid
* memory addresses. For details, refer to \ref group_dfu_config_linker_scripts.
*
* \subsubsection ssection_dfu_step_0 STEP 0: Projects preparation
*
* 1. Create a project for CY8CKIT-062-WIFI-BT or CY8CKIT-149 with the DFU loader
*    application using the Hello_World template application ("Getting started"
*    section in the Project Creator).
*    Name it "QSG_DFU_App0_I2C". For details, refer to the ModusToolbox&trade; 3.x
*    IDE Quick Start Guide.
* 2. Create a project for the DFU loadable application in the same way and name
*    it "QSG_DFU_App1_Hello_World".
* 3. Include the DFU middleware into each project using the ModusToolbox&trade; Library
*    Manager or download it from GitHub and copy it to the project manually.
* 4. Include a DFU header in main.c of each project to get access to DFU API:
*    \snippet snippet/main.c snipped_cy_dfu_include
*
* \subsubsection ssection_dfu_step_1 STEP 1: Setup Loader Application QSG_DFU_App0_I2C
*
* 1. Copy the app0 linker script files and put them next to main.c:
*      - For CY8CKIT-062-WIFI-BT kit: <br>
*        [DFU location]\\linker_scripts\\CAT1A\\TOOLCHAIN_<COMPILER>\\dfu_cm4_app0.[ext]
*      - For CY8CKIT-149 kit: <br>
*        [DFU location]\\linker_scripts\\CAT2\\TOOLCHAIN_<COMPILER>\\dfu_cm0p_app0.[ext] <br>
*        [DFU location] - The folder with the DFU library downloaded in STEP 0 <br>
*        [ext] - The linker script extension according to the used compiler.
*
*    For example, for CY8CKIT-062-WIFI-BT kit and GCC ARM compiler, with DFU
*    loaded by the Library Manager as a "Shared Git Repo" copy <br>
*    ..\\mtb_shared\\dfu\\[VERSION]\\linker_scripts\\CAT1A\\TOOLCHAIN_GCC_ARM\\dfu_cm4_app0.ld file
*
*   \note For ARM compiler, copy additional **dfu_common.h** and **dfu_elf_symbols.c**
*         files to the project. Those files are located in the same folder as the selected linker file.

* 2. Update project's Makefile to add DFU user and I2C transport components :
*    locate the **COMPONENTS** variable and add **DFU_USER** and **DFU_I2C**:
*    \code COMPONENTS=DFU_USER DFU_I2C \endcode
*
* 3. For CY8CKIT-149 kit, configure  the I2C communication interface.
*    \warning Not needed for the CAT1 devices - configuration is done by HAL.\n
*     Please check/setup the required pins assignments in the BSP.
*
*    Open the ModusToolbox&trade; Device Configurator and enable SCB on the Peripheral
*    tab under Communication section with the following parameter.
*
*    For CY8CKIT-149, SCB 1 is connected to the KitProg.
*    |SCB parameter name     | Value         |
*    |-----------------------|---------------|
*    |Personality            | I2C           |
*    |Name                   | **DFU_I2C**   |
*    |Mode                   | Slave         |
*    |Data Rate (kbps)       | 100           |
*    |Slave Address (7-bit)  | 12            |
*    \warning The SCB personality must be **I2C** and the name must be **DFU_I2C**.
*
*   \image html dfu_basic_i2c_kit149.png
*
*    See \ref group_dfu_mtb_cfg
*
* \subsubsection ssection_dfu_step_2 STEP 2: Update Loader QSG_DFU_App0_I2C main.c
*
* 1. Include a DFU reset handler to start the appropriate application after
*    a reset:
*    \snippet snippet/main.c snipped_cy_dfu_onreset
* 2. Initialize the variables and call the DFU initialization function:
*    \snippet snippet/main.c snipped_cy_dfu_init
* 3. Initialize the DFU transport layer:
*    \snippet snippet/main.c snipped_cy_dfu_init_comm
* 4. Update the main loop with the Host Command/Response protocol processing:
*    \snippet snippet/main.c snipped_cy_dfu_command_process
*    \warning An additional timeout in the main loop can break the DFU transfer.
*             For example, CY8CKIT-149 Hello_World template application uses the 0.5
*             seconds delay for the LED blinking. This needs to be disabled during
*             the DFU image transfer.
* 5. Update the main loop with a routine to switch to the loaded
*    QSG_DFU_App1_Hello_World application:
*
*    For example, to switch by pressing the kit user button using HAL drivers:
*    - Add pin initialization to the main() function initialization section:
*    \snippet snippet/main.c snipped_cy_dfu_switch_app1_init
*    - Add the following routine to the main loop section:
*    \snippet snippet/main.c snipped_cy_dfu_switch_app1_hal
*    \note For the CAT2 device, to use HAL drivers add mtb-hal-cat2 library in the
*    Library Manager add the CY_USING_HAL define to the Makefile:
*    \code DEFINES=CY_USING_HAL \endcode
*    include cyhal.h in the main.c
*    \code #include "cyhal.h" \endcode
*
* \subsubsection ssection_dfu_step_3 STEP 3: Build and Program Loader QSG_DFU_App0_I2C
* 1. Update the project Makefile to use the previously copied DFU linker script
*    by setting the LINKER_SCRIPT variable.
*      - CY8CKIT-062-WIFI-BT + GCC_ARM:
*        \code LINKER_SCRIPT=dfu_cm4_app0.ld \endcode
*      - CY8CKIT-149 + GCC_ARM:
*        \code LINKER_SCRIPT=dfu_cm0p_app0.ld \endcode
*
* 2. Connect your kit to the computer. Build and program the device.
*    \warning The DFU loader application requires an XRES reset after programming to
*    initialize the <i>ram_common</i> data section.
* 3. Observe the kit LED blinking.
*
*  \anchor loadable_DFU_app_steps
* \subsubsection ssection_dfu_step_4 STEP 4: Setup Loadable QSG_DFU_App1_Hello_World
*
* 1. Copy the app1 linker script file and put them next to main.c. The linker
*    script files are located at:
*      - CY8CKIT-062-WIFI-BT kit: <br>
*        [[dfu location]]\\[VERSION]\\linker_scripts\\CAT1A\\TOOLCHAIN_<COMPILER>\\dfu_cm4_app1.[ext]
*      - CY8CKIT-149 kit: <br>
*        [[dfu location]]\\[VERSION]\\linker_scripts\\CAT2\\TOOLCHAIN_<COMPILER>\\dfu_cm0p_app1.[ext]
*    For the GCC ARM compiler, copy dfu_cm4_app0.ld (CY8CKIT-062-WIFI-BT kit) of dfu_cm0p_app0.ld file (CY8CKIT-149 kit).
*   \note For the ARM compiler, copy additional **dfu_common.h** and **dfu_elf_symbols.c**
*         files to the project. Those files are located in the same folder as the selected linker file.
*
* \subsubsection ssection_dfu_step_5 STEP 5: Update Loadable QSG_DFU_App1_Hello_World main.c
*
* 1. Update the main.c file with the .cy_app_signature section
*    \snippet snippet/main.c snipped_cy_dfu_app_signature
*
* \subsubsection ssection_dfu_step_6 STEP 6: Build and Program Patch
*
* 1. Update the project Makefile to use the previously copied DFU linker script by
*    setting the LINKER_SCRIPT variable.
*      - CY8CKIT-062-WIFI-BT + GCC_ARM:
*        \code LINKER_SCRIPT=dfu_cm4_app1.ld \endcode
*      - CY8CKIT-149 + GCC_ARM:
*        \code LINKER_SCRIPT=dfu_cm0p_app1.ld \endcode
* 2. Add the post build step to run CyMCUElfTool to generate a patch file in
*    the *.cyacd2 format (see CyMCUElfTool User Guide):
*     + Update the application ELF with a CRC checksum:
*         \<MCUELFTOOL\> --sign app.elf CRC --output app_crc.elf
*     + Generate a patch file:
*         \<MCUELFTOOL\> -P app_crc.elf --output app.cyacd2
*
*    Generate a *.cyacd2 file in the project root.\n
*
* \code
* # Path to Elf tool directory.
* CY_MCUELFTOOL_DIR=$(wildcard $(CY_TOOLS_DIR)/cymcuelftool-*)
* # CY MCU ELF tool executable path.
* ifeq ($(OS),Windows_NT)
*     CY_MCUELFTOOL=$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool.exe
* else
*     CY_MCUELFTOOL=$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool
* endif
* BINARY_PATH=./build/$(TARGET)/$(CONFIG)/$(APPNAME)
* # Custom post-build commands to run.
* POSTBUILD="$(CY_MCUELFTOOL)" --sign $(BINARY_PATH).elf \
*        CRC --output $(APPNAME)_crc.elf && \
*        "$(CY_MCUELFTOOL)" -P $(APPNAME)_crc.elf --output $(APPNAME)_crc.cyacd2
* \endcode
*
* 3. Build a project.
* 4. Open the DFU Host Tool. Connect to the device. Select the generated .cyacd2
*     in the project root and program it to the device.
*    \image html dfu_qsg_hti2c.png
* 5. QSG_DFU_App1_Hello_World application will start after successful programming.
*    Observe the LED blinking and UART output.
* 6. Update the QSG_DFU_App1_Hello_World application (e.g. change blinking led
*    frequency or UART output) and build it.
* 7. Press the kit reset button to return to the loader application and program
*    the updated QSG_DFU_App1_Hello_World.
*    Observe the project updated behavior.
*    \note The current application can be changed from the firmware by calling the
*    \ref Cy_DFU_ExecuteApp function.
*
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
* 1. Create a ModusToolbox&trade; application for the CAT1A or CAT1C devices. For example,
*   the CY8CKIT-062-WIFI-BT kit can be used as CAT1A or KIT_XMC72_EVK as CAT1C.
*   Create a new application in the ModusToolbox&trade; IDE using an appropriate BSP
*   and an empty application as a template (Empty App). Name it "DFU_App0". For details, refer to the
*   ModusToolbox&trade; 3.x IDE Quick Start Guide.
*
* 2. Include the DFU middleware into the project using the ModusToolbox&trade; Library
*    Manager.
*
* 3. Add the DFU transport components to project's Makefile to enable the transport interface(s).
*    In our case, I2C is used:
*   \code COMPONENTS += DFU_I2C \endcode
*
* 4. Update project's Makefile to use MCUBoot flow:
*   \code DEFINES += CY_DFU_FLOW=CY_DFU_MCUBOOT_FLOW \endcode
*   \code COMPONENTS += DFU_USER \endcode
*
* \subsubsection subsubsection_qsg_mcuboot_s2 STEP2: Add DFU logic to main.c
*
* 1. Include the required headers.
*   \snippet snippet/main.c snipped_cy_dfu_include
* 2. Initialize the variables and call the DFU initialization function:
*  \snippet snippet/main.c snipped_cy_dfu_init
* 3. Initialize the DFU transport layer:
*  \snippet snippet/main.c snipped_cy_dfu_init_comm
* 4. Update the main loop with the Host Command/Response protocol processing:
*  \snippet snippet/main.c snipped_cy_dfu_mcbtflw_command_process
*
* \subsubsection subsubsection_qsg_mcuboot_s3 STEP3: Build and Program Loader DFU_App0
* Connect your kit to the computer. Build and program the device.
* \note The CY_DFU_PRODUCT warning displays if default values are used and they need to be
*       changed. CY_DFU_PRODUCT can be defined in the Makefile.
*
* \subsubsection subsubsection_qsg_mcuboot_s4 STEP4: Create a loadable application (Application 1).
* 1. Create a ModusToolbox&trade; application for the same devices as in STEP1. Use an empty
*   application as a template (Empty App). Name it "DFU_App1".
*
* 2. Disable the adding the CM0+ core code to the result binary. For the CAT1A device
*  (CY8CKIT-062-WIFI-BT kit), disable the CM0P_SLEEP component in Makefile.
*  \code DISABLE_COMPONENTS=CM0P_SLEEP \endcode
*  For the CAT1C device (KIT_XMC72_EVK kit), disable the XMC7xDUAL_CM0P_SLEEP component in Makefile.
*  \code DISABLE_COMPONENTS=XMC7xDUAL_CM0P_SLEEP \endcode
*
* 3. Update the project post build steps to generate HEX files with an offset
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
* \section section_dfu_configuration Configuration Considerations
********************************************************************************
*
********************************************************************************
* \subsection group_dfu_config_linker_scripts Linker scripts
********************************************************************************
*
* The DFU SDK projects linker scripts differ from the default
* startup linker scripts.
*
* The DFU middleware contains two sets of linker script files for the CAT1A and CAT2-based devices.
* The DFU linker scripts include the following files:
* - CAT1A:
*     - dfu_cm4_app0.{ld, icf, scat}, dfu_cm4_app1.{ld, icf, scat} for ARM GCC,
*        IAR, and ARM compilers.
*     - dfu_common.h and dfu_elf_symbols.c for the ARM compiler.
*
* - CAT2:
*     - dfu_cm0p_app0.{ld, icf, scat}, dfu_cm0p_app1.{ld, icf, scat} for ARM GCC,
*        IAR, and ARM compilers.
*     - dfu_common.h and dfu_elf_symbols.c for the ARM compiler.
*
* These files define the symbols for the memory layout for each application
* inside the device.
*
* \par Memory layout of GCC_ARM linker scripts (dfu_{cm0p, cm4}_{app0, app1}.ld)
*
* This part of the GCC linker script files must have the same memory layout
* across all the application projects in the designed device.
* Any changes made to any application must be copied to other
* applications linker script files.
*
* Memory regions:
* * <i>flash_app{X}</i> - Code and data
*   of the user application {X}.
* * <i>flash_boot_meta</i> - For the DFU SDK
*   metadata. Cypress DFU SDK code examples place DFU SDK metadata
*   inside this region.
* * <i>ram_common</i> - Shared between the DFU SDK applications.
*   The user can place it anywhere inside the RAM,
*   So, one app sets some values there, switches to another app.
*   Then app may read or update the values.
* * <i>ram_app{X}</i> - data, stack, heap etc. for the user app{X}.
*
* Also, the linker script files for CAT1A include the following memory regions:
* * <i>flash_cm0p</i> - Code and data
*   of the default application CM0+ CPU.
*   \warning There are different CM0+ images available. Please adjust the size
*   of the CM0+ application image according to the size in  BSP default
*   linker script.
* * <i>sflash_user_data</i>, <i>eFuse</i>, <i>flash_toc</i>, <i>em_eeprom</i>,
*   <i>xip</i> - These regions are not used by typical DFU SDK code examples.
*   They are kept because they may be used in user code.
*
* ELF file symbols:
* CyMCUElfTool uses special ELF file symbols besides the command-line arguments for
* its configuration. These symbols are defined in each linker script.
* 1. __cy_memory_{N}_start - Defines the start address of the memory region.
*    __cy_memory_{N}_length - Defines the length of the memory region.
*    __cy_memory_{N}_row_size - Defines the row size of the memory region.
*
*    CyMCUElfTool uses these symbols to determine which memory regions to
*    place into the output files. I.e. without these symbols, some data,
*    like XIP may be absent in the output file.
*    These symbols are critical for the .cyacd2 file generation, CyMCUElfTool
*    must know the row size of all the data being exported to the .cyacd2
*    file. The updating is done by rows, and a row size may vary across
*    the memory regions.
*
*    E.g. The internal flash of PSoC6 devices start at address 0x1000_0000 and
*    the length and row size may be device-dependent.
*    For example, if the length and size are 512KB and 512 bytes, the memory symbols for the internal flash will be:
*    \code
*        __cy_memory_0_start    = 0x10000000;
*        __cy_memory_0_length   = 512 * 1024;
*        __cy_memory_0_row_size = 512;
*    \endcode
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
*    If the user decides to use a different mechanism for the SDK metadata
*    initialization, these symbols can be removed.
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
* \par File dfu_{cm0p, cm4}_app0.ld
*
* This file is a linker script for the app0 for DFU SDK applications.
*
* It is similar to the default startup GCC's linker script but contains the following changes:
* 1. The memory regions are separated between the CPU
*    application 0 and CPU application 1 described above.
*    For CAT1A devices, there is an additional region for the CM0+ application.
* 2. The DFU-specific ELF file symbols are described above.
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
*    If no CRC-32C at the end of the metadata is required, the section can
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
* \par File dfu_{cm0p, cm4}_app1.ld
*
* Used to create linker scripts for application \#2, .. \#N
* It is similar to dfu_{cm0p, cm4}_app0.ld linker script, but contains the following changes:
* - Region alias for flash and ram are flash_app1 and ram_app1
* - Application ID __cy_app_id = 1
* - For CAT1A devices, removed section for CM0+ CPU as it is allocated only once in scope
*   of the linker script dfu_cm4_app0.ld
*
* \par Files dfu_{cm0p, cm4}_{app0, app1}.{icf, scat}
*
* These files are the linker scripts for the IAR and ARM
* compilers for the DFU SDK applications.
*
* Their difference from the default startup linker scripts is similar to
* the DFU SDK GCC's linker scripts described above.
*
********************************************************************************
* \subsection group_dfu_mtb_cfg Use of the ModusToolbox&trade; tools for HW initialization
********************************************************************************
* The following section describes the communication interfaces settings in the Device Configurator
* required to use the included with DFU middleware communication files with the DFU Host tool.
* \warning The ModusToolbox&trade; Device Configurator is not used for templates based on the HAL drivers.\n
*  Please check/setup the required pins assignments in the BSP.
*
*
* \par I2C
*      Parameter name         | Value                                |
*      -----------------------|--------------------------------------
*      Personality alias name | DFU_I2C
*      Mode                   | Slave
*      Data Rate              | Any, I2C speed in DFU Host tool should be the same
*      Use TX FIFO            | True
*      Use RX FIFO            | True
*      Slave Address          | Any, I2C address in DFU Host tool should be the same
*
*      \image html dfu_basic_i2c.png
*
* \par SPI
*      Parameter name         | Value                                |
*      -----------------------|--------------------------------------
*      Personality alias name | DFU_SPI
*      Mode                   | Slave
*      Sub Mode               | Motorola
*      SCLK Mode              | Any, Sub Mode in DFU Host tool should be the same
*      Data Rate              | 1000 kbps (For other data rates, adjust the value of the SPI_BYTE_TO_BYTE macro in the transport_spi.c file)
*      Bit Order              | Any, Shift direction in DFU Host tool should be the same
*      RX Data Width          | 8
*      TX Data Width          | 8
*      SS Polarity            | Active Low
*
*      \note By default, used the Slave Select 1 line. To change it, update
*      the CY_SPI_SLAVE_SELECT macro in transport_spi.c file.
*
* \par UART
*      Parameter name         | Value                                |
*      -----------------------|--------------------------------------
*      Personality alias name | DFU_UART
*      Com Mode               | Standard
*      Baud Rate              | 115200 bps (For other baud rates, adjust the value of the UART_BYTE_TO_BYTE_TIMEOUT_US macro in transport_uart.c file)
*      Bit Order              | LSB first
*      Data Width             | 8 bits
*      Parity                 | Any, Parity in DFU Host tool should be the same
*      Stop Bits              | Any, Stop Bits in DFU Host tool should be the same
*
* \par USB CDC transports
* To set up the USB device personality in the ModusToolbox&trade; Device Configurator
* for the USB DFU transport for CY8CKIT-062-WIFI-BT, see the screenshots
* below. For other kits, verify the USB pins.
* \image html dfu_usb_cdc.png
*
* \par CAN FD
* 
* The CAN FD transport is supported by the following devices:
* - PSoC Control C3 (CAT1B),
* - XMC7000 (CAT1C).
* 
* To set up the CAN FD personality in the ModusToolbox&trade; Device Configurator
* for the CAN FD DFU transport for KIT_XMC72_EVK, see the screenshots
* below. For other kits, verify the CAN Rx and CAN Tx pins connections.
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
* \section section_dfu_design Design Considerations
********************************************************************************
*
********************************************************************************
* \subsection group_dfu_ucase_i2c Firmware Update via I2C
********************************************************************************
*
* See \ref section_dfu_quick_start for steps how to set up a DFU project that
* upgrades an application via the I2C transport interface.
*
********************************************************************************
* \subsection group_dfu_ucase_uart Firmware Update via UART
********************************************************************************
*
* See \ref section_dfu_quick_start for basic steps how to setup a DFU project.
* Specific steps for the UART transport support:
* - Add UART transport component in project's Makefile:
*    locate **COMPONENTS** variable and add **DFU_UART**:
*    \code COMPONENTS+=DFU_UART \endcode
* - For templates based on the PDL drivers:
*   - Select and configure the SCB block using the ModusToolbox&trade; Device
*     Configurator see \ref group_dfu_mtb_cfg or manually using
*     the configuration structures.
*   - Adjust value of the UART_BYTE_TO_BYTE_TIMEOUT_US constant to align with UART
*     speed in the transport_uart.c file.
*   - Adjust UART interrupt priority in the UART_INTR_PRIORITY in the
*     transport_uart.c file.
* - Build and program a project into the device.
* - Open the DFU Host Tool. Select the UART interface. Set the UART baud rate
*   according to the SCB UART setup in the previous step.
* - Select the *.cyacd2 application image and upload to the device.
*
********************************************************************************
* \subsection group_dfu_ucase_spi Firmware Update via SPI
********************************************************************************
*
* See \ref section_dfu_quick_start for basic steps how to set up a DFU project.
* The steps for the SPI transport support:
* - Add SPI transport component in project's Makefile:
*    locate **COMPONENTS** variable and add **DFU_SPI**:
*    \code COMPONENTS+=DFU_SPI \endcode
* - For templates based on the PDL drivers:
*   - Select and configure the SCB block using the ModusToolbox&trade; Device
*     Configurator see \ref group_dfu_mtb_cfg or manually using
*     the configuration structures.
*   - Adjust value of the SPI_BYTE_TO_BYTE constant to align with SPI speed
*     in the transport_spi.c file.
*   - Check the value of the CY_SPI_SLAVE_SELECT in the transport_spi.c file.
*   - Adjust SPI interrupt priority in the SPI_INTR_PRIORITY in the
*     transport_spi.c file.
* - Build and program a project into the device.
* - Open the DFU Host Tool. Select the SPI interface. Set SPI mode, shift the
*   direction and speed according to the SCB SPI setup in the previous step.
* - Select the *.cyacd2 application image and upload to the device.
*
********************************************************************************
* \subsection group_dfu_ucase_usb Firmware Update via USB CDC transport
********************************************************************************
*
* See \ref section_dfu_quick_start for basic steps how to setup a DFU project.
* Specific steps for the USB transport support:
* - Add USB_CDC transport component in project's Makefile:
*    locate **COMPONENTS** variable and add **DFU_USB_CDC**:
*    \code COMPONENTS+=DFU_USB_CDC \endcode
* - Enable and configure the USB Device block using the ModusToolbox&trade; Device Configurator
*   see \ref group_dfu_mtb_cfg or manually using the configuration structures.
* - Generate USB descriptors and USB Middleware structures using the USB Configurator.
*   Open the USB configuration file (cycfg_usb_cdc.cyusbdev)
*   in the DFU \\export\\config\\COMPONENT_CAT1\\COMPONENT_DFU_USB_CDC folder,
*   then click Save to generate configuration files (cycfg_usbdev.c and cycfg_usbdev.h).
*   These files must be included into the build flow
*   (see USB Middleware API Reference \ref group_dfu_more_info).
* - Build and program a project into the device. Connect your Host to the USB
*   device.
* - For the USB CDC class: open the DFU Host Tool. Select the UART interface,
*   because the Host recognizes the USB device as a virtual UART
*   (the name is "DFU USB CDC transport").
*   UART settings: baud rate - 115200, data bits - 8, stop bits - 1, parity - None.
* - Select the *.cyacd2 application image and upload to the device.
*
********************************************************************************
* \subsection group_dfu_ucase_emusb Firmware Update via emUSB CDC transport
********************************************************************************
*
* Specific steps for the emUSB transport support:
* - Add emUSB_CDC transport components to the project's Makefile:
*    \code COMPONENTS+=USBD_BASE \endcode
*    \code COMPONENTS+=DFU_EMUSB_CDC \endcode
*    \code COMPONENTS+=SOFTFP \endcode
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
*    \code DEFINES+=DFU_CANFD_IRQ_PRIORITY=5 \endcode
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
* For an application image, DFU supports 2 types of checksums: CRC-32 and SHA1.
* SHA1 is calculated with a crypto hardware block, which is available only on CAT1A devices.
* The default application checksum is CRC-32.
* The steps to set the SHA1 checksum for an application image:
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
* The DFU design does not limit the number of applications but it is limited
* by memory size and metadata size. The maximum size
* of DFU metadata is limited to the size of the flash row, because metadata
* should be in a single flash row. For example, the 512-byte metadata supports
* up to 63 applications.
* An arbitrary number of applications can be protected from overwriting. Such
* a protected application is called "Golden Image".
* See \ref section_dfu_quick_start for a steps to setup basic 2 application DFU
* projects. The following steps show how to set up a 3rd application.
* The same approach can be used to setup 4th - Nth applications.
* - Define the sizes for each of the three applications and define the start and
*   size of each memory region (flash, RAM) for each application.
* - Copy the linker script dfu_cm4_app1 from DFU linker_scripts folder
*   according to the selected compiler and rename it (for example dfu_cm4_app2).
* - Add flash and RAM sections to the 3rd application. Name them flash_app2,
*   ram_app2.
* - Update the size and start address for each section in each linker script
*   based on the defined in the first step allocation.
* - Set __cy_app_id symbol to 2
* - Update the region aliases for flash and RAM to use flash_app2 and ram_app2
*   accordingly:
*   \code
*   REGION_ALIAS("flash", flash_app2);
*   REGION_ALIAS("ram",     ram_app2);
*   \endcode
* - Add symbols __cy_app2_verify_start and __cy_app2_verify_length for metadata
*   initialization in the same way as for application 0 and 1.
* - Add a macro to the dfu_user.h CY_DFU_APP2_VERIFY_START and
*   CY_DFU_APP2_VERIFY_LENGTH in the same way as for application 0 and 1
* - Add to the cy_dfu_metadata array of the dfu_user.c CY_DFU_APP2_VERIFY_START
*   and CY_DFU_APP2_VERIFY_LENGTH to update the metadata with the 3rd application.
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
* CyMCUElfTool and used by host programs such as Cypress DFU Host Program and
* CySmart to send applications to the target DFU module
* (see \ref group_dfu_more_info). Refer to the
* [AN213924](https://www.infineon.com/an213924) DFU SDK User Guide for the .cyacd2
* file format. See the \ref loadable_DFU_app_steps "Loadable Application Setup"
* section of the \ref section_dfu_quick_start for the steps to convert
* a general application into a DFU loadable application.
*
* The steps to create a .cyacd2 file with a CRC application signature:
* -# Copy the path to the CyMCUElfTool binary. The path can be found in the
*    folder with ModusToolbox&trade; tools (for example
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
* \section group_dfu_changelog Changelog
********************************************************************************
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td rowspan="3">5.2</td>
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
*     <td rowspan="3">5.1</td>
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
*     <td rowspan="5">5.0</td>
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
*     <td>Added PSoC 4 devices support.
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
#define CY_DFU_SDK_MW_VERSION_MAJOR       (5)

/** The DFU SDK minor version */
#define CY_DFU_SDK_MW_VERSION_MINOR       (2)

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
    CY_DFU_USB_CDC = 0x04U, /**< USB CDC transport interface */
    CY_DFU_USB_HID = 0x05U, /**< USB HID transport interface */
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

#if CY_DFU_OPT_CUSTOM_CMD != 0
    Cy_DFU_CustomCommandHandler handlerCmd; /**< User handler for the custom commands.*/
#endif /* CY_DFU_OPT_CUSTOM_CMD != 0 */

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

/** \} group_dfu_globals */

/**
* \addtogroup group_dfu_functions
* \{
*/

cy_en_dfu_status_t Cy_DFU_Init(uint32_t *state, cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_Continue(uint32_t *state, cy_stc_dfu_params_t *params);

uint32_t Cy_DFU_DataChecksum(const uint8_t *address, uint32_t length, cy_stc_dfu_params_t *params);

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
*   DFU Functions for application management.
*/
void Cy_DFU_ExecuteApp(uint32_t appId);
void Cy_DFU_OnResetApp0(void);
uint32_t Cy_DFU_GetRunningApp(void);
cy_en_dfu_status_t Cy_DFU_SwitchToApp(uint32_t appId);
cy_en_dfu_status_t Cy_DFU_CopyApp(uint32_t destAddress, uint32_t srcAddress, uint32_t length,
                                            uint32_t rowSize, cy_stc_dfu_params_t *params);
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */
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
*      \snippet snippet/main.c snippet_cy_dfu_UserCommands
*
*   2. Define the function to handle the custom commands.
*   \note A single function is used as the handler for all custom commands.
*
*      \snippet snippet/main.c snippet_cy_dfu_UserCommandHandlerDeclaration
*
*      \snippet snippet/main.c snippet_cy_dfu_UserCommandHandlerDefinition
*
*   3. Register the function to handle custom commands as a callback in the DFU core before use.
*      \snippet snippet/main.c snippet_cy_dfu_UserCommandHandlerRegister
*
*   4. Release the callback function when custom command handling is no longer required.
*      \snippet snippet/main.c snippet_cy_dfu_UserCommandHandlerUnregister
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

#if !defined(CY_PSOC_CREATOR_USED)

    /* Should be 0 in a non-Creator flow */
    #define CY_DFU_SILICON_ID      (0U)
    #define CY_DFU_SILICON_REV     (0U)
#else
    #include "cy_device_headers.h" /* For CY_SILICON_ID            */
    #include <cyfitter.h>          /* For CYDEV_CHIP_REVISION_USED */

    #define CY_DFU_SILICON_ID  CY_SILICON_ID
    #define CY_DFU_SILICON_REV CYDEV_CHIP_REVISION_USED
#endif /* defined CY_DOXYGEN */


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
