/***************************************************************************//**
* \file transport_i2c.c
* \version 4.0
*
* This file provides the source code of the DFU communication APIs
* for the SCB Component I2C mode.
*
* Note
* This file supports the PSoC Creator and ModusToolbox flows.
* For the PSoC Creator flow, the default I2C component instance name is 
* "I2C". For the ModusToolbox flow, the I2C personality alias must be 
* "DFU_I2C".
* This file serves as a template and can be modified defining any component
* name or personality alias.
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "transport_i2c.h"
#include "cy_scb_i2c.h"
#include "cy_sysint.h"
#include <string.h>

#if defined(CY_PSOC_CREATOR_USED)
    
    /* USER CONFIGURABLE: Change the header file based on the I2C component instance name */
    #include "I2C.h"
    
    /* USER CONFIGURABLE: Instance name of the I2C component */
    #define CY_DFU_I2C_INSTANCE     I2C
    
    #define JOIN_LEVEL2(a, b)       a ## b
    #define JOIN_LEVEL1(a, b)       JOIN_LEVEL2(a, b)
    #define CY_DFU_I2C_HW           JOIN_LEVEL1(CY_DFU_I2C_INSTANCE, _SCB__HW)
    #define I2C_API(fn)             JOIN_LEVEL1(CY_DFU_I2C_INSTANCE, fn)
    #define CY_DFU_I2C_CONTEXT      JOIN_LEVEL1(CY_DFU_I2C_INSTANCE, _context)

#else /* ModusToolbox is used for the DFU transport configuration */

/* Includes driver configuration */
#include "cycfg_peripherals.h"

#if !defined DFU_I2C_HW
    #error The I2C personality alias must be DFU_I2C to support DFU communication API.

    /* Dummy configuration to generate only error above during a build */
    #define CY_DFU_I2C_HW           NULL
    #define CY_DFU_I2C_CFG_PTR      NULL
    #define I2C_INTR_SOURCE         (0U)

#else

    /* USER CONFIGURABLE: the pointer to the base address of the hardware */
    #define CY_DFU_I2C_HW           DFU_I2C_HW

    /* USER CONFIGURABLE: the pointer to the configuration */
    #define CY_DFU_I2C_CFG_PTR      (&DFU_I2C_config)

    /* USER CONFIGURABLE: Interrupt configuration for the I2C block. */
    #define I2C_INTR_SOURCE         DFU_I2C_IRQ

#endif /* !defined DFU_I2C_HW */

/** The instance-specific context structure.
* It is used while the driver operation for internal configuration and
* data keeping for the I2C. The user should not modify anything in this
* structure.
*/
static cy_stc_scb_i2c_context_t     I2C_context;
#define CY_DFU_I2C_CONTEXT          I2C_context

/**
* I2C_initVar indicates whether the I2C driver has been initialized. The 
* variable is initialized to false and set to true the first time 
* \ref I2C_I2cCyBtldrCommStart is called. This allows  the driver to restart 
* without re-initialization after the first call to the 
* \ref I2C_I2cCyBtldrCommStart routine.
* For re-initialization set \ref I2C_initVar to false and call 
* \ref I2C_I2cCyBtldrCommStart.
* Note that PSoC Creator I2C component uses its own initVar variable.
*/
bool I2C_initVar = false;

/* Timeout unit in millisecond */
#define I2C_WAIT_1_MS  (1u)


/*******************************************************************************
* Interrupt configuration
*******************************************************************************/
                                              
/** Interrupt priority for Cortex-M4. Valid range: 0 to 7. */          
 #define I2C_INTR_PRIORITY               (7U) 


/*******************************************************************************
* I2C transport buffers
*******************************************************************************/

/* Size of Read/Write buffers for I2C DFU  */
#define I2C_BTLDR_SIZEOF_TX_BUFFER   (64u)
#define I2C_BTLDR_SIZEOF_RX_BUFFER   (64u)

/* Writes to this buffer */
static uint8_t I2C_slaveTxBuf[I2C_BTLDR_SIZEOF_TX_BUFFER];

/* Reads from this buffer */
static uint8_t I2C_slaveRxBuf[I2C_BTLDR_SIZEOF_RX_BUFFER];

/* Flag to release buffer to be read */
static uint32_t I2C_applyBuffer;

/* Callback to insert the response on a read request */
static void I2C_I2CResposeInsert(uint32_t event);

/* Return number of bytes to copy into DFU buffer */
#define I2C_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32_t) (actBufSize) < (uint32_t) (bufSize)) ? \
                                ((uint32_t) (actBufSize)) : ((uint32_t) (bufSize)) )


/*******************************************************************************
* Function Name: I2C_Interrupt
****************************************************************************//**
*
* The SCB I2C driver interrupt handler.
*
*******************************************************************************/
__STATIC_INLINE void I2C_Interrupt(void)
{
    Cy_SCB_I2C_Interrupt(CY_DFU_I2C_HW, &CY_DFU_I2C_CONTEXT);
}


/*******************************************************************************
* Function Name: I2C_Start
****************************************************************************//**
*
* Starts SCB I2C operation. Setup interrupt. 
*
* \globalvars
* \ref I2C_initVar - used to check initial configuration, modified on first
*                    function call.
*
*******************************************************************************/
static void I2C_Start(void);
static void I2C_Start(void)
{
    if (false == I2C_initVar)
    {
        cy_en_scb_i2c_status_t status;

        /* Interrupt configuration structure */
        static const cy_stc_sysint_t I2C_SCB_IRQ_cfg = 
        {
            .intrSrc      = (IRQn_Type) I2C_INTR_SOURCE,
            .intrPriority = I2C_INTR_PRIORITY
        };

        /* Configure the I2C block */
        status = Cy_SCB_I2C_Init(CY_DFU_I2C_HW, CY_DFU_I2C_CFG_PTR, &CY_DFU_I2C_CONTEXT);
        
        /* A I2C initialization error - stops the execution */
        CY_ASSERT(CY_SCB_I2C_SUCCESS == status);
        (void) status;

        (void) Cy_SysInt_Init(&I2C_SCB_IRQ_cfg, &I2C_Interrupt);
        NVIC_EnableIRQ((IRQn_Type) I2C_SCB_IRQ_cfg.intrSrc);
        
        /* Component is configured */
        I2C_initVar = true;
    }

    Cy_SCB_I2C_Enable(CY_DFU_I2C_HW);
}

#endif  /* #if defined(CY_PSOC_CREATOR_USED) */


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommStart
****************************************************************************//**
*
* Starts the I2C component.
*
* This function does not configure an infrastructure required for the SCB I2C
* operation: clocks and pins. For the PSoC Creator and ModusToolbox flows, the
* generated files configure clocks and pins. This configuration must be
* performed by the application when the project uses only PDL.
*
*******************************************************************************/
void I2C_I2cCyBtldrCommStart(void)
{
    #if defined(CY_PSOC_CREATOR_USED)
        I2C_API(_Start)();
    #else
        I2C_Start();
    #endif /* CY_PSOC_CREATOR_USED */
       
    Cy_SCB_I2C_SlaveConfigReadBuf(CY_DFU_I2C_HW, I2C_slaveTxBuf, 0u, &CY_DFU_I2C_CONTEXT);
    Cy_SCB_I2C_SlaveConfigWriteBuf(CY_DFU_I2C_HW, I2C_slaveRxBuf, I2C_BTLDR_SIZEOF_RX_BUFFER, &CY_DFU_I2C_CONTEXT);
    Cy_SCB_I2C_RegisterEventCallback(CY_DFU_I2C_HW, &I2C_I2CResposeInsert, &CY_DFU_I2C_CONTEXT);
    I2C_applyBuffer = 0u;
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommStop
****************************************************************************//**
*
*  Disables the I2C component.
*
*******************************************************************************/
void I2C_I2cCyBtldrCommStop(void)
{
    Cy_SCB_I2C_Disable(CY_DFU_I2C_HW, &CY_DFU_I2C_CONTEXT);
    Cy_SCB_I2C_DeInit(CY_DFU_I2C_HW);
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommReset
****************************************************************************//**
*
*  Resets the receive and transmit communication buffers and the slave status.
*
*******************************************************************************/
void I2C_I2cCyBtldrCommReset(void)
{
    Cy_SCB_ClearTxFifo(CY_DFU_I2C_HW);
    Cy_SCB_ClearRxFifo(CY_DFU_I2C_HW);
    
    Cy_SCB_I2C_SlaveConfigReadBuf(CY_DFU_I2C_HW, I2C_slaveTxBuf, 0u, &CY_DFU_I2C_CONTEXT);
    Cy_SCB_I2C_SlaveConfigWriteBuf(CY_DFU_I2C_HW, I2C_slaveRxBuf, I2C_BTLDR_SIZEOF_RX_BUFFER, &CY_DFU_I2C_CONTEXT);
    
    (void)Cy_SCB_I2C_SlaveClearReadStatus(CY_DFU_I2C_HW, &CY_DFU_I2C_CONTEXT);
    (void)Cy_SCB_I2C_SlaveClearWriteStatus(CY_DFU_I2C_HW, &CY_DFU_I2C_CONTEXT);
    
    I2C_applyBuffer = 0u;
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommRead
****************************************************************************//**
*
*  Allows the caller to read data from the DFU host (the host writes the
*  data). The function handles polling to allow a block of data to be completely
*  received from the host device.
*
*  \param pData: Pointer to storage for the block of data to be read from the
*   DFU host
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
cy_en_dfu_status_t I2C_I2cCyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0u))
    {
        status = CY_DFU_ERROR_TIMEOUT;
 
        while (0u != timeout)
        {
            /* Check if host complete write */
            if (0u != (Cy_SCB_I2C_SlaveGetStatus(CY_DFU_I2C_HW, &CY_DFU_I2C_CONTEXT) & CY_SCB_I2C_SLAVE_WR_CMPLT))
            {
                /* Get number of received bytes */
                *count = I2C_BYTES_TO_COPY(Cy_SCB_I2C_SlaveGetWriteTransferCount(CY_DFU_I2C_HW, &CY_DFU_I2C_CONTEXT), size);

                /* Clear slave status */
                (void)Cy_SCB_I2C_SlaveClearWriteStatus(CY_DFU_I2C_HW, &CY_DFU_I2C_CONTEXT);

                /* Copy command into DFU buffer */
                (void) memcpy((void *) pData, (const void *) I2C_slaveRxBuf, *count);
                
                /* Prepare the slave buffer for next reception */
                Cy_SCB_I2C_SlaveConfigWriteBuf(CY_DFU_I2C_HW, I2C_slaveRxBuf, I2C_BTLDR_SIZEOF_RX_BUFFER, &CY_DFU_I2C_CONTEXT);
                status = CY_DFU_SUCCESS;
                break;
            }

            CyDelay(I2C_WAIT_1_MS);
            --timeout;
        }
    }

    return (status);
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommWrite
****************************************************************************//**
*
*  Allows the caller to write data to the DFU host (the host reads the
*  data). The function does not use timeout and returns after data has been
*  copied into the transmit buffer. The data transmission starts immediately
*  after the first data element is written into the buffer and lasts until all
*  data elements from the buffer are sent.
*
*  \param pData: Pointer to the block of data to be written to the DFU
*   host.
*  \param size: Number of bytes to be written.
*  \param count: Pointer to the variable to write the number of bytes actually
*   written.
*  \param timeOut: The timeout is not used by this function.
*   The function returns as soon as data is copied into the transmit buffer.
*
*  \return
*   Returns CYRET_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   "Return Codes" section of the System Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t I2C_I2cCyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;
    
    if ((NULL != pData) && (size > 0u))
    {
        /* Copy response into read buffer */
        *count = size;
        (void) memcpy((void *) I2C_slaveTxBuf, (const void *) pData, (uint32_t) size);

        /* Read buffer is ready to be released to host */
        I2C_applyBuffer = (uint32_t) size;

        if (0u != timeOut)
        {
            /* Suppress compiler warning */
        }

        status = CY_DFU_SUCCESS;
    }

    return (status);
}


/*******************************************************************************
* Function Name: I2C_I2CResposeInsert
****************************************************************************//**
*
*  Releases the read buffer to be read when a response is copied to the buffer
*  and a new read transaction starts.
*  Closes the read buffer when write transaction is started.
*
* \globalvars
*  I2C_applyBuffer - the flag to release the buffer with a response
*  to be read by the host.
*
*******************************************************************************/
static void I2C_I2CResposeInsert(uint32_t event)
{
    if ((CY_SCB_I2C_SLAVE_READ_EVENT == event) && (0u != I2C_applyBuffer))
    {
        /* Address phase, host reads: release read buffer */
        (void)Cy_SCB_I2C_SlaveClearReadStatus(CY_DFU_I2C_HW, &CY_DFU_I2C_CONTEXT);
        Cy_SCB_I2C_SlaveConfigReadBuf(CY_DFU_I2C_HW, I2C_slaveTxBuf, I2C_applyBuffer, &CY_DFU_I2C_CONTEXT);
        I2C_applyBuffer  = 0u;
    }
    else if(CY_SCB_I2C_SLAVE_WRITE_EVENT == event)
    {
        /* Address phase, host writes: make read buffer empty so that host will
         * receive only 0xFF (CY_SCB_I2C_DEFAULT_TX) until the DFU
         * application has a valid response packet.
         */
        Cy_SCB_I2C_SlaveConfigReadBuf(CY_DFU_I2C_HW, I2C_slaveTxBuf, 0u, &CY_DFU_I2C_CONTEXT);
    }
    else
    {
        /* No action */
    }
}


#ifndef CY_DFU_I2C_TRANSPORT_DISABLE

/*******************************************************************************
* Function Name: Cy_DFU_TransportStart
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportStart(void)
{
    I2C_I2cCyBtldrCommStart();
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportStop
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportStop(void)
{
    I2C_I2cCyBtldrCommStop();
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportReset
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportReset(void)
{
    I2C_I2cCyBtldrCommReset();
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportRead
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_TransportRead(uint8_t *buffer, uint32_t size, uint32_t *count, uint32_t timeout)
{
    return (I2C_I2cCyBtldrCommRead(buffer, size, count, timeout));
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportWrite
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_TransportWrite(uint8_t *buffer, uint32_t size, uint32_t *count, uint32_t timeout)
{
    return (I2C_I2cCyBtldrCommWrite(buffer, size, count, timeout));
}

#endif /* CY_DFU_I2C_TRANSPORT_DISABLE */


/* [] END OF FILE */
