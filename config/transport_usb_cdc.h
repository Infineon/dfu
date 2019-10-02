/***************************************************************************//**
* \file transport_usb_cdc.h
* \version 4.0
*
* This file provides the constants and parameter values of the DFU communication 
* API implementation for the USB device that implements a virtual COM port 
* (CDC class).
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(TRANSPORT_USB_CDC_H)
#define TRANSPORT_USB_CDC_H

#include "cy_dfu.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*    Variables with External Linkage
***************************************/

extern bool USB_DEV_initVar;


/***************************************
*        Function Prototypes
***************************************/

/* The USB device CDC class DFU physical layer functions */
void USB_CDC_CyBtldrCommStart(void);
void USB_CDC_CyBtldrCommStop (void);
void USB_CDC_CyBtldrCommReset(void);
cy_en_dfu_status_t USB_CDC_CyBtldrCommRead (uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);
cy_en_dfu_status_t USB_CDC_CyBtldrCommWrite(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);

#if defined(__cplusplus)
}
#endif

#endif /* !defined(TRANSPORT_USB_CDC_H) */


/* [] END OF FILE */
