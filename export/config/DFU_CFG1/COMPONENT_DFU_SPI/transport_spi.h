/***************************************************************************//**
* \file transport_spi.h
*
* This file provides constants and parameter values of the DFU
* communication APIs for the SPI driver.
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

#if !defined(TRANSPORT_SPI_H)
#define TRANSPORT_SPI_H

#include "cy_dfu.h"
#include "mtb_hal_spi.h"

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
/** Execute these actions for SPI transport from the user application
 * side.
 *
 * \ref Cy_DFU_TransportSpiCallback
 */
typedef enum
{
    CY_DFU_TRANSPORT_SPI_INIT   = 0x01U, /**< Initialize and enable SPI transport */
    CY_DFU_TRANSPORT_SPI_DEINIT = 0x02U, /**< De-initialize and disable SPI transport */
} cy_en_dfu_transport_spi_action_t;

/** The type for the user SPI callback to execute some actions. Typically, it is
 * initialization/de-initialization of SPI hardware.
 * \ref cy_stc_dfu_transport_spi_cfg_t
 */
typedef void (*Cy_DFU_TransportSpiCallback) (cy_en_dfu_transport_spi_action_t action);

/** Configuration structure for DFU SPI transport */
typedef struct
{
    mtb_hal_spi_t               *spi;    /**< The pointer to the HAL SPI object */
    Cy_DFU_TransportSpiCallback callback; /**< The pointer to the callback function for
                                             * initialization/de-initialization of SPI hardware */
} cy_stc_dfu_transport_spi_cfg_t;


/***************************************
*        Function Prototypes
***************************************/

/* SPI DFU physical layer functions */
void Cy_DFU_TransportSpiConfig(cy_stc_dfu_transport_spi_cfg_t * config);
/** \} group_dfu_functions_transport */

void SPI_SpiCyBtldrCommStart(void);
void SPI_SpiCyBtldrCommStop (void);
void SPI_SpiCyBtldrCommReset(void);
cy_en_dfu_status_t SPI_SpiCyBtldrCommRead (uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);
cy_en_dfu_status_t SPI_SpiCyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);

#if defined(__cplusplus)
}
#endif

#endif /* !defined(TRANSPORT_SPI_H) */


/* [] END OF FILE */
