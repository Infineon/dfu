/***************************************************************************//**
* \file transport_emusb_cdc.c
* \version 1.0
*
* This file provides the source code of the DFU communication API implementation
* for the emUSB-Device that implements a virtual COM port (CDC class).
*
* Note
* This file supports only the ModusToolbox flow.
* For the ModusToolbox flow, the USB device personality alias must be
* "DFU_EMUSB_CDC".
* This file serves as a template and can be modified defining any component
* name or personality alias.
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

#include "USB.h"
#include "USB_CDC.h"
#include "transport_emusb_cdc.h"


/* USER CONFIGURABLE: The USB device COM port data endpoint maximum packet size */
#ifndef CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET
    #define CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET   (USB_FS_BULK_MAX_PACKET_SIZE)
#endif /* #ifndef CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET */

/* Interrupt endpoint size */
#define CY_DFU_USB_HID_ENDPOINT_SIZE         (USB_FS_INT_MAX_PACKET_SIZE)

/**
* USB_DEV_CDC_initVar indicates whether the emUSB-Device has been initialized. The
* variable is initialized to false and set to true the first time
* \ref USB_CDC_CyBtldrCommStart is called. This allows  the driver to restart
* without re-initialization after the first call to the
* \ref USB_CDC_CyBtldrCommStart routine.
* For re-initialization set \ref USB_DEV_CDC_initVar to false and call
* \ref USB_CDC_CyBtldrCommStart.
*/
bool USB_DEV_CDC_initVar = false;

/* Data structure for emUSB-Device */
static const USB_DEVICE_INFO _DeviceInfo =
{
    0x058B,                    // VendorId
    0xF21D,                    // ProductId
    "Infineon",                // VendorName
    "DFU USB CDC Transport",   // ProductName
    "0132456789"               // SerialNumber
};

/* Handle for emUSB CDC instance */
static USB_CDC_HANDLE    hInst;

/* Buffer for store data in OUT direction (Host to Device) */
static U8 OutBuffer[CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET];


/*******************************************************************************
* Function Name: USB_DEV_Start
****************************************************************************//**
*
* Starts the USB device operation. It does not wait until USB device is
* enumerated. It invokes USBD_Start().
*
* \globalvars
* \ref USB_DEV_CDC_initVar - Used to check the initial configuration, modified on the
*                        first function call.
*
*******************************************************************************/
static void USB_DEV_Start(void);
static void USB_DEV_Start(void)
{

    USB_ADD_EP_INFO EPBulkIn;
    USB_ADD_EP_INFO EPBulkOut;
    USB_ADD_EP_INFO EPIntIn;
    USB_CDC_INIT_DATA InitData;

    if (!USB_DEV_CDC_initVar)
    {
        memset(&EPBulkIn, 0x0, sizeof(EPBulkIn));
        memset(&EPBulkOut, 0x0, sizeof(EPBulkOut));
        memset(&EPIntIn, 0x0, sizeof(EPIntIn));
        memset(&InitData, 0x0, sizeof(InitData));

        /* Initializes the USB device with its settings */
        USBD_Init();

        /* IN direction (Device to Host) endpoint */
        EPBulkIn.Flags          = 0;                                  // Flags not used.
        EPBulkIn.InDir          = USB_DIR_IN;                         // IN direction (Device to Host)
        EPBulkIn.Interval       = 0;                                  // Interval not used for Bulk endpoints.
        EPBulkIn.MaxPacketSize  = CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET; // Maximum packet size (64 for Bulk in full-speed and 512 for high-speed).
        EPBulkIn.TransferType   = USB_TRANSFER_TYPE_BULK;             // Endpoint type - Bulk.
        InitData.EPIn  = USBD_AddEPEx(&EPBulkIn, NULL, 0);

        /* OUT direction (Device to Host) endpoint */
        EPBulkOut.Flags         = 0;                                  // Flags not used.
        EPBulkOut.InDir         = USB_DIR_OUT;                        // OUT direction (Host to Device)
        EPBulkOut.Interval      = 0;                                  // Interval not used for Bulk endpoints.
        EPBulkOut.MaxPacketSize = CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET; // Maximum packet size (64 for Bulk in full-speed and 512 for high-speed).
        EPBulkOut.TransferType  = USB_TRANSFER_TYPE_BULK;             // Endpoint type - Bulk.
        InitData.EPOut = USBD_AddEPEx(&EPBulkOut, OutBuffer, sizeof(OutBuffer));

        /* Interrupt endpoint */
        EPIntIn.Flags           = 0;                                  // Flags not used.
        EPIntIn.InDir           = USB_DIR_IN;                         // IN direction (Device to Host)
        EPIntIn.Interval        = 64;                                 // Interval of 8 ms (64 * 125us)
        EPIntIn.MaxPacketSize   = CY_DFU_USB_HID_ENDPOINT_SIZE;       // Maximum packet size (64 for Interrupt).
        EPIntIn.TransferType    = USB_TRANSFER_TYPE_INT;              // Endpoint type - Interrupt.
        InitData.EPInt = USBD_AddEPEx(&EPIntIn, NULL, 0);

        /* Adds a CDC class to the stack */
        hInst = USBD_CDC_Add(&InitData);

        /* Set data for device enumeration */
        USBD_SetDeviceInfo(&_DeviceInfo);

        USB_DEV_CDC_initVar = true;
    }
}



/*******************************************************************************
* Function Name: USB_CDC_CyBtldrCommStart
****************************************************************************//**
*
* Starts the USB device operation.
*
* \note
* This function does not configure an infrastructure required for the USB device
* operation: clocks and pins. For the ModusToolbox flow, the generated files
* configure clocks and pins. This configuration must be performed by the
* application when the project uses only PDL.
*
*******************************************************************************/
void USB_CDC_CyBtldrCommStart(void)
{
    USB_DEV_Start();

    /* Start the emUSB-Device Core */
    USBD_Start();
}


/*******************************************************************************
* Function Name: USB_CDC_CyBtldrCommStop
****************************************************************************//**
*
* Disables the USB device component.
*
*******************************************************************************/
void USB_CDC_CyBtldrCommStop(void)
{
    /* Stop the USB communication with detaching Device from the HOST */
    USBD_DeInit();
}


/*******************************************************************************
* Function Name: USB_CDC_CyBtldrCommReset
****************************************************************************//**
*
* Resets the receive and transmits communication buffers.
*
*******************************************************************************/
void USB_CDC_CyBtldrCommReset(void)
{
    if (USBD_IsConfigured() > 0U)
    {
        /* Cancel any read or write operation */
        USBD_CDC_CancelRead(hInst);
        USBD_CDC_CancelWrite(hInst);
    }
}


/*******************************************************************************
* Function Name: USB_CDC_CyBtldrCommRead
****************************************************************************//**
*
* Allows the caller to read data from the DFU host (the host writes the
* data). The function handles polling to allow a block of data to be completely
* received from the host device.
*
* \param pData   The pointer to a buffer to store a received command.
* \param size    The number of bytes to be read.
* \param count   The pointer to the variable that contains the number of received bytes.
* \param timeout The time to wait before the function returns because of a timeout,
*                in milliseconds.
*
* \return
* The status of the operation:
* - \ref CY_DFU_SUCCESS - If successful.
* - \ref CY_DFU_ERROR_TIMEOUT - If no data has been received.
* - See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
cy_en_dfu_status_t USB_CDC_CyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t retCode = CY_DFU_ERROR_TIMEOUT;

    CY_ASSERT_L1((pData != NULL) && (size > 0U) && (count != NULL));

    /* Check Device enumeration */
    if ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED)
    {
        /* Wait (blocking with timeout) for data to be available for a read */
        int32_t retVal = USBD_CDC_Receive(hInst, pData, size, timeout);

        /* Data received successfully */
        if (retVal > 0)
        {
            *count = retVal;
            retCode = CY_DFU_SUCCESS;
        }

        /* An error occurred */
        if (retVal < 0)
        {
            retCode = CY_DFU_ERROR_UNKNOWN;
        }
    }

    return (retCode);
}


/*******************************************************************************
* Function Name: USB_CDC_CyBtldrCommWrite
****************************************************************************//**
*
* Allows the caller to write data to the DFU host (the host reads the
* data). The function uses a timeout and returns after data has been
* copied into the transmit buffer. The data transmission starts immediately
* after the first data element is written into the buffer and lasts until all
* data elements from the buffer are sent.
*
* \param pData     The pointer to the block of data to be written to the DFU
*                  host.
* \param size      The number of bytes to be written.
* \param count     The pointer to the variable to write the number of actually written bytes.
* \param timeOut   The time out to wait for before the data is copied to the transmit buffer.
*                  The function returns as soon as data is copied into the
*                  transmit buffer.
* \return
* The status of the operation:
* - \ref CY_DFU_SUCCESS - If successful.
* - See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
cy_en_dfu_status_t USB_CDC_CyBtldrCommWrite(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t retCode = CY_DFU_ERROR_TIMEOUT;

    CY_ASSERT_L1((pData != NULL) && (size > 0U) && (count != NULL));
    CY_ASSERT_L1(size <= CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET);

    /* Check Device enumeration */
    if ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED)
    {
        /* Wait (blocking with timeout) for an endpoint availability for a write */
        if (USBD_CDC_WaitForTX(hInst, timeout) == 0U)
        {
            /* Write data to the Host (blocking with timeout) */
            int32_t retVal = USBD_CDC_Write(hInst, pData, size, timeout);
            int32_t numBytes = size;

            /* Data sent successfully */
            if (retVal == numBytes)
            {
                *count = size;
                retCode = CY_DFU_SUCCESS;
            }

            /* An error occurred */
            if (retVal < 0)
            {
                retCode = CY_DFU_ERROR_UNKNOWN;
            }
        }
    }

    return (retCode);
}


/* [] END OF FILE */
