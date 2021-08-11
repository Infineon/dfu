/*******************************************************************************
* \file dfu_elf_symbols.c
* \version 4.20
*
* This file provides inline assembly to add symbols in the an ELF file required
* by CyMCUElfTool to generate correct CYACD2 image.
*
* \note
* This file requires modifications of DFU specific symbols for the
* application #1.
*
********************************************************************************
* \copyright
* (c) (2016-2021), Cypress Semiconductor Corporation (an Infineon company) or
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

#include "dfu_common.h"


/* Symbols that are added to the ELF file */
__asm
(
    /* DFU specific symbols */
    ".global __cy_app_id \n"
    ".global __cy_product_id \n"
    ".global __cy_boot_metadata_addr \n"
    ".global __cy_boot_metadata_length \n"

    ".global  __cy_app_core1_start_addr \n"
    ".global  __cy_checksum_type \n"
    ".global  __cy_app_verify_start \n"
    ".global  __cy_app_verify_length \n"

    ".equ __cy_boot_metadata_addr,   " CY_STR(CY_BOOT_METADATA_ADDR) "\n"
    ".equ __cy_boot_metadata_length, " CY_STR(CY_BOOT_METADATA_LENGTH) "\n"

    ".equ __cy_app_id,               0 \n"
    ".equ __cy_product_id,           " CY_STR(CY_PRODUCT_ID) "\n"
    ".equ __cy_checksum_type,        " CY_STR(CY_CHECKSUM_TYPE) "\n"
    ".equ __cy_app_core1_start_addr, " CY_STR(CY_APP0_FLASH_ADDR) " \n"
    ".equ __cy_app_verify_start,     " CY_STR(CY_APP0_FLASH_ADDR) " \n"
    ".equ __cy_app_verify_length,    " CY_STR(CY_APP0_FLASH_LENGTH) " - " CY_STR(CY_BOOT_SIGNATURE_SIZE) "\n"

    /* flash */
    ".global __cy_memory_0_start    \n"
    ".global __cy_memory_0_length   \n"
    ".global __cy_memory_0_row_size \n"

    /* flash */
    ".equ __cy_memory_0_start,    0x00000000 \n"
    ".equ __cy_memory_0_length,   0x00020000 \n"
    ".equ __cy_memory_0_row_size, 0x100 \n"
);


/* [] END OF FILE */
