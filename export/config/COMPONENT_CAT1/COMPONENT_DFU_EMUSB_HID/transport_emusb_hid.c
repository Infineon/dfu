/***************************************************************************//**
* \file transport_emusb_hid.c
* \version 5.2
*
* This file provides the source code of the DFU communication API implementation
* for the emUSB-Device that implements a HID class.
*
********************************************************************************
* \copyright
* (c) (2016-2024), Cypress Semiconductor Corporation (an Infineon company) or
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
#include "USB_HID.h"
#include "transport_emusb_hid.h"

#include "stdio.h"

/* MAX size in bytes for HID data packet */
#ifndef CY_DFU_USB_HID_INT_MAX_PACKET
    #define CY_DFU_USB_HID_INT_MAX_PACKET           (USB_FS_INT_MAX_PACKET_SIZE)
#endif /* #ifndef CY_DFU_USB_HID_INT_MAX_PACKET */

/* Defines the input (device -> host) report size */
#define INPUT_REPORT_SIZE   (CY_DFU_USB_HID_INT_MAX_PACKET)

/* Defines the output (Host -> device) report size */
#define OUTPUT_REPORT_SIZE  (CY_DFU_USB_HID_INT_MAX_PACKET)

/* Defines the vendor specific page that
 * shall be used, allowed values 0x00 - 0xff.
 * This value must be identical to HOST application.
*/
#define VENDOR_PAGE_ID      (0x00)


/**
* USB_DEV_HID_initVar indicates whether the emUSB-Device has been initialized. The
* variable is initialized to false and set to true the first time
* \ref USB_HID_CyBtldrCommStart is called. This allows  the driver to restart
* without re-initialization after the first call to the
* \ref USB_HID_CyBtldrCommStart routine.
* For re-initialization set \ref USB_DEV_HID_initVar to false and call
* \ref USB_HID_CyBtldrCommStart.
*/
bool USB_DEV_HID_initVar = false;

/* Data structure for emUSB-Device */
static const USB_DEVICE_INFO DeviceInfo =
{
    0x058B,                    // VendorId
    0xF21D,                    // ProductId
    "Infineon",                // VendorName
    "PSoC_DFU_HID_Dev",        // ProductName
    "0132456789"               // SerialNumber
};

/* Data structure for Report descriptor */
static const U8 HIDReport[] =
{
    0x06, VENDOR_PAGE_ID, 0xFF,    // USAGE_PAGE (Vendor Defined Page)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xA1, 0x01,                    // COLLECTION (Application)
    0x19, 0x00,                    //   USAGE_MINIMUM (0)
    0x29, OUTPUT_REPORT_SIZE,      //   USAGE_MAXIMUM (64)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, OUTPUT_REPORT_SIZE,      //   REPORT_COUNT (64)
    0x91, 0x00,                    //   OUTPUT
    0x19, 0x00,                    //   USAGE_MINIMUM (0)
    0x29, INPUT_REPORT_SIZE,       //   USAGE_MAXIMUM (64)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, INPUT_REPORT_SIZE,       //   REPORT_COUNT (64)
    0x81, 0x00,                    //   INPUT
    0xC0                           // END_COLLECTION
};


/* Handle for emUSB HID instance */
static USB_HID_HANDLE    hInst;

/* Initialization structure for HID interface */
static USB_HID_INIT_DATA_EX InitData;

/* Buffer for store data in OUT direction (Host to Device) */
static U8 OutBuffer[CY_DFU_USB_HID_INT_MAX_PACKET];


/*******************************************************************************
* Internal function declarations
*******************************************************************************/
static void USB_DEV_Start(void);


/*******************************************************************************
* Function Name: USB_DEV_Start
****************************************************************************//**
*
* Starts the USB device operation. It does not wait until USB device is
* enumerated. It invokes USBD_Start().
*
* \globalvars
* \ref USB_DEV_HID_initVar - Used to check the initial configuration, modified on the
*                        first function call.
*
*******************************************************************************/
static void USB_DEV_Start(void)
{
    USB_ADD_EP_INFO EPIntIn;
    USB_ADD_EP_INFO EPIntOut;

    if (!USB_DEV_HID_initVar)
    {
        memset(&EPIntIn, 0x0, sizeof(EPIntIn));
        memset(&EPIntOut, 0x0, sizeof(EPIntOut));

        /* Initializes the USB device with its settings */
        USBD_Init();

        /* IN direction (Device to Host) Int endpoint */
        EPIntIn.Flags           = 0;                             // Flags not used.
        EPIntIn.InDir           = USB_DIR_IN;                    // IN direction (Device to Host)
        EPIntIn.Interval        = 1;                             // Interval of 1 ms in full-speed
        EPIntIn.MaxPacketSize   = CY_DFU_USB_HID_INT_MAX_PACKET; // Maximum packet size (64 for Interrupt).
        EPIntIn.TransferType    = USB_TRANSFER_TYPE_INT;         // Endpoint type - Interrupt.
        InitData.EPIn = USBD_AddEPEx(&EPIntIn, NULL, 0);

        /* OUT direction (Device to Host) Int endpoint */
        EPIntOut.Flags          = 0;                             // Flags not used.
        EPIntOut.InDir          = USB_DIR_OUT;                   // OUT direction (Host to Device)
        EPIntOut.Interval       = 1;                             // Interval of 1 ms in full-speed
        EPIntOut.MaxPacketSize  = CY_DFU_USB_HID_INT_MAX_PACKET; // Maximum packet size (64 for Interrupt).
        EPIntOut.TransferType   = USB_TRANSFER_TYPE_INT;         // Endpoint type - Interrupt.
        InitData.EPOut = USBD_AddEPEx(&EPIntOut, OutBuffer, sizeof(OutBuffer));

        /* Initialization of the HID interface */
        InitData.pReport = HIDReport;
        InitData.NumBytesReport  = sizeof(HIDReport);
        InitData.pInterfaceName  = "DFU HID";

        /* Adds a HID class to the stack */
        hInst = USBD_HID_AddEx(&InitData);

        /* Set data for device enumeration */
        USBD_SetDeviceInfo(&DeviceInfo);

        USB_DEV_HID_initVar = true;
    }
}



/*******************************************************************************
* Function Name: USB_HID_CyBtldrCommStart
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
void USB_HID_CyBtldrCommStart(void)
{
    USB_DEV_Start();

    /* Start the emUSB-Device Core */
    USBD_Start();
}


/*******************************************************************************
* Function Name: USB_HID_CyBtldrCommStop
****************************************************************************//**
*
* Disables the USB device component.
*
*******************************************************************************/
void USB_HID_CyBtldrCommStop(void)
{
    /* Stop the USB communication with detaching Device from the HOST */
    USBD_DeInit();
}


/*******************************************************************************
* Function Name: USB_HID_CyBtldrCommReset
****************************************************************************//**
*
* Resets the receive and transmits communication buffers.
*
*******************************************************************************/
void USB_HID_CyBtldrCommReset(void)
{
    if (USBD_IsConfigured() > 0U)
    {
        /* Cancel any read or write operation */
        USBD_CancelIO(InitData.EPIn);
        USBD_CancelIO(InitData.EPOut);
    }
}


/*******************************************************************************
* Function Name: USB_HID_CyBtldrCommRead
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
cy_en_dfu_status_t USB_HID_CyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t retCode = CY_DFU_ERROR_TIMEOUT;

    CY_ASSERT_L1((pData != NULL) && (size > 0U) && (count != NULL));

    /* Check Device enumeration */
    if ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED)
    {
        /* Wait (blocking with timeout) for data to be available for a read */
        int32_t retVal = USBD_HID_Read(hInst, pData, CY_DFU_USB_HID_INT_MAX_PACKET, timeout);
        int32_t numBytes = CY_DFU_USB_HID_INT_MAX_PACKET;

        /* Data received successfully */
        if (retVal == numBytes)
        {
            *count = numBytes;
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
* Function Name: USB_HID_CyBtldrCommWrite
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
cy_en_dfu_status_t USB_HID_CyBtldrCommWrite(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t retCode = CY_DFU_ERROR_TIMEOUT;

    CY_ASSERT_L1((pData != NULL) && (size > 0U) && (count != NULL));
    CY_ASSERT_L1(size <= CY_DFU_USB_HID_INT_MAX_PACKET);

    /* Check Device enumeration */
    if ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED)
    {
        /* Wait (blocking with timeout, but actual waiting is not expected) for an endpoint availability for a write */
        if (USBD_HID_WaitForTX(hInst, timeout) == 0U)
        {
            /* Write data to the Host (blocking with timeout until send all data) */
            memset(pData + size, 0x0, CY_DFU_USB_HID_INT_MAX_PACKET - size);
            int32_t retVal = USBD_HID_Write(hInst, pData, CY_DFU_USB_HID_INT_MAX_PACKET, timeout);
            int32_t numBytes = CY_DFU_USB_HID_INT_MAX_PACKET;

            /* Data sent successfully */
            if (retVal == numBytes)
            {
                *count = numBytes;
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
