/***************************************************************************//**
* \file transport_ble.h
* \version 4.0
*
* This file provides constants and parameter values of the DFU
* communication APIs for the BLE Component.
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
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
#define CYBLE_BTS_COMMAND_DATA_LEN_OFFSET       (2u)
#define CYBLE_BTS_COMMAND_CONTROL_BYTES_NUM     (7u)
#define CYBLE_BTS_COMMAND_MAX_LENGTH            (265u)


/***************************************
*        Global variables declaration
***************************************/
extern cy_stc_ble_conn_handle_t appConnHandle;

#if defined(__cplusplus)
}
#endif

#endif /* !defined(TRANSPORT_BLE_H) */


/* [] END OF FILE */
