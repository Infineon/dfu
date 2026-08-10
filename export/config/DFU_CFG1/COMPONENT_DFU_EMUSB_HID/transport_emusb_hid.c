/***************************************************************************//**
* \file transport_emusb_hid.c
*
* This file provides the source code of the DFU communication API implementation
* for the emUSB-Device that implements a HID class.
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

#include "USB.h"
#include "USB_HID.h"
#include "transport_emusb_hid.h"

#include "stdio.h"

/* MAX size in bytes for HID data packet */
#ifndef CY_DFU_USB_HID_INT_MAX_PACKET
    #if defined (CY_IP_MXUSBFS) || defined (CY_IP_MXS40SUSBFS)
        #define CY_DFU_USB_HID_INT_MAX_PACKET           (USB_FS_INT_MAX_PACKET_SIZE)
    #elif defined (CY_IP_MXS22USBHS)
        #define CY_DFU_USB_HID_INT_MAX_PACKET           (USB_HS_INT_MAX_PACKET_SIZE)
    #endif /* #if defined (CY_IP_MXUSBFS) || defined (CY_IP_MXS40SUSBFS) */
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

/* Data structure for Report descriptor */
static const uint8_t HIDReport[] =
{
    0x06, VENDOR_PAGE_ID, 0xFF,    /* Usage Page (Vendor Defined Page) */
    0x09, 0x01,                    /*  Usage (Vendor Usage 1)          */
    0xA1, 0x01,                    /*  The Collection (Application)    */
    0x19, 0x00,                    /*    Usage Minumum (0)             */
    0x29, OUTPUT_REPORT_SIZE,      /*    Usage Maximum (64)            */
    0x15, 0x00,                    /*    Logical Minimum (0)           */
    0x26, 0xFF, 0x00,              /*    Logical Maximum (255)         */
    0x75, 0x08,                    /*    Report Size (8)               */
    0x95, OUTPUT_REPORT_SIZE,      /*    Report Count (64)             */
    0x91, 0x00,                    /*    Output                        */
    0x19, 0x00,                    /*    Usage Minumum (0)             */
    0x29, INPUT_REPORT_SIZE,       /*    Usage Maximum (64)            */
    0x15, 0x00,                    /*    Logical Minimum (0)           */
    0x26, 0xFF, 0x00,              /*    Logical Maximum (255)         */
    0x75, 0x08,                    /*    Report Size (8)               */
    0x95, INPUT_REPORT_SIZE,       /*    Report Count (64)             */
    0x81, 0x00,                    /*    Input                         */
    0xC0                           /*  END_COLLECTION                  */
};


/* Handle for emUSB HID instance */
static USB_HID_HANDLE    hInst;

/* Initialization structure for HID interface */
static USB_HID_INIT_DATA_EX InitData;

/* Buffer for store data in OUT direction (Host to Device) */
static uint8_t  OutBuffer[CY_DFU_USB_HID_INT_MAX_PACKET];

/* The pointer to the initialization/de-initialization callback function
 * USB hardware
 */
static Cy_DFU_TransportUsbHidCallback usb_callback;


/*******************************************************************************
* Function Name: Cy_DFU_TransportUsbHidConfig
****************************************************************************//**
*
* Configure DFU USB HID Transport
*
* Call this function in the user application to provide the HAL object and callback
* function to DFU transport.
*
* \param config Configuration structure
*
*******************************************************************************/
void Cy_DFU_TransportUsbHidConfig(cy_stc_dfu_transport_usb_hid_cfg_t * config)
{
    CY_ASSERT(NULL != config);
    CY_ASSERT(NULL != config->callback);

    usb_callback = config->callback;
}


/*******************************************************************************
* Function Name: USB_HID_CyBtldrCommStart
****************************************************************************//**
*
* Starts the USB device operation.
*
* \note
* This function configures USB block but the pins/clocks need to be configured
* in User Application or in Device Configurator.
*
*******************************************************************************/
void USB_HID_CyBtldrCommStart(void)
{
    CY_ASSERT(NULL != usb_callback);
    if (NULL != usb_callback)
    {
        usb_callback(CY_DFU_TRANSPORT_USB_HID_INIT);

        USB_ADD_EP_INFO EPIntIn;
        USB_ADD_EP_INFO EPIntOut;

        (void) memset(&EPIntIn, 0x0, sizeof(EPIntIn));
        (void) memset(&EPIntOut, 0x0, sizeof(EPIntOut));

        /* IN direction (Device to Host) Int endpoint */
        EPIntIn.Flags           = 0;                             /* Flags not used.                         */
        EPIntIn.InDir           = USB_DIR_IN;                    /* IN direction (Device to Host)           */
        EPIntIn.Interval        = 1;                             /* Interval of 1 ms in full-speed          */
        EPIntIn.MaxPacketSize   = CY_DFU_USB_HID_INT_MAX_PACKET; /* Maximum packet size (64 for Interrupt). */
        EPIntIn.TransferType    = USB_TRANSFER_TYPE_INT;         /* Endpoint type - Interrupt.              */
        InitData.EPIn = (uint8_t)USBD_AddEPEx(&EPIntIn, NULL, 0);

        /* OUT direction (Device to Host) Int endpoint */
        EPIntOut.Flags          = 0;                             /* Flags not used.                         */
        EPIntOut.InDir          = USB_DIR_OUT;                   /* OUT direction (Host to Device)          */
        EPIntOut.Interval       = 1;                             /* Interval of 1 ms in full-speed          */
        EPIntOut.MaxPacketSize  = CY_DFU_USB_HID_INT_MAX_PACKET; /* Maximum packet size (64 for Interrupt). */
        EPIntOut.TransferType   = USB_TRANSFER_TYPE_INT;         /* Endpoint type - Interrupt.              */
        InitData.EPOut = (uint8_t)USBD_AddEPEx(&EPIntOut, OutBuffer, sizeof(OutBuffer));

        /* Initialization of the HID interface */
        InitData.pReport = HIDReport;
        CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 10.3', 'The size of HIDReport array fit in unsigned 16-bit int size');
        InitData.NumBytesReport  = sizeof(HIDReport);
        InitData.pInterfaceName  = "DFU HID";

        /* Adds a HID class to the stack */
        hInst = USBD_HID_AddEx(&InitData);

        usb_callback(CY_DFU_TRANSPORT_USB_HID_ENABLE);
    }
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
    CY_ASSERT(NULL != usb_callback);
    if (NULL != usb_callback)
    {
        usb_callback(CY_DFU_TRANSPORT_USB_HID_DISABLE);
        usb_callback(CY_DFU_TRANSPORT_USB_HID_DEINIT);
    }
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
    if ((int8_t)USBD_IsConfigured() > 0)
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
    (void) size;

    cy_en_dfu_status_t retCode = CY_DFU_ERROR_TIMEOUT;

    CY_ASSERT_L1((pData != NULL) && (size > 0U) && (count != NULL));

    /* Check Device enumeration */
    if ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED)
    {
        /* Wait (blocking with timeout) for data to be available for a read */
        int32_t retVal = (int32_t)USBD_HID_Read(hInst, pData, CY_DFU_USB_HID_INT_MAX_PACKET, timeout);
        int32_t numBytes = (int32_t)CY_DFU_USB_HID_INT_MAX_PACKET;

        /* Data received successfully */
        if (retVal == numBytes)
        {
            *count = (uint32_t)numBytes;
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
* \param timeout   The time out to wait for before the data is copied to the transmit buffer.
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
        if (USBD_HID_WaitForTX(hInst, timeout) == 0)
        {
            /* Write data to the Host (blocking with timeout until send all data) */
            (void) memset(pData + size, 0x0, CY_DFU_USB_HID_INT_MAX_PACKET - size);
            int32_t retVal = USBD_HID_Write(hInst, pData, CY_DFU_USB_HID_INT_MAX_PACKET, (int32_t)timeout);
            int32_t numBytes = (int32_t)CY_DFU_USB_HID_INT_MAX_PACKET;

            /* Data sent successfully */
            if (retVal == numBytes)
            {
                *count = (uint32_t)numBytes;
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
