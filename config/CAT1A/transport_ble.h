/***************************************************************************//**
* \file transport_ble.h
* \version 4.20
*
* This file provides constants and parameter values of the DFU
* communication APIs for the BLE Component.
*
********************************************************************************
* \copyright
* (c) (2016-2021), Cypress Semiconductor Corporation (an Infineon company) or
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

#if !defined(TRANSPORT_BLE_H)
#define TRANSPORT_BLE_H

#include <stdint.h>
#include "cycfg_ble.h"
#include "cy_dfu.h"
#include "cy_ble.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*        Function Prototypes
***************************************/

/* BLE DFU physical layer functions */
void CyBLE_CyBtldrCommStart(void);
void CyBLE_CyBtldrCommStop (void);
void CyBLE_CyBtldrCommReset(void);
cy_en_dfu_status_t CyBLE_CyBtldrCommRead (uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);
cy_en_dfu_status_t CyBLE_CyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);
void DFUCallBack(uint32 event, void* eventParam);

/* BLE Callback */
extern void AppCallBack(uint32 event, void* eventParam);

/***************************************
*        API Constants
***************************************/
#define CYBLE_BTS_COMMAND_DATA_LEN_OFFSET       (2U)
#define CYBLE_BTS_COMMAND_CONTROL_BYTES_NUM     (7U)
#define CYBLE_BTS_COMMAND_MAX_LENGTH            (265U)


/***************************************
*        Global variables declaration
***************************************/
extern cy_stc_ble_conn_handle_t appConnHandle;

#if defined(__cplusplus)
}
#endif

#endif /* !defined(TRANSPORT_BLE_H) */


/* [] END OF FILE */
