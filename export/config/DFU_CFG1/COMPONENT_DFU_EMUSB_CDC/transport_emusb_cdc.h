/***************************************************************************//**
* \file transport_emusb_cdc.h
* \version 6.1.0
*
* This file provides the constants and parameter values of the DFU communication
* API implementation for the emUSB-Device that implements a virtual COM port
* (CDC class).
*
********************************************************************************
* \copyright
* (c) (2016-2025), Cypress Semiconductor Corporation (an Infineon company) or
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

#if !defined(TRANSPORT_EMUSB_CDC_H)
#define TRANSPORT_EMUSB_CDC_H

#include "cy_dfu.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** Execute these actions for USB CDC transport from the user application
 * side.
 *
 * \ref Cy_DFU_TransportUsbCdcCallback
 */
typedef enum
{
    CY_DFU_TRANSPORT_USB_CDC_INIT    = 0x01U, /**< Initialize USB CDC transport */
    CY_DFU_TRANSPORT_USB_CDC_ENABLE  = 0x02U, /**< Enable USB CDC transport */
    CY_DFU_TRANSPORT_USB_CDC_DEINIT  = 0x03U, /**< De-initialize USB CDC transport */
    CY_DFU_TRANSPORT_USB_CDC_DISABLE = 0x04U, /**< Disable USB CDC transport */
} cy_en_dfu_transport_usb_cdc_action_t;

/** The type for the user USB CDC callback to execute some actions. Typically, it is
 * initialization/de-initialization of USB CDC hardware.
 * \ref cy_stc_dfu_transport_usb_cdc_cfg_t
 */
typedef void (*Cy_DFU_TransportUsbCdcCallback) (cy_en_dfu_transport_usb_cdc_action_t action);

/** Configuration structure for DFU USB CDC transport */
typedef struct
{
    Cy_DFU_TransportUsbCdcCallback callback; /**< The pointer to the callback function for
                                             * initialization/de-initialization of USB CDC hardware */
} cy_stc_dfu_transport_usb_cdc_cfg_t;


/***************************************
*        Function Prototypes
***************************************/

/* The USB device CDC class DFU physical layer functions */
void Cy_DFU_TransportUsbCdcConfig(cy_stc_dfu_transport_usb_cdc_cfg_t * config);
void USB_CDC_CyBtldrCommStart(void);
void USB_CDC_CyBtldrCommStop (void);
void USB_CDC_CyBtldrCommReset(void);
cy_en_dfu_status_t USB_CDC_CyBtldrCommRead (uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);
cy_en_dfu_status_t USB_CDC_CyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);

#if defined(__cplusplus)
}
#endif

#endif /* !defined(TRANSPORT_EMUSB_CDC_H) */


/* [] END OF FILE */
