/*******************************************************************************
* \file dfu_common.h
* \version 4.10
* 
* This file provides project configuration macro definitions. They are used 
* in the scatter files and source code files.
* 
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
    
#ifndef DFU_MDK_COMMON_H_
#define DFU_MDK_COMMON_H_

/* Expand expression to the string */
#define CY_STR_EXPAND(foo)  #foo
#define CY_STR(foo)         CY_STR_EXPAND(foo)


/* DFU SDK parameters */
/* The user application may either update them or leave the defaults if they fit */
#define CY_BOOT_METADATA_ADDR       0x100FFA00
#define CY_BOOT_METADATA_LENGTH     0x00000200
#define CY_PRODUCT_ID               0x01020304
#define CY_CHECKSUM_TYPE            0

/*
* The size of the section .cy_app_signature.
* 1, 2 or 4 for a checksum
* CRC-32 size: 4 bytes
* SHA1 size:   20 byte
* SHA256 size: 32 byte
* RSASSA-PKCS1-v1.5 with the 2048 bit RSA key: 256 bytes
*
* SHA1 must be used.
*/
#define CY_BOOT_SIGNATURE_SIZE      4

/* For the MDK linker script, defines TOC parameters */
/* Update per device series to be in the last flash row */
#define CY_TOC_START                0x16007C00
#define CY_TOC_SIZE                 0x00000400

/* Application ranges in RAM */
#define CY_APP_RAM_COMMON_ADDR      0x08002000
#define CY_APP_RAM_COMMON_LENGTH    0x00000400

/* Note: the ram_appX regions has to be 0x400 aligned as they contain 
 * Interrupt Vector Table Remapped at the start.
 */
#define CY_APP0_RAM_ADDR            (CY_APP_RAM_COMMON_ADDR + CY_APP_RAM_COMMON_LENGTH)
#define CY_APP0_RAM_LENGTH          0x00008000

#define CY_APP1_RAM_ADDR            (CY_APP_RAM_COMMON_ADDR + CY_APP_RAM_COMMON_LENGTH)
#define CY_APP1_RAM_LENGTH          0x00008000

/* Memory region ranges per applications */
#define CY_CM0P_FLASH_ADDR          0x10000000
#define CY_CMP0_FLASH_LENGTH        0x00002000

#define CY_APP0_FLASH_ADDR          (CY_CM0P_FLASH_ADDR + CY_CMP0_FLASH_LENGTH)
#define CY_APP0_FLASH_LENGTH        0x00010000

#define CY_APP1_FLASH_ADDR          0x10050000
#define CY_APP1_FLASH_LENGTH        0x00010000

/* DFU SDK metadata address range in flash */
#define CY_BOOT_META_FLASH_ADDR     0x100FFA00
#define CY_BOOT_META_FLASH_LENGTH   0x00000200

/* Application ranges in emulated EEPROM */
#define CY_APP0_EM_EEPROM_ADDR      0x14000000
#define CY_APP0_EM_EEPROM_LENGTH    0x00000000

#define CY_APP1_EM_EEPROM_ADDR      0x14000000
#define CY_APP1_EM_EEPROM_LENGTH    0x00000000

/* Application ranges in SMIF XIP */
#define CY_APP0_SMIF_ADDR           0x18000000
#define CY_APP0_SMIF_LENGTH         0x00000000

#define CY_APP1_SMIF_ADDR           0x18000000
#define CY_APP1_SMIF_LENGTH         0x00000000

#endif /* DFU_MDK_COMMON_H_ */


/* [] END OF FILE */
