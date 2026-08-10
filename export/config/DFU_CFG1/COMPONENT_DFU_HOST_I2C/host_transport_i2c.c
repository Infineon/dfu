/***************************************************************************//**
* \file host_transport_i2c.c
*
* This file provides the source code of the DFU host communication APIs
* for the I2C driver from HAL.
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

#include "mtb_hal.h"
#include "host_transport_i2c.h"


/*******************************************************************************
* Internal variables
*******************************************************************************/
/* The I2C HAL object for DFU host */
static mtb_hal_i2c_t *host_i2cObj;

/* The pointer to the initialization/de-initialization callback function
 * I2C host hardware
 */
static host_I2cCallback host_i2cCallback;

/* The I2C address of the companion device */
static uint16_t companion_i2cAddr;


/*******************************************************************************
* Function Name: Host_I2cConfig
****************************************************************************//**
*
* Configure the DFU host I2C transport.
*
* \param config Configuration structure.
*
*******************************************************************************/
void Host_I2cConfig(host_i2c_cfg_t * config)
{
    CY_ASSERT(NULL != config);
    CY_ASSERT(NULL != config->i2cObj);
    CY_ASSERT(NULL != config->callback);

    host_i2cObj       = config->i2cObj;
    host_i2cCallback  = config->callback;
    companion_i2cAddr = config->i2cAddr;
}


/*******************************************************************************
* Function Name: Host_I2cStart
****************************************************************************//**
*
* Starts the I2C host transport for a companion device.
*
*******************************************************************************/
void Host_I2cStart(void)
{
    CY_ASSERT(NULL != host_i2cCallback);

    if (NULL != host_i2cCallback)
    {
        host_i2cCallback(HOST_I2C_INIT);
    }
}


/*******************************************************************************
* Function Name: Host_I2cStop
****************************************************************************//**
*
*  Stops the I2C host transport for a companion device.
*
*******************************************************************************/
void Host_I2cStop(void)
{
    CY_ASSERT(NULL != host_i2cCallback);

    if (NULL != host_i2cCallback)
    {
        host_i2cCallback(HOST_I2C_DEINIT);
    }
}


/*******************************************************************************
* Function Name: Host_I2cReset
****************************************************************************//**
*
*  De-init a DFU host device.
*
*******************************************************************************/
void Host_I2cReset(void)
{
    companion_i2cAddr = 0U;
}


/*******************************************************************************
* Function Name: Host_I2cRead
****************************************************************************//**
*  This function is intended for use with a companion device.
*
*  Allows the caller to read data from the companion device.
*
*  \param pData: Pointer to storage for the block of data to be read from
*   a companion device.
*  \param size: Number of bytes to be read.
*  \param count: Pointer to the variable to write the number of bytes actually
*   read.
*  \param timeOut The amount of time (in milliseconds) for which the
*                function should wait before indicating communication
*                time out.
*
*  \return
*   Returns CYRET_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   "Return Codes" section of the System Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t Host_I2cRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;

    if ((pData != NULL) && (size > 0U))
    {
        status = CY_DFU_ERROR_TIMEOUT;

        if (CY_RSLT_SUCCESS == mtb_hal_i2c_controller_read(host_i2cObj,
                                                           companion_i2cAddr,
                                                           pData,
                                                           size,
                                                           timeout,
                                                           false))
        {
            *count = size;
            status = CY_DFU_SUCCESS;
        }
    }

    return (status);
}


/*******************************************************************************
* Function Name: Host_I2cWrite
****************************************************************************//**
*  This function is intended for use with a companion device.
*
*  Allows the caller to write data to the companion device.
*
*  \param pData: Pointer to the block of data to be written to the companion device.
*
*  \param size: Number of bytes to be written.
*  \param count: Pointer to the variable to write the number of bytes actually
*   written.
*  \param timeOut: The timeout is not used by this function.
*
*  \return
*   Returns CYRET_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   "Return Codes" section of the System Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t Host_I2cWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;

    if ((NULL != pData) && (size > 0U))
    {
        status = CY_DFU_ERROR_TIMEOUT;

        if (CY_RSLT_SUCCESS == mtb_hal_i2c_controller_write(host_i2cObj,
                                                            companion_i2cAddr,
                                                            pData,
                                                            size,
                                                            timeOut,
                                                            false))
        {
            *count = size;
            status = CY_DFU_SUCCESS;
        }
    }

    return (status);
}

/* [] END OF FILE */
