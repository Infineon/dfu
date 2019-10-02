/*******************************************************************************
* \file dfu_elf_symbols.c
* \version 4.0
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
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
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

    /* Emulated EEPROM flash area */
    ".global __cy_memory_1_start    \n"
    ".global __cy_memory_1_length   \n"
    ".global __cy_memory_1_row_size \n"

    /* Supervisory flash */
    ".global __cy_memory_2_start    \n"
    ".global __cy_memory_2_length   \n"
    ".global __cy_memory_2_row_size \n"

    /* XIP */
    ".global __cy_memory_3_start    \n"
    ".global __cy_memory_3_length   \n"
    ".global __cy_memory_3_row_size \n"

    /* eFuse */
    ".global __cy_memory_4_start    \n"
    ".global __cy_memory_4_length   \n"
    ".global __cy_memory_4_row_size \n"

    /* flash */
    ".equ __cy_memory_0_start,    0x10000000 \n"
    ".equ __cy_memory_0_length,   0x00100000 \n"
    ".equ __cy_memory_0_row_size, 0x200 \n"

    /* Emulated EEPROM flash area */
    ".equ __cy_memory_1_start,    0x14000000 \n"
    ".equ __cy_memory_1_length,   0x8000 \n"
    ".equ __cy_memory_1_row_size, 0x200 \n"

    /* Supervisory flash */
    ".equ __cy_memory_2_start,    0x16000000 \n"
    ".equ __cy_memory_2_length,   0x8000 \n"
    ".equ __cy_memory_2_row_size, 0x200 \n"
    
    /* XIP */
    ".equ __cy_memory_3_start,    0x18000000 \n"
    ".equ __cy_memory_3_length,   0x08000000 \n"
    ".equ __cy_memory_3_row_size, 0x200 \n"
    
    /* eFuse */
    ".equ __cy_memory_4_start,    0x90700000 \n"
    ".equ __cy_memory_4_length,   0x100000 \n"
    ".equ __cy_memory_4_row_size, 1 \n"
);


/* [] END OF FILE */
