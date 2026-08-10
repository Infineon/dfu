/***************************************************************************//**
* \file host_transport_i2c.h
*
* This file provides constants and parameter values of the DFU host
* communication APIs for the I2C driver.
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

#if !defined(HOST_TRANSPORT_I2C_H)
#define HOST_TRANSPORT_I2C_H

#include "cy_dfu.h"

#if defined(__cplusplus)
extern "C" {
#endif


/***************************************
*    Variables with External Linkage
***************************************/

/** Execute these actions for I2C host transport from the user application side.
 *
 * \ref host_I2cCallback
 */
typedef enum
{
    HOST_I2C_INIT   = 0x01U, /**< Initialize and enable I2C host transport */
    HOST_I2C_DEINIT = 0x02U, /**< De-initialize and disable I2C host transport */
} host_i2c_action_t;

/** The type for the user I2C host callback to execute some actions. Typically, it is
 * initialization/de-initialization of I2C host hardware.
 * \ref host_i2c_cfg_t
 */
typedef void (*host_I2cCallback) (host_i2c_action_t action);

/** Configuration structure for DFU host I2C transport */
typedef struct
{
    mtb_hal_i2c_t   *i2cObj;   /**< The pointer to the HAL I2C host object */
    host_I2cCallback callback; /**< The pointer to the callback function for
                                * initialization/de-initialization of I2C host hardware */
    uint16_t         i2cAddr;  /**< The I2C address of the companion device */
} host_i2c_cfg_t;


/***************************************
*        Function Prototypes
***************************************/
void Host_I2cConfig(host_i2c_cfg_t * config);
void Host_I2cStart(void);
void Host_I2cStop (void);
void Host_I2cReset(void);
cy_en_dfu_status_t Host_I2cRead (uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout);
cy_en_dfu_status_t Host_I2cWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut);

#if defined(__cplusplus)
}
#endif

#endif /* !defined(HOST_TRANSPORT_I2C_H) */


/* [] END OF FILE */
