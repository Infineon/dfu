/***************************************************************************//**
* \file dfu_user.h
* \version 5.1
*
* This file provides declarations that can be modified by the user but
* are used by the DFU SDK.
*
********************************************************************************
* \copyright
* (c) (2016-2023), Cypress Semiconductor Corporation (an Infineon company) or
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

#if !defined(DFU_USER_H)
#define DFU_USER_H

#include <stdint.h> //Check if could be removed

#include "cy_flash.h"


#if defined(__cplusplus)
extern "C" {
#endif


/**
* \addtogroup group_dfu_macro_config
* \{
*/

#ifndef CY_DFU_LOG_LEVEL
    #define CY_DFU_LOG_LEVEL CY_DFU_LOG_LEVEL_OFF /**< Defines log level */
#endif

#define CY_DFU_BASIC_FLOW               (0U) /**< Basic Bootloader flow includes
                                              * application transfer and bootload. */
#define CY_DFU_MCUBOOT_FLOW             (1U) /**< MCUBoot compatibility flow includes
                                              * only application transport. */

/**
 * Configuration option to select DFU flow. Possible options are:
 * - \ref CY_DFU_BASIC_FLOW - supported for the CAT1A and CAT2 devices
 * - \ref CY_DFU_MCUBOOT_FLOW - supported for the CAT1A and CAT1C devices */
#ifndef CY_DFU_FLOW
    #if defined COMPONENT_CAT1A || defined COMPONENT_CAT2
        #define CY_DFU_FLOW                     (CY_DFU_BASIC_FLOW)
    #else
        #define CY_DFU_FLOW                     (CY_DFU_MCUBOOT_FLOW)
    #endif
#endif /* CY_DFU_FLOW */

/** Define program row size of the Non-Volatile memory */
#ifndef CY_NVM_SIZEOF_ROW
    #ifdef  CY_FLASH_SIZEOF_ROW
        #define CY_NVM_SIZEOF_ROW       CY_FLASH_SIZEOF_ROW
    #else
        #define CY_NVM_SIZEOF_ROW       (512)
    #endif /* CY_FLASH_SIZEOF_ROW */
#endif /* CY_NVM_SIZEOF_ROW */

/** The size of a buffer to hold DFU commands */
/* 16 bytes is a maximum overhead of a DFU packet and additional data for the Program Data command */
#define CY_DFU_SIZEOF_CMD_BUFFER  (CY_NVM_SIZEOF_ROW + 16U)

/** The size of a buffer to hold an NVM row of data to write or verify */
#define CY_DFU_SIZEOF_DATA_BUFFER (CY_NVM_SIZEOF_ROW + 16U)

/** A non-zero value enables the Verify Data DFU command  */
#ifndef CY_DFU_OPT_VERIFY_DATA
    #define CY_DFU_OPT_VERIFY_DATA     (1)
#endif /* CY_DFU_OPT_VERIFY_DATA */

/** A non-zero value enables the Erase Data DFU command   */
#ifndef CY_DFU_OPT_ERASE_DATA
    #define CY_DFU_OPT_ERASE_DATA      (1)
#endif /* CY_DFU_OPT_ERASE_DATA */

/** A non-zero value enables the Verify App DFU command   */
#ifndef CY_DFU_OPT_VERIFY_APP
    #define CY_DFU_OPT_VERIFY_APP      (1)
#endif /* CY_DFU_OPT_VERIFY_APP */

/**
* A non-zero value enables the Send Data DFU command.
* If the "Send Data" DFU command is enabled, \c packetBuffer and \c dataBuffer
* must be non-overlapping.
*
* Else, \c dataBuffer must be inside \c packetBuffer with an offset of
* \c CY_DFU_PACKET_DATA_IDX, typically 4 bytes. \n
* <code>params->dataBuffer = &packetBuffer[4];</code> \n
* \note that \c packetBuffer in this case must be 4 bytes aligned, as
* \c dataBuffer is required to be 4 bytes aligned.
*/

#ifndef CY_DFU_OPT_SEND_DATA
    #define CY_DFU_OPT_SEND_DATA       (1)
#endif /* CY_DFU_OPT_SEND_DATA */

/** A non-zero value enables the usage of CRC-16 for DFU packet verification */
#ifndef CY_DFU_OPT_PACKET_CRC
    #define CY_DFU_OPT_PACKET_CRC      (0)
#endif /* CY_DFU_OPT_PACKET_CRC */

/** A non-zero value enables the user custom command support */
#ifndef CY_DFU_OPT_CUSTOM_CMD
    #define CY_DFU_OPT_CUSTOM_CMD      (0)
#endif /* CY_DFU_OPT_CUSTOM_CMD */

/**
* The number of applications in the metadata,
* for 512 bytes in a flash row - 63 is the maximum possible value,
* because 4 bytes are reserved for the entire metadata CRC.
*
* The smallest metadata size if CY_DFU_MAX_APPS * 8 (bytes per one app) + 4 (bytes for CRC-32C)
*/
#ifndef CY_DFU_MAX_APPS
    #define CY_DFU_MAX_APPS            (2U)
#endif /* CY_DFU_MAX_APPS */

/* MCUBoot compatibility flow specific constants */
#if (CY_DFU_FLOW == CY_DFU_MCUBOOT_FLOW) && !defined(CY_DOXYGEN)
    #if !defined CY_DFU_PRODUCT
        #define CY_DFU_PRODUCT          (0x01020304)  /**< Default value for Product ID*/
        #warning CY_DFU_PRODUCT is set to default value
    #endif /* !defined CY_DFU_PRODUCT */
#endif /* CY_DFU_FLOW == CY_DFU_MCUBOOT_FLOW */

/* Basic bootloader flow specific constants */
#if (CY_DFU_FLOW == CY_DFU_BASIC_FLOW) || defined(CY_DOXYGEN)

    /**
    * Set to non-zero for the DFU SDK Program Data command to check
    * if the Golden image is going to be overwritten while updating.
    */
    #ifndef CY_DFU_OPT_GOLDEN_IMAGE
        #define CY_DFU_OPT_GOLDEN_IMAGE    (0)
    #endif /* CY_DFU_OPT_GOLDEN_IMAGE */

    /**
    * List of Golden Image Application IDs.
    * Here "Golden Image Application" means an application that cannot be changed with
    * CommandProgramData()
    *
    * Usage. Define the list of Golden Image Application IDs without enclosing
    * parenthesis, e.g.
    * \code #define CY_DFU_GOLDEN_IMAGE_IDS()     0U, 1U, 3U \endcode
    * later it is used in cy_dfu.c file:
    * \code uint8_t goldenImages[] = { CY_DFU_GOLDEN_IMAGE_IDS() }; \endcode
    */
    #ifndef CY_DFU_GOLDEN_IMAGE_IDS
        #define CY_DFU_GOLDEN_IMAGE_IDS()  0U
    #endif /* CY_DFU_GOLDEN_IMAGE_IDS */

    /** A non-zero value enables the Get Metadata DFU command */
    #ifndef CY_DFU_OPT_GET_METADATA
        #define CY_DFU_OPT_GET_METADATA    (1)
    #endif /* CY_DFU_OPT_GET_METADATA */

    /** A non-zero value enables the Set EI Vector DFU command */
    #ifndef CY_DFU_OPT_SET_EIVECTOR
        #define CY_DFU_OPT_SET_EIVECTOR    (0)
    #endif /* CY_DFU_OPT_SET_EIVECTOR */

    /** A non-zero value allows writing metadata with the Set App Metadata DFU command. */
    #ifndef CY_DFU_METADATA_WRITABLE
        #define CY_DFU_METADATA_WRITABLE   (1)
    #endif /* CY_DFU_METADATA_WRITABLE */

    /** Non-zero value enables the usage of hardware Crypto API */
    #ifndef CY_DFU_OPT_CRYPTO_HW
        #define CY_DFU_OPT_CRYPTO_HW       (0)
    #endif /* CY_DFU_OPT_CRYPTO_HW */

    #if defined(__ARMCC_VERSION)
        #include "dfu_common.h"

        #define CY_DFU_SIGNATURE_SIZE          CY_BOOT_SIGNATURE_SIZE
        #define CY_DFU_APP0_VERIFY_START       ( CY_APP0_FLASH_ADDR )
        #define CY_DFU_APP0_VERIFY_LENGTH      ( CY_APP0_FLASH_LENGTH - CY_DFU_SIGNATURE_SIZE)
        #define CY_DFU_APP1_VERIFY_START       ( CY_APP1_FLASH_ADDR )
        #define CY_DFU_APP1_VERIFY_LENGTH      ( CY_APP1_FLASH_LENGTH - CY_DFU_SIGNATURE_SIZE)

    #elif defined(__GNUC__) || defined(__ICCARM__)
        /*
        * These variables are defined in the linker scripts, the values of their addresses define
        * corresponding applications start address and length.
        */
        extern uint8_t __cy_app0_verify_start;
        extern uint8_t __cy_app0_verify_length;
        extern uint8_t __cy_app1_verify_start;
        extern uint8_t __cy_app1_verify_length;
        extern uint8_t __cy_boot_signature_size;

        #define CY_DFU_APP0_VERIFY_START       ( (uint32_t)&__cy_app0_verify_start )
        #define CY_DFU_APP0_VERIFY_LENGTH      ( (uint32_t)&__cy_app0_verify_length )
        #define CY_DFU_APP1_VERIFY_START       ( (uint32_t)&__cy_app1_verify_start )
        #define CY_DFU_APP1_VERIFY_LENGTH      ( (uint32_t)&__cy_app1_verify_length )
        #define CY_DFU_SIGNATURE_SIZE          ( (uint32_t)&__cy_boot_signature_size )
    #else
        #error "Not implemented for this compiler"
    #endif /* defined(__CC_ARM) */
#endif /* (CY_DFU_FLOW == CY_DFU_BASIC_FLOW) && !defined(CY_DOXYGEN) */

/** \} group_dfu_macro_config */

#if defined(__cplusplus)
}
#endif

#endif /* !defined(DFU_USER_H) */


/* [] END OF FILE */
