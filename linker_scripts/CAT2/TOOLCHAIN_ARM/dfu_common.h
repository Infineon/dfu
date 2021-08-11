/*******************************************************************************
* \file dfu_common.h
* \version 4.20
*
* This file provides project configuration macro definitions. They are used
* in the scatter files and source code files.
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

#ifndef DFU_MDK_COMMON_H_
#define DFU_MDK_COMMON_H_

/* Expand expression to the string */
#define CY_STR_EXPAND(foo)  #foo
#define CY_STR(foo)         CY_STR_EXPAND(foo)


/* DFU SDK parameters */
/* The user application may either update them or leave the defaults if they fit */
#define CY_BOOT_METADATA_ADDR       0x0001FA00
#define CY_BOOT_METADATA_LENGTH     0x00000100
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
//#define CY_TOC_START                0x16007C00
//#define CY_TOC_SIZE                 0x00000400

/* Application ranges in RAM */
#define CY_APP_RAM_COMMON_ADDR      0x20000000
#define CY_APP_RAM_COMMON_LENGTH    0x00000400

/* Note: the ram_appX regions has to be 0x400 aligned as they contain
 * Interrupt Vector Table Remapped at the start.
 */
#define CY_APP0_RAM_ADDR            (CY_APP_RAM_COMMON_ADDR + CY_APP_RAM_COMMON_LENGTH)
#define CY_APP0_RAM_LENGTH          0x00003000

#define CY_APP1_RAM_ADDR            (CY_APP_RAM_COMMON_ADDR + CY_APP_RAM_COMMON_LENGTH)
#define CY_APP1_RAM_LENGTH          0x00003000

/* Memory region ranges per applications */

#define CY_APP0_FLASH_ADDR          0X00000000
#define CY_APP0_FLASH_LENGTH        0x00009000

#define CY_APP1_FLASH_ADDR          0x00010000
#define CY_APP1_FLASH_LENGTH        0x00008000

/* DFU SDK metadata address range in flash */
#define CY_BOOT_META_FLASH_ADDR     0x0001FA00
#define CY_BOOT_META_FLASH_LENGTH   0x00000100

#endif /* DFU_MDK_COMMON_H_ */


/* [] END OF FILE */
