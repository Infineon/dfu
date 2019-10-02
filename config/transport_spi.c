/***************************************************************************//**
* \file transport_spi.c
* \version 4.0
*
* This file provides the source code of the DFU communication APIs
* for the SCB Component SPI mode.
*
* Note
* This file supports the PSoC Creator and ModusToolbox flows.
* For the PSoC Creator flow, the default SPI component instance name is 
* "SPI". For the ModusToolbox flow, the SPI personality alias must be 
* "DFU_SPI".
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

#include "transport_spi.h"
#include "cy_scb_spi.h"

#if defined(CY_PSOC_CREATOR_USED)
    
    /* USER CONFIGURABLE: Change the header file based on the I2C component instance name */
    #include "SPI.h"
    
    /* USER CONFIGURABLE: Instance name of the I2C component */
    #define CY_DFU_SPI_INSTANCE     SPI
                                                 
    #define JOIN_LEVEL2(a, b)       a ## b
    #define JOIN_LEVEL1(a, b)       JOIN_LEVEL2(a, b)
    #define CY_DFU_SPI_HW           JOIN_LEVEL1(CY_DFU_SPI_INSTANCE, _SCB__HW)
    #define SPI_API(fn)             JOIN_LEVEL1(CY_DFU_SPI_INSTANCE, fn)
#else

/* Includes driver configuration */
#include "cycfg_peripherals.h"

#if !defined DFU_SPI_HW
    #error The SPI personality alias must be DFU_SPI to support DFU communication API.

    /* Dummy configuration to generate only error above during a build */
    #define CY_DFU_SPI_HW           NULL
    #define CY_DFU_SPI_CFG_PTR      NULL

#else

    /* USER CONFIGURABLE: the pointer to the base address of the hardware */
    #define CY_DFU_SPI_HW           DFU_SPI_HW

    /* USER CONFIGURABLE: the pointer to the driver configuration */
    #define  CY_DFU_SPI_CFG_PTR     (&DFU_SPI_config)

#endif /* !defined DFU_SPI_HW */  
    
/* USER CONFIGURABLE: The slave select line constant. Update it based on the
* pin selected for slave select.
*/
#define CY_SPI_SLAVE_SELECT     CY_SCB_SPI_SLAVE_SELECT1

/**
* SPI_initVar indicates whether the SPI driver has been initialized. The 
* variable is initialized to false and set to true the first time 
* \ref SPI_SpiCyBtldrCommStart is called. This allows  the driver to restart 
* without re-initialization after the first call to the 
* \ref SPI_SpiCyBtldrCommStart routine.
* For re-initialization set \ref SPI_initVar to false and call 
* \ref SPI_SpiCyBtldrCommStart.
* Note that PSoC Creator SPI component uses its own initVar variable.
*/
bool SPI_initVar = false;


/*******************************************************************************
* Function Name: SPI_Start
****************************************************************************//**
*
* Starts SCB SPI operation.
*
* \globalvars
* \ref SPI_initVar - used to check initial configuration, modified on first
*                    function call.
*
*******************************************************************************/
static void SPI_Start(void);
static void SPI_Start(void)
{
    if (false == SPI_initVar)
    {
        cy_en_scb_spi_status_t status;

        /* Configure component */
        status = Cy_SCB_SPI_Init(CY_DFU_SPI_HW, CY_DFU_SPI_CFG_PTR, NULL);
        
        /* A SPI initialization error - stops the execution */
        CY_ASSERT(CY_SCB_SPI_SUCCESS == status);
        (void) status;        

        /* Set active slave select to line 0 */
        Cy_SCB_SPI_SetActiveSlaveSelect(CY_DFU_SPI_HW, CY_SPI_SLAVE_SELECT);
        
        /* Component is configured */
        SPI_initVar = true;
    }
    
    Cy_SCB_SPI_Enable(CY_DFU_SPI_HW);
}

#endif  /* #if defined(CY_PSOC_CREATOR_USED) */    

/* Byte to byte time interval in microseconds. Slave waits for this amount of
 * time between checking whether more data is being received. Slave starts
 * processing the packet if the number of received bytes remain same between
 * two successive checks. Change this depending on the expected inter-byte delay
 * for a given SPI master. 
 */
#define SPI_SPI_BYTE_TO_BYTE   (32u)

/* Timeout unit in millisecond */
#define SPI_WAIT_1_MS           (1u)

/* Return number of bytes to copy into DFU buffer */
#define SPI_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32)(actBufSize) < (uint32)(bufSize)) ? \
                                ((uint32) (actBufSize)) : ((uint32) (bufSize)) )                            
                            
                                
/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommStart
****************************************************************************//**
*
*  Starts the SPI component.
*
* \note
*  This function does not configure an infrastructure required for the SCB SPI
*  operation: clocks and pins. For the PSoC Creator and ModusToolbox flows, the
*  generated files configure clocks and pins. This configuration must be
*  performed by the application when the project uses only PDL.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommStart(void)
{
    #if defined(CY_PSOC_CREATOR_USED)
        SPI_API(_Start)();
    #else
        SPI_Start();
    #endif /* #if defined(CY_PSOC_CREATOR_USED) */
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommStop
****************************************************************************//**
*
*  Disables the SPI component.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommStop(void)
{
    Cy_SCB_SPI_Disable(CY_DFU_SPI_HW, NULL);
    Cy_SCB_SPI_DeInit(CY_DFU_SPI_HW);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommReset
****************************************************************************//**
*
*  Resets the receive and transmit communication buffers and the slave status.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommReset(void)
{
    Cy_SCB_SPI_ClearTxFifo(CY_DFU_SPI_HW);
    Cy_SCB_SPI_ClearRxFifo(CY_DFU_SPI_HW);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommRead
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
cy_en_dfu_status_t SPI_SpiCyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t statusLoc = CY_DFU_ERROR_UNKNOWN;
    uint32_t byteCount;

    if ((pData != NULL) && (size > 0u))
    {
        statusLoc = CY_DFU_ERROR_TIMEOUT;
        
        /* Wait with timeout 1mS for packet start */
        byteCount = 0u;
 
        do
        {
            /* Check packet start */
            if (0u != Cy_SCB_GetNumInRxFifo(CY_DFU_SPI_HW))
            {
                /* Wait for packet end */
                do
                {
                    byteCount = Cy_SCB_GetNumInRxFifo(CY_DFU_SPI_HW);
                    CyDelayUs(SPI_SPI_BYTE_TO_BYTE);
                }
                while (byteCount != Cy_SCB_GetNumInRxFifo(CY_DFU_SPI_HW));

                /* Disable data reception into RX FIFO */
                CY_DFU_SPI_HW->RX_FIFO_CTRL |= SCB_RX_FIFO_CTRL_FREEZE_Msk;

                byteCount = SPI_BYTES_TO_COPY(byteCount, size);
                *count = byteCount;
                
                /* Get data from RX buffer into DFU buffer */
                (void) Cy_SCB_SPI_ReadArray(CY_DFU_SPI_HW, (void*)pData, byteCount);
               
                statusLoc = CY_DFU_SUCCESS;
                break;
            }

            CyDelay(SPI_WAIT_1_MS);
            --timeout;
        }
        while (0u != timeout);
    }

    return (statusLoc);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommWrite
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
cy_en_dfu_status_t SPI_SpiCyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t statusLoc = CY_DFU_ERROR_UNKNOWN;

    if ((NULL != pData) && (size > 0u))
    {
        statusLoc = CY_DFU_ERROR_TIMEOUT;

        Cy_SCB_SPI_ClearTxFifo(CY_DFU_SPI_HW);
        
        /* Put data into TX buffer */
        Cy_SCB_SPI_WriteArrayBlocking(CY_DFU_SPI_HW, (void *)pData, size);

        /* Wait with timeout 1mS for packet end */
        do
        {
            /* Check for packet end */
            uint32_t numInTxFifo = Cy_SCB_SPI_GetNumInTxFifo(CY_DFU_SPI_HW); 
            uint32_t txSrValid = Cy_SCB_GetTxSrValid(CY_DFU_SPI_HW);
            if ((!Cy_SCB_SPI_IsBusBusy(CY_DFU_SPI_HW)) &&
                (0u == numInTxFifo) &&
                (0u == txSrValid))
            {
                *count = size;
                statusLoc = CY_DFU_SUCCESS;
                break;
            }

            CyDelay(SPI_WAIT_1_MS);
            --timeout;
        }
        while (0u != timeout);

        /* Enable data reception into RX FIFO */
        Cy_SCB_SPI_ClearRxFifo(CY_DFU_SPI_HW);
        CY_DFU_SPI_HW->RX_FIFO_CTRL &= (uint32_t)~SCB_RX_FIFO_CTRL_FREEZE_Msk;
    }

    return (statusLoc);
}


#ifndef CY_DFU_SPI_TRANSPORT_DISABLE

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
    return (SPI_SpiCyBtldrCommRead(buffer, size, count, timeout));
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
    return (SPI_SpiCyBtldrCommWrite(buffer, size, count, timeout));
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
    SPI_SpiCyBtldrCommReset();
}


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
    SPI_SpiCyBtldrCommStart();
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
    SPI_SpiCyBtldrCommStop();
}

#endif /* CY_DFU_SPI_TRANSPORT_DISABLE */

/* [] END OF FILE */
