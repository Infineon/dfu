/***************************************************************************//**
* \file transport_usb_cdc.c
* \version 5.1
*
* This file provides the source code of the DFU communication API implementation
* for the USB device that implements a virtual COM port (CDC class).
*
* Note
* This file supports only the ModusToolbox flow.
* For the ModusToolbox flow, the USB device personality alias must be
* "DFU_USB_CDC".
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

#include "cy_sysint.h"
#include "cy_usb_dev_cdc.h"
#include "transport_usb_cdc.h"

/* USER CONFIGURABLE: The USB device COM port used for bootloading */
#define CY_DFU_USB_CDC_PORT                 (0U)

/* USER CONFIGURABLE: The USB device COM port data endpoint maximum packet size */
#define CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET  (64U)

/* Include USBFS driver configuration */
#include "cycfg_peripherals.h"

#ifndef DFU_USB_CDC_HW
    #error The USB device personality alias must be "DFU_USB_CDC" to support DFU communication API.

    /* Dummy configuration to generate only error above during a build */
    #define CY_DFU_USB_HW           NULL
    #define CY_DFU_USB_CFG          NULL
#else

    /* Includes the USB device middleware configuration and USB device descriptors */
    #include "cycfg_usbdev.h"

    /* USER CONFIGURABLE: The pointer to the hardware base address  */
    #define CY_DFU_USB_HW           DFU_USB_CDC_HW

    /* USER CONFIGURABLE: The pointer to the hardware configuration */
    #define CY_DFU_USB_CFG          (&DFU_USB_CDC_config)

    /* USER CONFIGURABLE: The pointer to the USB device middleware configuration */
    #define CY_DFU_USB_DEV_CFG      (&usb_devConfig)

    /* USER CONFIGURABLE: The USB device number used for bootloading */
    #define CY_DFU_USB_DEVICE_NUM   (0U)

    /* USER CONFIGURABLE: The pointer to the USB device middleware device
    * structure that contains USB Descriptors
    */
    #define CY_DFU_USB_DEV_DESCR    (&usb_devices[CY_DFU_USB_DEVICE_NUM])

    /* USER CONFIGURABLE: The USB device middleware CDC class configuration */
    #define CY_DFU_USB_DEV_CDC_CFG  (&usb_cdcConfig)
#endif /* DFU_USB_CDC_HW */

/**
* The instance-specific context structures.
* These structure are used during the USB device operation that supports CDC
* class for internal configuration and data retention. The user must not modify
* anything in this structures.
*/
static cy_stc_usbfs_dev_drv_context_t  USB_drvContext;
static cy_stc_usb_dev_context_t        USB_DEV_devContext;
static cy_stc_usb_dev_cdc_context_t    USB_DEV_CDC_cdcContext;

#define CY_DFU_USB_CONTEXT             USB_drvContext
#define CY_DFU_USB_DEV_CONTEXT         USB_DEV_devContext
#define CY_DFU_USB_DEV_CDC_CONTEXT     USB_DEV_CDC_cdcContext

/**
* USB_DEV_initVar indicates whether the USBFS driver has been initialized. The
* variable is initialized to false and set to true the first time
* \ref USB_CDC_CyBtldrCommStart is called. This allows  the driver to restart
* without re-initialization after the first call to the
* \ref USB_CDC_CyBtldrCommStart routine.
* For re-initialization set \ref USB_DEV_initVar to false and call
* \ref USB_CDC_CyBtldrCommStart.
*/
bool USB_DEV_initVar = false;

/* Waits for 1 millisecond using Cy_SysLib_Delay() */
#define CY_DFU_WAIT_1_MS    (1U)


/***************************************************************************
* Interrupt configuration
***************************************************************************/

#define USB_DEV_INTR_HIGH_SOURCE    (IRQn_Type) usb_interrupt_hi_IRQn
#define USB_DEV_INTR_MED_SOURCE     (IRQn_Type) usb_interrupt_med_IRQn
#define USB_DEV_INTR_LOW_SOURCE     (IRQn_Type) usb_interrupt_lo_IRQn

#define USB_DEV_INTR_HIGH_PRIORITY  (0U)
#define USB_DEV_INTR_MED_PRIORITY   (1U)
#define USB_DEV_INTR_LOW_PRIORITY   (2U)


/*******************************************************************************
* Function Name: USB_DEV_IsrHigh
****************************************************************************//**
*
* The USB device interrupt handler for interrupt sources assigned to the trigger
* high interrupt.
*
*******************************************************************************/
__STATIC_INLINE void USB_DEV_IsrHigh(void)
{
    /* Calls interrupt processing */
    Cy_USBFS_Dev_Drv_Interrupt(CY_DFU_USB_HW,
                               Cy_USBFS_Dev_Drv_GetInterruptCauseHi(CY_DFU_USB_HW),
                               &CY_DFU_USB_CONTEXT);
}


/*******************************************************************************
* Function Name: USB_DEV_IsrMedium
****************************************************************************//**
*
* The USB device interrupt handler for interrupt sources assigned to the trigger
* medium interrupt.
*
*******************************************************************************/
__STATIC_INLINE void USB_DEV_IsrMedium(void)
{
    /* Calls interrupt processing */
    Cy_USBFS_Dev_Drv_Interrupt(CY_DFU_USB_HW,
                               Cy_USBFS_Dev_Drv_GetInterruptCauseMed(CY_DFU_USB_HW),
                               &CY_DFU_USB_CONTEXT);
}


/*******************************************************************************
* Function Name: USB_DEV_IsrLow
****************************************************************************//**
*
* The USB device interrupt handler for interrupt sources assigned to the trigger
* low interrupt.
*
*******************************************************************************/
__STATIC_INLINE void USB_DEV_IsrLow(void)
{
    /* Calls interrupt processing */
    Cy_USBFS_Dev_Drv_Interrupt(CY_DFU_USB_HW,
                               Cy_USBFS_Dev_Drv_GetInterruptCauseLo(CY_DFU_USB_HW),
                               &CY_DFU_USB_CONTEXT);
}


/*******************************************************************************
* Function Name: USB_DEV_Start
****************************************************************************//**
*
* Starts the USB device operation. It does not wait until USB device is
* enumerated. It invokes Cy_USB_Dev_Init, Cy_USB_Dev_CDC_Init and
* Cy_USB_Dev_Connect.
*
* \globalvars
* \ref USB_DEV_initVar - Used to check the initial configuration, modified on the
*                        first function call.
*
*******************************************************************************/
static void USB_DEV_Start(void);
static void USB_DEV_Start(void)
{
    cy_en_usb_dev_status_t status;

    if (false == USB_DEV_initVar)
    {
        static const cy_stc_sysint_t UsbDevIntrHigh =
        {
            .intrSrc      = USB_DEV_INTR_HIGH_SOURCE,
            .intrPriority = USB_DEV_INTR_HIGH_PRIORITY,
        };

        static const cy_stc_sysint_t UsbDevIntrMedium =
        {
            .intrSrc      = USB_DEV_INTR_MED_SOURCE,
            .intrPriority = USB_DEV_INTR_MED_PRIORITY,
        };

        static const cy_stc_sysint_t UsbDevIntrLow =
        {
            .intrSrc      = USB_DEV_INTR_LOW_SOURCE,
            .intrPriority = USB_DEV_INTR_LOW_PRIORITY,
        };

        /* Initializes the USB */
        status = Cy_USB_Dev_Init(CY_DFU_USB_HW, CY_DFU_USB_CFG, &CY_DFU_USB_CONTEXT,
                                 CY_DFU_USB_DEV_DESCR, CY_DFU_USB_DEV_CFG, &CY_DFU_USB_DEV_CONTEXT);

        /* A USB device initialization error - stops the execution */
        CY_ASSERT(CY_USB_DEV_SUCCESS == status);

        status = Cy_USB_Dev_CDC_Init(CY_DFU_USB_DEV_CDC_CFG, &CY_DFU_USB_DEV_CDC_CONTEXT,
                                                             &CY_DFU_USB_DEV_CONTEXT);

        /* A CDC Class initialization error - stops the execution */
        CY_ASSERT(CY_USB_DEV_SUCCESS == status);
        (void) status;

        /* Initializes and enables USB interrupts */
        (void) Cy_SysInt_Init(&UsbDevIntrHigh,   &USB_DEV_IsrHigh);
        (void) Cy_SysInt_Init(&UsbDevIntrMedium, &USB_DEV_IsrMedium);
        (void) Cy_SysInt_Init(&UsbDevIntrLow,    &USB_DEV_IsrLow);

        NVIC_EnableIRQ(UsbDevIntrHigh.intrSrc);
        NVIC_EnableIRQ(UsbDevIntrMedium.intrSrc);
        NVIC_EnableIRQ(UsbDevIntrLow.intrSrc);

        USB_DEV_initVar = true;
    }

    /* Connects the USB device to appear on the bus */
    status = Cy_USB_Dev_Connect(false, CY_USB_DEV_WAIT_FOREVER, &CY_DFU_USB_DEV_CONTEXT);

    /* A USB Device connection error - stops the execution */
    CY_ASSERT(CY_USB_DEV_SUCCESS == status);
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
    Cy_USB_Dev_Disconnect(&CY_DFU_USB_DEV_CONTEXT);
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
    if (Cy_USB_Dev_GetConfiguration(&CY_DFU_USB_DEV_CONTEXT) > 0U)
    {
        uint32_t endpoint;
        cy_en_usb_dev_status_t status;

        /* Aborts CDC endpoints */
        endpoint = (uint32_t) CY_DFU_USB_DEV_CDC_CONTEXT.port[CY_DFU_USB_CDC_PORT].commEp;
        status = Cy_USB_Dev_AbortEpTransfer(endpoint, &CY_DFU_USB_DEV_CONTEXT);
        CY_ASSERT(CY_USB_DEV_SUCCESS == status);

        endpoint = (uint32_t) CY_DFU_USB_DEV_CDC_CONTEXT.port[CY_DFU_USB_CDC_PORT].dataOutEp;
        status = Cy_USB_Dev_AbortEpTransfer(endpoint, &CY_DFU_USB_DEV_CONTEXT);
        CY_ASSERT(CY_USB_DEV_SUCCESS == status);

        endpoint = (uint32_t) CY_DFU_USB_DEV_CDC_CONTEXT.port[CY_DFU_USB_CDC_PORT].dataInEp;
        status = Cy_USB_Dev_AbortEpTransfer(endpoint, &CY_DFU_USB_DEV_CONTEXT);
        CY_ASSERT(CY_USB_DEV_SUCCESS == status);

        /* Enables an OUT endpoint for the following read */
        endpoint = (uint32_t) CY_DFU_USB_DEV_CDC_CONTEXT.port[CY_DFU_USB_CDC_PORT].dataOutEp;
        status = Cy_USB_Dev_StartReadEp(endpoint, &CY_DFU_USB_DEV_CONTEXT);
        CY_ASSERT(CY_USB_DEV_SUCCESS == status);
        (void) status;
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
    (void)size;

    CY_ASSERT_L1((pData != NULL) && (size > 0U) && (count != NULL));

    while (0U != timeout)
    {
        if (Cy_USB_Dev_GetConfiguration(&CY_DFU_USB_DEV_CONTEXT) > 0U)
        {
            /* Waits for data to be available for a read */
            if (Cy_USB_Dev_CDC_IsDataReady(CY_DFU_USB_CDC_PORT, &CY_DFU_USB_DEV_CDC_CONTEXT))
            {
                /* Gets data from the USB buffer */
                *count = Cy_USB_Dev_CDC_GetAll(CY_DFU_USB_CDC_PORT,
                                               pData,
                                               CY_DFU_USB_CDC_ENDPOINT_MAX_PACKET,
                                               &CY_DFU_USB_DEV_CDC_CONTEXT);

                /* Data is expected, otherwise a transport error */
                retCode = (0U != *count) ? CY_DFU_SUCCESS : CY_DFU_ERROR_UNKNOWN;
                break;
            }
        }

        Cy_SysLib_Delay(CY_DFU_WAIT_1_MS);
        --timeout;
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

    while (0U != timeout)
    {
        if (Cy_USB_Dev_GetConfiguration(&CY_DFU_USB_DEV_CONTEXT) > 0U)
        {
            /* Waits for an endpoint available for a write */
            if (Cy_USB_Dev_CDC_IsReady(CY_DFU_USB_CDC_PORT, &CY_DFU_USB_DEV_CDC_CONTEXT))
            {
                cy_en_usb_dev_status_t status;

                /* Transmits data. This function does not wait until data is sent. */
                status = Cy_USB_Dev_CDC_PutData(CY_DFU_USB_CDC_PORT, pData, size, &CY_DFU_USB_DEV_CDC_CONTEXT);

                if (CY_USB_DEV_SUCCESS == status)
                {
                    /* The data put into the buffer to be sent */
                    *count = size;
                    retCode = CY_DFU_SUCCESS;
                }
                else
                {
                    /* A transport error */
                    retCode = CY_DFU_ERROR_UNKNOWN;
                }

                break;
            }
        }

        Cy_SysLib_Delay(CY_DFU_WAIT_1_MS);
        --timeout;
    }

    return (retCode);
}


/* [] END OF FILE */
