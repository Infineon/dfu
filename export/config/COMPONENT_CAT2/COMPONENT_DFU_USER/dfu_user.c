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
#include "cy_dfu.h"

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

#ifdef COMPONENT_DFU_BLE
    #include "transport_ble.h"
#endif  /* COMPONENT_DFU_BLE */



/*
* The DFU SDK metadata initial value is placed here
* Note: the number of elements equal to the number of the app multiplies by 2
*       because of the two fields per app plus one element for the CRC-32C field.
*/
CY_MISRA_DEVIATE_BLOCK_START('MISRA C-2012 Rule 9.3',1,'The rest of the array initialization is not required.');
CY_SECTION(".cy_boot_metadata") __USED
static const uint32_t cy_dfu_metadata[CY_FLASH_SIZEOF_ROW / sizeof(uint32_t)] =

{
    CY_DFU_APP0_VERIFY_START, CY_DFU_APP0_VERIFY_LENGTH, /* The App0 base address and length */
    CY_DFU_APP1_VERIFY_START, CY_DFU_APP1_VERIFY_LENGTH, /* The App1 base address and length */
    0U                                                             /* The rest does not matter     */
};
CY_MISRA_BLOCK_END('MISRA C-2012 Rule 9.3');


static uint32_t IsMultipleOf(uint32_t value, uint32_t multiple);
static void GetStartEndAddress(uint32_t appId, uint32_t *startAddress, uint32_t *endAddress);
static cy_en_dfu_transport_t selectedInterface = CY_DFU_I2C;


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
* \return 1 - value is multiple of parameter multiple, else 0
*
*******************************************************************************/
static uint32_t IsMultipleOf(uint32_t value, uint32_t multiple)
{
    return ( ((value % multiple) == 0U)? 1UL : 0UL);
}


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
    /* application flash limits */
    /* Note that App0 is out of range */
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_DFU_APP0_VERIFY_LENGTH;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;

    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    uint32_t app = Cy_DFU_GetRunningApp();
    uint32_t startAddress;
    uint32_t endAddress;

    GetStartEndAddress(app, &startAddress, &endAddress);

    /* Check if the address  and length are valid
     * Note Length = 0 is valid for erase command */
    if ( (IsMultipleOf(address, CY_FLASH_SIZEOF_ROW) == 0U) ||
         ( (length != CY_FLASH_SIZEOF_ROW) && ( (ctl & CY_DFU_IOCTL_ERASE) == 0U) ) )
    {
        status = CY_DFU_ERROR_LENGTH;
    }

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

    /* Check if the address is inside the valid range */
    if ((minUFlashAddress <= address) && (address < maxUFlashAddress))
    {   /* Do nothing, this is an allowed memory range to update to */
    }
    else
    {
        status = CY_DFU_ERROR_ADDRESS;
    }

    if (status == CY_DFU_SUCCESS)
    {
        if ((ctl & CY_DFU_IOCTL_ERASE) != 0U)
        {
            (void) memset(params->dataBuffer, 0, CY_FLASH_SIZEOF_ROW);
        }
CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 11.3','Casting uint8_t* to uint32_t* is safe as input address is always valid and aligned.');
        cy_en_flashdrv_status_t fstatus =  Cy_Flash_WriteRow(address, (uint32_t*)params->dataBuffer);
        status = (fstatus == CY_FLASH_DRV_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_DATA;
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
    /* application flash limits */
    /* Note that App0 is out of range */
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_DFU_APP0_VERIFY_LENGTH;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;

    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    /* Check if the length is valid */
    if (IsMultipleOf(length, CY_FLASH_SIZEOF_ROW) == 0U)
    {
        status = CY_DFU_ERROR_LENGTH;
    }

    /* Check if the address is inside the valid range */
    if ((minUFlashAddress <= address) && (address < maxUFlashAddress))
    {   /* Do nothing, this is an allowed memory range to update to */
    }
    else
    {
        status = CY_DFU_ERROR_ADDRESS;
    }

    /* Read or Compare */
    if (status == CY_DFU_SUCCESS)
    {
        if ((ctl & CY_DFU_IOCTL_COMPARE) == 0U)
        {
CY_MISRA_DEVIATE_BLOCK_START('MISRA C-2012 Rule 11.6',2,'Cast of uint32_t value\
to void* is safe as variable always has valid address value');
            (void) memcpy((void *)params->dataBuffer, (const void *)address, length);
            status = CY_DFU_SUCCESS;
        }
        else
        {
            status = ( memcmp((const void *)params->dataBuffer, (const void *)address, length) == 0 )
                     ? CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
CY_MISRA_BLOCK_END('MISRA C-2012 Rule 11.6');
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
    #ifdef COMPONENT_DFU_BLE
        case CY_DFU_BLE:
            CyBLE_CyBtldrCommStart();
            break;
    #endif /* COMPONENT_DFU_BLE */

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
    #ifdef COMPONENT_DFU_BLE
        case CY_DFU_BLE:
            CyBLE_CyBtldrCommStop();
            break;
    #endif /* COMPONENT_DFU_BLE */

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
    #ifdef COMPONENT_DFU_BLE
        case CY_DFU_BLE:
            CyBLE_CyBtldrCommReset();
            break;
    #endif /* COMPONENT_DFU_BLE */

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
    #ifdef COMPONENT_DFU_BLE
        case CY_DFU_BLE:
            status = CyBLE_CyBtldrCommRead(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_BLE */

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
    #ifdef COMPONENT_DFU_BLE
        case CY_DFU_BLE:
            status = CyBLE_CyBtldrCommWrite(buffer, size, count, timeout);
            break;
    #endif /* COMPONENT_DFU_BLE */

        default:
            /* Selected interface in not applicable */
            CY_ASSERT(false);
            break;
    }

    return status;
}


/* [] END OF FILE */
