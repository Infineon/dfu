/***************************************************************************//**
* \file transport_pmbus.h
*
* This file provides constants and parameter values of the DFU
* communication APIs for the PMBUS driver.
*
********************************************************************************
* \copyright
* (c) (2016-2026), Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
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

#if !defined(TRANSPORT_PMBUS_H)
#define TRANSPORT_PMBUS_H

#include "cy_dfu.h"
#include "mtb_pmbus.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*    Variables with External Linkage
***************************************/

/**
* \addtogroup group_dfu_functions_transport
* \{
*/
/** Configuration structure for DFU PMBUS transport */
typedef struct
{
    mtb_pmbus_stc_t *pmbus;      /**< The pointer to the PMBUS object */
    uint32_t         cmdCode;    /**< The PMBUS command used for DFU */
    uint8_t         *cmdData;    /**< The pointer to the PMBUS command data storage */
} cy_stc_dfu_transport_pmbus_cfg_t;


/***************************************
*        Function Prototypes
***************************************/

/* PMBUS DFU physical layer functions */
void Cy_DFU_TransportPMBusConfig(cy_stc_dfu_transport_pmbus_cfg_t * config);
/** \} group_dfu_functions_transport */
void PMBUS_CyBtldrCommStart(void);
void PMBUS_CyBtldrCommStop (void);
void PMBUS_CyBtldrCommReset(void);
cy_en_dfu_status_t PMBUS_CyBtldrCommRead (uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut);
cy_en_dfu_status_t PMBUS_CyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut);
bool dfu_pmbus_cmd_callback(mtb_pmbus_cmd_events_t event, int32_t page, int32_t phase, uint8_t byte);

#if defined(__cplusplus)
}
#endif

#endif /* !defined(TRANSPORT_PMBUS_H) */

/* [] END OF FILE */
