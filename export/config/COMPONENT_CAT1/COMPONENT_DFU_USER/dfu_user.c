/***************************************************************************//**
* \file dfu_user.c
* \version 5.1
*
* This file provides the custom API for a firmware application with
* DFU SDK.
* - Cy_DFU_ReadData (address, length, ctl, params) - to read  the NVM block
* - Cy_Bootalod_WriteData(address, length, ctl, params) - to write the NVM block
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

#include <string.h>
#include "cy_syslib.h"
#include "cyhal.h"
#include "cy_dfu.h"
#include "cy_dfu_logging.h"

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


/* Global flash object */
static cyhal_nvm_t flash_obj;

static cy_en_dfu_transport_t selectedInterface = CY_DFU_UART;

#ifdef CY_IP_M7CPUSS
    static const cyhal_flash_block_info_t* blocks_info;
    static uint8_t blocks_count;
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
    #if defined CY_FLASH_BASE
        addrValid = (CY_FLASH_BASE <= address) &&
                                (address < (CY_FLASH_BASE + CY_FLASH_SIZE));
    #else
        CY_UNUSED_PARAMETER(address);
    #endif
    CY_UNUSED_PARAMETER(params);
#endif

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
    if(!AddressValid(address, params))
    {
        status = CY_DFU_ERROR_ADDRESS;
    }

    /* Check if the length is valid
     * Note Length = 0 is valid for erase command */
    if ( (IsMultipleOf(address, CY_NVM_SIZEOF_ROW) == 0U) ||
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

        cy_rslt_t fstatus = CY_RSLT_SUCCESS;

        #ifdef CY_IP_M7CPUSS

            uint32_t nvm_sector_size = 0U;
            uint32_t int_status;
            for (uint32_t block_num = 0; block_num < blocks_count; block_num++)
            {
                uint32_t flash_start_address = (&blocks_info[block_num])->start_address;
                uint32_t flash_size          = (&blocks_info[block_num])->size;
                if((flash_start_address <= address) && (address < flash_start_address + flash_size))
                {
                    nvm_sector_size = (&blocks_info[0])->sector_size;
                    break;
                }
            }
            if(nvm_sector_size == 0U)
            {
                CY_ASSERT(false);
            }
            int_status = Cy_SysLib_EnterCriticalSection();
            if(address % nvm_sector_size == 0)
            {
                fstatus = cyhal_flash_erase(&flash_obj, address);
            }
            if(fstatus == CY_RSLT_SUCCESS)
            {
                fstatus = cyhal_flash_program(&flash_obj, address, (uint32_t*)params->dataBuffer);
            }
            else
            {
                status = CY_DFU_ERROR_DATA;
                CY_DFU_LOG_ERR("Flash erase failed: module=0x%X code=0x%X",
                                    (unsigned int)CY_RSLT_GET_MODULE(fstatus),
                                    (unsigned int)CY_RSLT_GET_CODE(fstatus));
            }
            Cy_SysLib_ExitCriticalSection(int_status);
        #else
            fstatus = cyhal_flash_write(&flash_obj, address, (uint32_t*)params->dataBuffer);
        #endif /* CY_IP_M7CPUSS */
        if((CY_DFU_SUCCESS == status) && (fstatus != CY_RSLT_SUCCESS))
        {
            status = CY_DFU_ERROR_DATA;
            CY_DFU_LOG_ERR("Flash write failed: fstatus 0x%X ", (unsigned int)fstatus);
        }
        else
        {
            status = CY_DFU_SUCCESS;
        }
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
    if (IsMultipleOf(length, CY_NVM_SIZEOF_ROW) == 0U)
    {
        status = CY_DFU_ERROR_LENGTH;
    }

    /* Check if the address is inside the valid range */
    if(!AddressValid(address, params))
    {
        status = CY_DFU_ERROR_ADDRESS;
    }

    /* Read or Compare */
    if (status == CY_DFU_SUCCESS)
    {
        if ((ctl & CY_DFU_IOCTL_COMPARE) == 0U)
        {
            cy_rslt_t fstatus = cyhal_flash_read(&flash_obj, address, params->dataBuffer, length);
            status = (fstatus == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_DATA;
        }
        else
        {
            status = ( memcmp(params->dataBuffer, (const void *)address, length) == 0 )
                    ? CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
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

    /* Initialize flash object */
    cy_rslt_t result = cyhal_nvm_init(&flash_obj);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_DFU_LOG_ERR("HAL FLash Initialization failed");
        CY_ASSERT(false);
    }

#ifdef CY_IP_M7CPUSS
    cyhal_flash_info_t flash_info;
    /* Enable code flash write function */
    Cy_Flashc_MainWriteEnable();

    /* Get flash characteristics */
    cyhal_flash_get_info(&flash_obj, &flash_info);
    blocks_info = flash_info.blocks;
    blocks_count = flash_info.block_count;
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

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportStop
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportStop()
{
    /* Release flash object */
    cyhal_nvm_free(&flash_obj);

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

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }
}


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

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }
}


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

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }

    return status;
}


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

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }

    return status;
}


/* [] END OF FILE */
