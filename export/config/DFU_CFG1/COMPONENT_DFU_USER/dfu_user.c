/***************************************************************************//**
* \file dfu_user.c
*
* This file provides the custom API for a firmware application with
* DFU SDK.
* - Cy_DFU_ReadData (address, length, ctl, params) - to read  the NVM block
* - Cy_Bootalod_WriteData(address, length, ctl, params) - to write the NVM block
*
********************************************************************************
* \copyright
* (c) (2016-2026), Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
********************************************************************************
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#include <string.h>
#include "cy_syslib.h"
#include "cy_dfu.h"
#include "cy_dfu_logging.h"
#include "mtb_hal_system.h"

#if (CY_DFU_OPT_EXTERNAL_MEMORY == 0U)
#include "mtb_hal_nvm.h"
#endif /* #if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U) */

#ifdef COMPONENT_DFU_I2C
    #include "transport_i2c.h"
#endif /* COMPONENT_DFU_I2C */

#ifdef COMPONENT_DFU_UART
    #include "transport_uart.h"
#endif /* COMPONENT_DFU_UART */

#ifdef COMPONENT_DFU_SPI
    #include "transport_spi.h"
#endif  /* COMPONENT_DFU_SPI*/

#ifdef COMPONENT_DFU_USB_CDC
    #include "transport_usb_cdc.h"
#endif  /* COMPONENT_DFU_USB_CDC */

#ifdef COMPONENT_DFU_EMUSB_CDC
    #include "transport_emusb_cdc.h"
#endif  /* COMPONENT_DFU_EMUSB_CDC */

#ifdef COMPONENT_DFU_EMUSB_HID
    #include "transport_emusb_hid.h"
#endif  /* COMPONENT_DFU_EMUSB_HID */

#ifdef COMPONENT_DFU_CANFD
    #include "transport_canfd.h"
#endif  /* COMPONENT_DFU_CANFD */

#ifdef COMPONENT_DFU_PMBUS
    #include "transport_pmbus.h"
#endif  /* COMPONENT_DFU_PMBUS */

#ifdef COMPONENT_DFU_HOST_I2C
    #include "host_transport_i2c.h"
#endif /* COMPONENT_DFU_HOST_I2C */

#ifdef COMPONENT_DFU_HOST_UART
    #include "host_transport_uart.h"
#endif /* COMPONENT_DFU_HOST_UART */

#if !defined(COMPONENT_DFU_I2C) && !defined(COMPONENT_DFU_UART) && !defined(COMPONENT_DFU_SPI) &&\
    !defined(COMPONENT_DFU_USB_CDC) && !defined(COMPONENT_DFU_EMUSB_CDC) && !defined(COMPONENT_DFU_EMUSB_HID) &&\
    !defined(COMPONENT_DFU_CANFD) && !defined(COMPONENT_DFU_PMBUS)
    #warning "Select at least one of the DFU transports."
#endif /* !defined(COMPONENT_DFU_I2C) ... !defined(COMPONENT_DFU_PMBUS) */

#if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U)
    #ifndef CY_EXT_NVM0_BASE
        #define CY_EXT_NVM0_BASE         CY_XIP_PORT0_BASE
        #define CY_EXT_NVM0_SIZE         CY_XIP_PORT0_SIZE
    #endif /* CY_EXT_NVM0_BASE */
    #ifndef CY_EXT_NVM1_BASE
        #define CY_EXT_NVM1_BASE         CY_XIP_PORT1_BASE
        #define CY_EXT_NVM1_SIZE         CY_XIP_PORT1_SIZE
    #endif /* CY_EXT_NVM1_BASE */
#endif /* CY_DFU_OPT_EXTERNAL_MEMORY != 0U */

/* Global NVM object */
#if (CY_DFU_OPT_EXTERNAL_MEMORY == 0U)
    static mtb_hal_nvm_t nvm_obj;
#endif /* (CY_DFU_OPT_EXTERNAL_MEMORY == 0U) */

#if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U)
static mtb_serial_memory_t * serialMemObjPtr = NULL;

void Cy_DFU_AddExtMemory(mtb_serial_memory_t *serialMemObj)
{
    serialMemObjPtr = serialMemObj;
}
#endif /* #if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U) */

static cy_en_dfu_transport_t selectedInterface = CY_DFU_UART;
#if (CY_DFU_OPT_HOST_MODE != 0U) && (defined(COMPONENT_DFU_HOST_I2C) || defined(COMPONENT_DFU_HOST_UART))
static cy_en_dfu_transport_t selectedHostInterface = CY_DFU_NONE;
#endif /* CY_DFU_OPT_HOST_MODE && COMPONENT_DFU_HOST_I2C */

#ifdef CY_IP_M7CPUSS
    static const mtb_hal_nvm_region_info_t* blocks_info;
    static uint8_t blocks_count;
    static uint32_t blocks_sector_size;
#endif

#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
    /*
    * The DFU SDK metadata initial value is placed here
    * Note: the number of elements equal to the number of the app multiplies by 2
    *       because of the two fields per app plus one element for the CRC-32C field.
    */
    CY_SECTION(".cy_boot_metadata") __USED
    static const uint32_t cy_dfu_metadata[CY_FLASH_SIZEOF_ROW / sizeof(uint32_t)] =
    {
        CY_DFU_APP0_VERIFY_START, CY_DFU_APP0_VERIFY_LENGTH, /* The App0 base address and length */
        CY_DFU_APP1_VERIFY_START, CY_DFU_APP1_VERIFY_LENGTH, /* The App1 base address and length */
        0U                                                             /* The rest does not matter     */
    };
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/


static bool IsMultipleOf(uint32_t value, uint32_t multiple);
static bool AddressValid(uint32_t address, cy_stc_dfu_params_t *params);


#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
    static void GetStartEndAddress(uint32_t appId, uint32_t *startAddress, uint32_t *endAddress);
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */

#if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U)
    static cy_en_dfu_status_t Ext_Flash_WriteRow(uint32_t address, size_t length, cy_stc_dfu_params_t *params);
    static cy_en_dfu_status_t Ext_Flash_ReadRow(uint32_t address, size_t length, uint8_t *data);
#endif /* (CY_DFU_OPT_EXTERNAL_MEMORY != 0U) */


/*******************************************************************************
* Function Name: IsMultipleOf
****************************************************************************//**
*
* This internal function check if value parameter is a multiple of parameter
* multiple
*
* \param value      value that will be checked
* \param multiple   value with which value is checked
*
* \return True - value is multiple of parameter multiple, else False
*
*******************************************************************************/
static bool IsMultipleOf(uint32_t value, uint32_t multiple)
{
    return ((value % multiple) == 0U);
}


/*******************************************************************************
* Function Name: AddressValid
****************************************************************************//**
*
* Internal function to validate address
*
* \param address    The address to check.
* \param params     The pointer to a DFU parameters structure, see \ref cy_stc_dfu_params_t.
*
* \return True - address valid
*
*******************************************************************************/
static bool AddressValid(uint32_t address, cy_stc_dfu_params_t *params)
{
    bool addrValid = true;

#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
    addrValid = ((((CY_FLASH_BASE + CY_DFU_APP0_VERIFY_LENGTH)) <= address) &&
                                (address < (CY_FLASH_BASE + CY_FLASH_SIZE))) ||
                ((CY_EM_EEPROM_BASE <= address) &&
                        (address < (CY_EM_EEPROM_BASE + CY_EM_EEPROM_SIZE)));
    CY_UNUSED_PARAMETER(params);
#else /* MCUBoot flow*/
    #if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U) /* External memory */
    addrValid = ((CY_EXT_NVM0_BASE <= address) && (address < (CY_EXT_NVM0_BASE + CY_EXT_NVM0_SIZE))) ||
    ((CY_EXT_NVM1_BASE <= address) && (address < (CY_EXT_NVM1_BASE + CY_EXT_NVM1_SIZE)));
        CY_UNUSED_PARAMETER(params);
    #else /* Internal memory */
        #ifdef CY_IP_M7CPUSS
            blocks_sector_size = 0U;
            for (uint32_t block_num = 0U; block_num < blocks_count; block_num++)
            {
                uint32_t flash_start_address = (&blocks_info[block_num])->start_address;
                uint32_t flash_size = (&blocks_info[block_num])->size;
                if ((flash_start_address <= address) && (address < flash_start_address + flash_size))
                {
                    blocks_sector_size = (&blocks_info[0])->sector_size;
                    break;
                }
            }
            addrValid = (blocks_sector_size > 0U);
        #else
            #if defined CY_FLASH_BASE
                addrValid = (CY_FLASH_BASE <= address) &&
                            (address < (CY_FLASH_BASE + CY_FLASH_SIZE));
            #else
                CY_DFU_LOG_WRN("Address validation skipped");
                CY_UNUSED_PARAMETER(address);
            #endif /* defined CY_FLASH_BASE */
            CY_UNUSED_PARAMETER(params);
        #endif /* CY_IP_M7CPUSS */
    #endif /* (CY_DFU_OPT_EXTERNAL_MEMORY != 0U) */
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */

    return addrValid;
}


#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
    /*******************************************************************************
    * Function Name: GetStartEndAddress
    ****************************************************************************//**
    *
    * This internal function returns start and end address of application
    *
    * \param appId          The application number
    * \param startAddress   The pointer to a variable where an application start
    *                       address is stored
    * \param endAddress     The pointer to a variable where a size of application
    *                       area is stored.
    *
    *******************************************************************************/
    static void GetStartEndAddress(uint32_t appId, uint32_t *startAddress, uint32_t *endAddress)
    {
        uint32_t verifyStart;
        uint32_t verifySize;

        (void)Cy_DFU_GetAppMetadata(appId, &verifyStart, &verifySize);

    #if (CY_DFU_APP_FORMAT == CY_DFU_SIMPLIFIED_APP)
        *startAddress = verifyStart - CY_DFU_SIGNATURE_SIZE;
        *endAddress = verifyStart + verifySize;
    #else
        *startAddress = verifyStart;
        *endAddress = verifyStart + verifySize + CY_DFU_SIGNATURE_SIZE;
    #endif
    }
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */

#if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U)
    /*******************************************************************************
    * Function Name: Ext_Flash_WriteRow
    ****************************************************************************//**
    *
    * This internal function combines erase and write phases for storing data in external memory.
    *
    * \param address    The address in the serial memory where data must be stored.
    * \param length     The size of the stored data.
    * \param params     The pointer to a DFU parameters structure, see \ref cy_stc_dfu_params_t
    *
    * \return See \ref cy_en_dfu_status_t.
    *
    *******************************************************************************/
    static cy_en_dfu_status_t Ext_Flash_WriteRow(uint32_t address, size_t length, cy_stc_dfu_params_t *params)
    {
        cy_en_dfu_status_t status = CY_DFU_SUCCESS;

        size_t eraseBlockSize;
        size_t eraseBlockStart;

        uint32_t extmemAddress = address - CY_EXT_NVM0_BASE;

        if (serialMemObjPtr == NULL)
        {
            status = CY_DFU_ERROR_READ_EXT;
        }
        else
        {
            /* Erase command */
            if (length == 0U)
            {
                eraseBlockSize = mtb_serial_memory_get_erase_size(serialMemObjPtr, extmemAddress);

                /* The address is expected to be valid and aligned with external memory
                * Erase command rules.
                */

                cy_rslt_t extstatus = mtb_serial_memory_erase(serialMemObjPtr, extmemAddress, eraseBlockSize);
                status = (extstatus == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_WRITE_EXT;
            }
            else /* Write command */
            {
            #ifndef CY_DFU_DISABLE_EXTMEM_ERASE
                /* Erase all at once */
                #if defined (CY_DFU_APP_ADDRESS) && defined (CY_DFU_APP_SIZE)
                    /* Check if the address to write is the beginning of a new application */
                    if ((CY_DFU_APP_ADDRESS - CY_EXT_NVM0_BASE) == extmemAddress)
                    {
                        eraseBlockStart = mtb_serial_memory_get_sector_start_address(serialMemObjPtr, extmemAddress);

                        /* The size of memory to erase:
                         * the last sector address - the first sector address + the last sector size
                         */
                        eraseBlockSize = (size_t)mtb_serial_memory_get_sector_start_address(serialMemObjPtr, extmemAddress + (CY_DFU_APP_SIZE - 1U)) -
                                           extmemAddress + mtb_serial_memory_get_erase_size(serialMemObjPtr, extmemAddress + (CY_DFU_APP_SIZE - 1U));

                        cy_rslt_t extstatus = mtb_serial_memory_erase(serialMemObjPtr, eraseBlockStart, eraseBlockSize);
                        status = (extstatus == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_WRITE_EXT;
                    }
                #else /* if defined (CY_DFU_APP_ADDRESS) && defined (CY_DFU_APP_SIZE) */
                    /* Only erase the block for the current writing */
                    static size_t lastErasedBlockStart = 0U;
                    static size_t lastErasedBlockEnd = 0U;

                    /* Check and erase the memory */
                    if (!((lastErasedBlockStart <= extmemAddress) && ((extmemAddress + length) <= lastErasedBlockEnd)))
                    {
                        eraseBlockStart = mtb_serial_memory_get_sector_start_address(serialMemObjPtr, extmemAddress);

                        /* The size of memory to erase:
                         * the last sector address - the first sector address + the last sector size */
                        eraseBlockSize = (size_t)mtb_serial_memory_get_sector_start_address(serialMemObjPtr, extmemAddress + (length - 1U)) -
                                           extmemAddress + mtb_serial_memory_get_erase_size(serialMemObjPtr, extmemAddress + (length - 1U));

                        cy_rslt_t extstatus = mtb_serial_memory_erase(serialMemObjPtr, eraseBlockStart, eraseBlockSize);
                        if (extstatus == CY_RSLT_SUCCESS)
                        {
                            status = CY_DFU_SUCCESS;
                            /* Remember details of the block erased */
                            lastErasedBlockStart = eraseBlockStart;
                            lastErasedBlockEnd = eraseBlockStart + eraseBlockSize;
                        }
                        else
                        {
                            status = CY_DFU_ERROR_WRITE_EXT;
                        }
                    }
                #endif /* if defined (CY_DFU_APP_ADDRESS) && defined (CY_DFU_APP_SIZE) */
            #endif /* !define CY_DFU_DISABLE_EXTMEM_ERASE */

            if (status == CY_DFU_SUCCESS)
            {
                cy_rslt_t extstatus = mtb_serial_memory_write(serialMemObjPtr, extmemAddress, length, params->dataBuffer);
                status = (extstatus == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_WRITE_EXT;
            }
        }
    }

        return status;
    }


    /*******************************************************************************
    * Function Name: Ext_Flash_ReadRow
    ****************************************************************************//**
    *
    * This internal function reads data from external memory.
    *
    * \param address    The address in the serial memory from which data must be read.
    * \param length     The size of the requested data.
    * \param data       The pointer to the data buffer.
    *
    *******************************************************************************/
    static cy_en_dfu_status_t Ext_Flash_ReadRow(uint32_t address, size_t length, uint8_t *data)
    {
        cy_en_dfu_status_t status = CY_DFU_ERROR_READ_EXT;
        uint32_t extmemAddress = address - CY_EXT_NVM0_BASE;

        if (serialMemObjPtr == NULL)
        {
            status = CY_DFU_ERROR_READ_EXT;
        }
        else
        {
            cy_rslt_t extstatus = mtb_serial_memory_read(serialMemObjPtr, extmemAddress, length, data);
            status = (extstatus == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_READ_EXT;
        }

        return status;
    }
#endif /* (CY_DFU_OPT_EXTERNAL_MEMORY != 0U) */


/*******************************************************************************
* Function Name: Cy_DFU_WriteData
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_WriteData (uint32_t address, uint32_t length, uint32_t ctl,
                                               cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    /* Check if the address is inside the valid range */
    #if (defined (SECURE_ALIAS_OFFSET) && defined (CY_DEVICE_PSE84))
       address &= ~(SECURE_ALIAS_OFFSET);
    #endif
    if (!AddressValid(address, params))
    {
        status = CY_DFU_ERROR_ADDRESS;
    }

    /* Check if the length is valid
     * Note Length = 0 is valid for erase command */
    if ( (IsMultipleOf(address, CY_NVM_SIZEOF_ROW) == false) ||
         ( (length != CY_NVM_SIZEOF_ROW) && ( (ctl & CY_DFU_IOCTL_ERASE) == 0U) ) )
    {
        status = CY_DFU_ERROR_LENGTH;
    }

#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
    uint32_t app = Cy_DFU_GetRunningApp();
    uint32_t startAddress;
    uint32_t endAddress;

    GetStartEndAddress(app, &startAddress, &endAddress);

    /* Refuse to write to a row within a range of the current application */
    if ( (startAddress <= address) && (address < endAddress) )
    {   /* It is forbidden to overwrite the currently running application */
        status = CY_DFU_ERROR_ADDRESS;
    }

    #if CY_DFU_OPT_GOLDEN_IMAGE
        if (status == CY_DFU_SUCCESS)
        {
            uint8_t goldenImages[] = { CY_DFU_GOLDEN_IMAGE_IDS() };
            uint32_t count = sizeof(goldenImages) / sizeof(goldenImages[0]);
            uint32_t idx;
            for (idx = 0U; idx < count; ++idx)
            {
                app = goldenImages[idx];
                GetStartEndAddress(app, &startAddress, &endAddress);

                if ( (startAddress <= address) && (address < endAddress) )
                {
                    status = Cy_DFU_ValidateApp(app, params);
                    status = (status == CY_DFU_SUCCESS) ? CY_DFU_ERROR_ADDRESS : CY_DFU_SUCCESS;
                    break;
                }
            }
        }
    #endif /* #if CY_DFU_OPT_GOLDEN_IMAGE != 0 */
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/

    if (status == CY_DFU_SUCCESS)
    {
        if ((ctl & CY_DFU_IOCTL_ERASE) != 0U)
        {
            (void) memset(params->dataBuffer, 0, CY_NVM_SIZEOF_ROW);
        }

    #if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U)
        status = Ext_Flash_WriteRow(address, length, params);
    #else /* Internal flash */
        cy_rslt_t fstatus = CY_RSLT_SUCCESS;

        #ifdef CY_IP_M7CPUSS
            uint32_t int_status;
            int_status = mtb_hal_system_critical_section_enter();
            if(address % blocks_sector_size == 0U)
            {
                fstatus = mtb_hal_nvm_erase(&nvm_obj, address);
            }
            if(fstatus == CY_RSLT_SUCCESS)
            {
                fstatus = mtb_hal_nvm_program(&nvm_obj, address, (uint32_t*)params->dataBuffer);
                if(fstatus != CY_RSLT_SUCCESS)
                {
                    status = CY_DFU_ERROR_DATA;
                    CY_DFU_LOG_ERR("NVM program failed: module=0x%X code=0x%X",
                                        (unsigned int)CY_RSLT_GET_MODULE(fstatus),
                                        (unsigned int)CY_RSLT_GET_CODE(fstatus));
                }
            }
            else
            {
                status = CY_DFU_ERROR_DATA;
                CY_DFU_LOG_ERR("NVM erase failed: module=0x%X code=0x%X",
                                    (unsigned int)CY_RSLT_GET_MODULE(fstatus),
                                    (unsigned int)CY_RSLT_GET_CODE(fstatus));
            }
            mtb_hal_system_critical_section_exit(int_status);
        #else
            #if defined CY_IP_MXS40SSRSS && defined COMPONENT_NON_SECURE_DEVICE
                #warning "Add custom non-secure application NVM erase and NVM write calls"
            #else
                uint32_t int_status = mtb_hal_system_critical_section_enter();
CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 11.3','Casting uint8_t* to uint32_t* is safe as input address is always valid and aligned.');
                fstatus = mtb_hal_nvm_write(&nvm_obj, address, (uint32_t*)params->dataBuffer);
                mtb_hal_system_critical_section_exit(int_status);
                if(fstatus != CY_RSLT_SUCCESS)
                {
                    status = CY_DFU_ERROR_DATA;
                    CY_DFU_LOG_ERR("NVM write failed: fstatus 0x%X ", (unsigned int)fstatus);
                }
            #endif /* defined CY_IP_MXS40SSRSS && defined COMPONENT_NON_SECURE_DEVICE */
        #endif /* CY_IP_M7CPUSS */
    #endif /* (CY_DFU_OPT_EXTERNAL_MEMORY != 0) */
    }

    if (CY_DFU_SUCCESS != status)
    {
        CY_DFU_LOG_ERR("Write operation failed at address 0x%X", (unsigned int)address);
    }

    return (status);
}


/*******************************************************************************
* Function Name: Cy_DFU_ReadData
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_ReadData (uint32_t address, uint32_t length, uint32_t ctl,
                                              cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    /* Check if the length is valid */
    if (IsMultipleOf(length, CY_NVM_SIZEOF_ROW) == false)
    {
        status = CY_DFU_ERROR_LENGTH;
    }

    /* Check if the address is inside the valid range */
    #if (defined (SECURE_ALIAS_OFFSET) && defined (CY_DEVICE_PSE84))
        address &= ~(SECURE_ALIAS_OFFSET);
    #endif
    if (!AddressValid(address, params))
    {
        status = CY_DFU_ERROR_ADDRESS;
    }

    /* Read or Compare */
    if (status == CY_DFU_SUCCESS)
    {
        if ((ctl & CY_DFU_IOCTL_COMPARE) == 0U)
        {
        #if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U)
            status = Ext_Flash_ReadRow(address, length, params->dataBuffer);
        #elif defined CY_IP_MXS40SSRSS && defined COMPONENT_NON_SECURE_DEVICE
            (void)memcpy(params->dataBuffer, (const uint8_t*)address, (size_t)length);
            status = CY_DFU_SUCCESS;
        #else
            cy_rslt_t fstatus = mtb_hal_nvm_read(&nvm_obj, address, params->dataBuffer, length);
            status = (fstatus == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_DATA;
        #endif /* (CY_DFU_OPT_EXTERNAL_MEMORY != 0U) */
        }
        else
        {
        #if (CY_DFU_OPT_EXTERNAL_MEMORY != 0U)
            uint8_t dataBuffer[CY_DFU_SIZEOF_DATA_BUFFER];
            status = Ext_Flash_ReadRow(address, length, dataBuffer);

            if (status == CY_DFU_SUCCESS)
            {
                status = ( memcmp(params->dataBuffer, dataBuffer, length) == 0)
                        ? CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
            }
        #else
CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 11.6','The cast from unsigned int to the pointer does not have any unintended effect, as the casted value represents the memory address');
            status = ( memcmp((const void *) params->dataBuffer, (const void *)address, length) == 0 )
                    ? CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
        #endif /* (CY_DFU_OPT_EXTERNAL_MEMORY != 0U) */
        }
    }
    return (status);
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportStart
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportStart(cy_en_dfu_transport_t transport)
{
    selectedInterface = transport;

#if (CY_DFU_OPT_EXTERNAL_MEMORY == 0U)
    /* Initialize NVM object */
    #if defined CY_IP_MXS40SSRSS && defined COMPONENT_NON_SECURE_DEVICE
        #warning "Add custom non-secure application NVM initialization call"
    #endif /* defined CY_IP_MXS40SSRSS && defined COMPONENT_NON_SECURE_DEVICE */
#endif /* (CY_DFU_OPT_EXTERNAL_MEMORY == 0U) */

#ifdef CY_IP_M7CPUSS
    mtb_hal_nvm_info_t nvm_info;
    /* Enable code flash write function */
    Cy_Flashc_MainWriteEnable();

    /* Get NVM characteristics */
    mtb_hal_nvm_get_info(&nvm_obj, &nvm_info);
    blocks_info = nvm_info.regions;
    blocks_count = nvm_info.region_count;
#endif

    switch (transport)
    {
    #ifdef COMPONENT_DFU_I2C
        case CY_DFU_I2C:
            I2C_I2cCyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_I2C */

    #ifdef COMPONENT_DFU_UART
        case CY_DFU_UART:
            UART_UartCyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_UART */
    #ifdef COMPONENT_DFU_SPI
        case CY_DFU_SPI:
            SPI_SpiCyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_SPI */
    #ifdef COMPONENT_DFU_USB_CDC
        case CY_DFU_USB_CDC:
            USB_CDC_CyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_USB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_CDC
        case CY_DFU_USB_CDC:
            USB_CDC_CyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_EMUSB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_HID
        case CY_DFU_USB_HID:
            USB_HID_CyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_EMUSB_HID */
    #ifdef COMPONENT_DFU_CANFD
        case CY_DFU_CANFD:
            CANFD_CanfdCyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_CANFD */
    #ifdef COMPONENT_DFU_PMBUS
        case CY_DFU_PMBUS:
            PMBUS_CyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_PMBUS */

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }
}

#if (CY_DFU_OPT_HOST_MODE != 0U)
/*******************************************************************************
* Function Name: Cy_DFU_HostTransportStart
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_HostTransportStart(cy_en_dfu_transport_t transport)
{
    selectedHostInterface = transport;

    #ifdef COMPONENT_DFU_HOST_I2C
    if (transport == CY_DFU_I2C)
    {
        Host_I2cStart();
    }
    else
    #endif /* COMPONENT_DFU_HOST_I2C */
    #ifdef COMPONENT_DFU_HOST_UART
    if (transport == CY_DFU_UART)
    {
        Host_UartStart();
    }
    else
    #endif /* COMPONENT_DFU_HOST_UART */
    {
        /* Selected interface in not applicable */
        CY_DFU_LOG_ERR("Selected interface in not applicable");
        CY_ASSERT(false);
        (void)transport;
    }
}
#endif /* CY_DFU_OPT_HOST_MODE */


/*******************************************************************************
* Function Name: Cy_DFU_TransportStop
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportStop(void)
{
    switch (selectedInterface)
    {
    #ifdef COMPONENT_DFU_I2C
        case CY_DFU_I2C:
            I2C_I2cCyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_I2C */

    #ifdef COMPONENT_DFU_UART
        case CY_DFU_UART:
            UART_UartCyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_UART */
    #ifdef COMPONENT_DFU_SPI
        case CY_DFU_SPI:
            SPI_SpiCyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_SPI */
    #ifdef COMPONENT_DFU_USB_CDC
        case CY_DFU_USB_CDC:
            USB_CDC_CyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_USB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_CDC
        case CY_DFU_USB_CDC:
            USB_CDC_CyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_EMUSB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_HID
        case CY_DFU_USB_HID:
            USB_HID_CyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_EMUSB_HID */
    #ifdef COMPONENT_DFU_CANFD
        case CY_DFU_CANFD:
            CANFD_CanfdCyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_CANFD */
    #ifdef COMPONENT_DFU_PMBUS
        case CY_DFU_PMBUS:
            PMBUS_CyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_PMBUS */

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }
}

#if (CY_DFU_OPT_HOST_MODE != 0U)
/*******************************************************************************
* Function Name: Cy_DFU_HostTransportStop
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_HostTransportStop(void)
{
    #ifdef COMPONENT_DFU_HOST_I2C
    if (selectedHostInterface == CY_DFU_I2C)
    {
        Host_I2cStop();
    }
    else
    #endif /* COMPONENT_DFU_HOST_I2C */
    #ifdef COMPONENT_DFU_HOST_UART
    if (selectedHostInterface == CY_DFU_UART)
    {
        Host_UartStop();
    }
    else
    #endif /* COMPONENT_DFU_HOST_UART */
    {
        /* Selected interface in not applicable */
        CY_DFU_LOG_ERR("Selected interface in not applicable");
        CY_ASSERT(false);
    }

    selectedHostInterface = CY_DFU_NONE;
}
#endif /* CY_DFU_OPT_HOST_MODE */

/*******************************************************************************
* Function Name: Cy_DFU_TransportReset
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportReset(void)
{
    switch (selectedInterface)
    {
    #ifdef COMPONENT_DFU_I2C
        case CY_DFU_I2C:
            I2C_I2cCyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_I2C */

    #ifdef COMPONENT_DFU_UART
        case CY_DFU_UART:
            UART_UartCyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_UART */
    #ifdef COMPONENT_DFU_SPI
        case CY_DFU_SPI:
            SPI_SpiCyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_SPI */
    #ifdef COMPONENT_DFU_USB_CDC
        case CY_DFU_USB_CDC:
            USB_CDC_CyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_USB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_CDC
        case CY_DFU_USB_CDC:
            USB_CDC_CyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_EMUSB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_HID
        case CY_DFU_USB_HID:
            USB_HID_CyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_EMUSB_HID */
    #ifdef COMPONENT_DFU_CANFD
        case CY_DFU_CANFD:
            CANFD_CanfdCyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_CANFD */
    #ifdef COMPONENT_DFU_PMBUS
        case CY_DFU_PMBUS:
            PMBUS_CyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_PMBUS */

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }
}

#if (CY_DFU_OPT_HOST_MODE != 0U)
/*******************************************************************************
* Function Name: Cy_DFU_HostTransportReset
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_HostTransportReset(void)
{
    #ifdef COMPONENT_DFU_HOST_I2C
    if (selectedHostInterface == CY_DFU_I2C)
    {
        Host_I2cReset();
    }
    else
    #endif /* COMPONENT_DFU_HOST_I2C */
    #ifdef COMPONENT_DFU_HOST_UART
    if (selectedHostInterface == CY_DFU_UART)
    {
        Host_UartReset();
    }
    else
    #endif /* COMPONENT_DFU_HOST_UART */
    {
        /* Selected interface in not applicable */
        CY_DFU_LOG_ERR("Selected interface in not applicable");
        CY_ASSERT(false);
    }
}
#endif /* CY_DFU_OPT_HOST_MODE */

/*******************************************************************************
* Function Name: Cy_DFU_TransportRead
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_TransportRead(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;

    switch (selectedInterface)
    {
    #ifdef COMPONENT_DFU_I2C
        case CY_DFU_I2C:
            status = I2C_I2cCyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_I2C */

    #ifdef COMPONENT_DFU_UART
        case CY_DFU_UART:
            status = UART_UartCyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_UART */
    #ifdef COMPONENT_DFU_SPI
        case CY_DFU_SPI:
            status = SPI_SpiCyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_SPI */
    #ifdef COMPONENT_DFU_USB_CDC
        case CY_DFU_USB_CDC:
            status = USB_CDC_CyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_USB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_CDC
        case CY_DFU_USB_CDC:
            status = USB_CDC_CyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_EMUSB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_HID
        case CY_DFU_USB_HID:
            status = USB_HID_CyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_EMUSB_HID */
    #ifdef COMPONENT_DFU_CANFD
        case CY_DFU_CANFD:
            status = CANFD_CanfdCyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_CANFD */
    #ifdef COMPONENT_DFU_PMBUS
        case CY_DFU_PMBUS:
            status = PMBUS_CyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_PMBUS */

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }

    return status;
}

#if (CY_DFU_OPT_HOST_MODE != 0U)
/*******************************************************************************
* Function Name: Cy_DFU_HostTransportRead
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_HostTransportRead(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;

    CY_ASSERT(selectedHostInterface != CY_DFU_NONE);

    #ifdef COMPONENT_DFU_HOST_I2C
    if (selectedHostInterface == CY_DFU_I2C)
    {
        status = Host_I2cRead(buffer, size, count, timeout);
    }
    else
    #endif /* COMPONENT_DFU_HOST_I2C */
    #ifdef COMPONENT_DFU_HOST_UART
    if (selectedHostInterface == CY_DFU_UART)
    {
        status = Host_UartRead(buffer, size, count, timeout);
    }
    else
    #endif /* COMPONENT_DFU_HOST_UART */
    {
        /* Selected interface in not applicable */
        CY_DFU_LOG_ERR("Selected interface in not applicable");
        CY_ASSERT(false);

        (void)buffer;
        (void)size;
        (void)count;
        (void)timeout;
    }

    return status;
}
#endif /* CY_DFU_OPT_HOST_MODE */

/*******************************************************************************
* Function Name: Cy_DFU_TransportWrite
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_TransportWrite(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;

    switch (selectedInterface)
    {
    #ifdef COMPONENT_DFU_I2C
        case CY_DFU_I2C:
            status = I2C_I2cCyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_I2C */

    #ifdef COMPONENT_DFU_UART
        case CY_DFU_UART:
            status = UART_UartCyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_UART */
    #ifdef COMPONENT_DFU_SPI
        case CY_DFU_SPI:
            status = SPI_SpiCyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_SPI */
    #ifdef COMPONENT_DFU_USB_CDC
        case CY_DFU_USB_CDC:
            status = USB_CDC_CyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_USB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_CDC
        case CY_DFU_USB_CDC:
            status = USB_CDC_CyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_EMUSB_CDC */
    #ifdef COMPONENT_DFU_EMUSB_HID
        case CY_DFU_USB_HID:
            status = USB_HID_CyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_EMUSB_HID */
    #ifdef COMPONENT_DFU_CANFD
        case CY_DFU_CANFD:
            status = CANFD_CanfdCyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_CANFD */
    #ifdef COMPONENT_DFU_PMBUS
        case CY_DFU_PMBUS:
            status = PMBUS_CyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_PMBUS */

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }

    return status;
}

#if (CY_DFU_OPT_HOST_MODE != 0U)
/*******************************************************************************
* Function Name: Cy_DFU_HostTransportWrite
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_HostTransportWrite(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;

    CY_ASSERT(selectedHostInterface != CY_DFU_NONE);

    #ifdef COMPONENT_DFU_HOST_I2C
    if (selectedHostInterface == CY_DFU_I2C)
    {
        status = Host_I2cWrite(buffer, size, count, timeout);
    }
    else
    #endif /* COMPONENT_DFU_HOST_I2C */
    #ifdef COMPONENT_DFU_HOST_UART
    if (selectedHostInterface == CY_DFU_UART)
    {
        status = Host_UartWrite(buffer, size, count, timeout);
    }
    else
    #endif /* COMPONENT_DFU_HOST_UART */
    {
        /* Selected interface in not applicable */
        CY_DFU_LOG_ERR("Selected interface in not applicable");
        CY_ASSERT(false);

        (void)buffer;
        (void)size;
        (void)count;
        (void)timeout;
    }

    return status;
}
#endif /* CY_DFU_OPT_HOST_MODE */

/* [] END OF FILE */
