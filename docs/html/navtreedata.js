/*
@ @licstart  The following is the entire license notice for the
JavaScript code in this file.

Copyright (C) 1997-2017 by Dimitri van Heesch

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

@licend  The above is the entire license notice
for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Device Firmware Update (DFU) Middleware Library 5.1", "index.html", [
    [ "Overview", "index.html#section_mainpage_overview", null ],
    [ "General Description", "index.html#section_dfu_general", null ],
    [ "Quick Start Guide", "index.html#section_dfu_quick_start", [
      [ "Basic Bootloader Flow", "index.html#subsection_dfu_qsg_basic", [
        [ "STEP 0: Projects preparation", "index.html#ssection_dfu_step_0", null ],
        [ "STEP 1: Setup Loader Application QSG_DFU_App0_I2C", "index.html#ssection_dfu_step_1", null ],
        [ "STEP 2: Update Loader QSG_DFU_App0_I2C main.c", "index.html#ssection_dfu_step_2", null ],
        [ "STEP 3: Build and Program Loader QSG_DFU_App0_I2C", "index.html#ssection_dfu_step_3", null ],
        [ "STEP 4: Setup Loadable QSG_DFU_App1_Hello_World", "index.html#ssection_dfu_step_4", null ],
        [ "STEP 5: Update Loadable QSG_DFU_App1_Hello_World main.c", "index.html#ssection_dfu_step_5", null ],
        [ "STEP 6: Build and Program Patch", "index.html#ssection_dfu_step_6", null ]
      ] ],
      [ "DFU Transport (MCUBoot compatible) flow", "index.html#subsection_dfu_qsg_mcuboot", [
        [ "Description", "index.html#subsubsection_qsg_mcuboot_description", null ],
        [ "STEP1: Projects preparation.", "index.html#subsubsection_qsg_mcuboot_s1", null ],
        [ "STEP2: Add DFU logic to main.c", "index.html#subsubsection_qsg_mcuboot_s2", null ],
        [ "STEP3: Build and Program Loader DFU_App0", "index.html#subsubsection_qsg_mcuboot_s3", null ],
        [ "STEP4: Create a loadable application (Application 1).", "index.html#subsubsection_qsg_mcuboot_s4", null ]
      ] ]
    ] ],
    [ "Configuration Considerations", "index.html#section_dfu_configuration", [
      [ "Linker scripts", "index.html#group_dfu_config_linker_scripts", null ],
      [ "Use of the ModusToolbox(TM) tools for HW initialization", "index.html#group_dfu_mtb_cfg", null ]
    ] ],
    [ "Design Considerations", "index.html#section_dfu_design", [
      [ "Firmware Update via I2C", "index.html#group_dfu_ucase_i2c", null ],
      [ "Firmware Update via UART", "index.html#group_dfu_ucase_uart", null ],
      [ "Firmware Update via SPI", "index.html#group_dfu_ucase_spi", null ],
      [ "Firmware Update via USB CDC transport", "index.html#group_dfu_ucase_usb", null ],
      [ "Firmware Update via emUSB CDC transport", "index.html#group_dfu_ucase_emusb", null ],
      [ "Change checksum types", "index.html#group_dfu_ucase_checksum", null ],
      [ "Multi-application DFU project", "index.html#group_dfu_ucase_multiapp", null ],
      [ "Creation of the CYACD2 file", "index.html#group_dfu_ucase_cyacd2", null ]
    ] ],
    [ "Changelog", "index.html#group_dfu_changelog", null ],
    [ "More Information", "index.html#group_dfu_more_info", null ],
    [ "MISRA-C:2012 Compliance", "group_dfu_MISRA.html", null ],
    [ "API Reference", "modules.html", "modules" ]
  ] ]
];

var NAVTREEINDEX =
[
"group__group__dfu__data__structs.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';