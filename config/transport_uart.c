/***************************************************************************//**
* \file transport_uart.c
* \version 4.0
*
* This file provides the source code of the DFU communication APIs
* for the SCB Component UART mode.
*
* Note
* This file supports the PSoC Creator and ModusToolbox flows.
* For the PSoC Creator flow, the default UART component instance name is 
* "UART". For the ModusToolbox flow, the UART personality alias must be 
* "DFU_UART".
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

#include "transport_uart.h"
#include "cy_scb_uart.h"

/*
* USER CONFIGURABLE: Byte to byte time interval: calculated basing on current 
* component baud rate configuration.
* Set it to approximately to (50e6 / baud_rate) value in microseconds.
* E.g. baud_rate = 115200, UART_BYTE_TO_BYTE_TIMEOUT_US ~ 434
*/
#define UART_BYTE_TO_BYTE_TIMEOUT_US  (868u)


#if defined(CY_PSOC_CREATOR_USED)
    
    /* USER CONFIGURABLE: UART component instance name */
    #include "UART.h"
    
    /* USER CONFIGURABLE: Instance name of the UART component */
    #define CY_DFU_UART_INSTANCE    UART
    
    #define UART_API(fn)            UART_API_IMPL(CY_DFU_UART_INSTANCE, fn)
    #define UART_API_IMPL(a, b)     UART_API_IMPL2(a, b)
    #define UART_API_IMPL2(a, b)    a ## b

    #define CY_DFU_UART_HW          UART_API_IMPL(CY_DFU_UART_INSTANCE, _SCB__HW)
    
#else

/* Includes driver configuration */
#include "cycfg_peripherals.h"

#if !defined DFU_UART_HW
    #error The UART personality alias must be DFU_UART to support DFU communication API.

    /* Dummy configuration to generate only error above during a build */
    #define CY_DFU_UART_HW          NULL
    #define CY_DFU_UART_CFG_PTR     NULL

#else

    /* USER CONFIGURABLE: the pointer to the base address of the hardware */
    #define CY_DFU_UART_HW          DFU_UART_HW

    /* USER CONFIGURABLE: the pointer to the configuration */
    #define  CY_DFU_UART_CFG_PTR    (&DFU_UART_config)

#endif /* !defined DFU_UART_HW */

/**
* UART_initVar indicates whether the UART driver has been initialized. The 
* variable is initialized to false and set to true the first time 
* \ref UART_UartCyBtldrCommStart is called. This allows  the driver to restart 
* without re-initialization after the first call to the 
* \ref UART_UartCyBtldrCommStart routine.
* For re-initialization set \ref UART_initVar to false and call 
* \ref UART_UartCyBtldrCommStart.
* Note that PSoC Creator UART component uses its own initVar variable.
*/
bool UART_initVar = false;


/*******************************************************************************
* Function Name: UART_Start
****************************************************************************//**
*
* Starts SCB UART operation.
*
* \globalvars
* \ref UART_initVar - used to check initial configuration, modified on first
*                     function call.
*
*******************************************************************************/
static void UART_Start(void);
static void UART_Start(void)
{
    if (false == UART_initVar)
    {
        cy_en_scb_uart_status_t status;
        
        /* Configure component */
        status = Cy_SCB_UART_Init(CY_DFU_UART_HW, CY_DFU_UART_CFG_PTR, NULL);

        /* A UART initialization error - stops the execution */
        CY_ASSERT(CY_SCB_UART_SUCCESS == status);
        (void) status;

        /* Component is configured */
        UART_initVar = true;
    }

    Cy_SCB_UART_Enable(CY_DFU_UART_HW);
}

#endif /* defined(CY_PSOC_CREATOR_USED) */

/* Returns a number of bytes to copy into a DFU buffer */
#define UART_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32)(actBufSize) < (uint32)(bufSize)) ? \
                                ((uint32) (actBufSize)) : ((uint32) (bufSize)) )

/*******************************************************************************
* Function Name: UART_UartCyBtldrCommStart
****************************************************************************//**
*
* Starts the UART component.
* 
* \note 
* This function does not configure an infrastructure required for the SCB UART
* operation: clocks and pins. For the PSoC Creator and ModusToolbox flows, the
* generated files configure clocks and pins. This configuration must be
* performed by the application when the project uses only PDL.
*
*******************************************************************************/
void UART_UartCyBtldrCommStart(void)
{
#if defined(CY_PSOC_CREATOR_USED)
    UART_API(_Start)();
#else
    UART_Start();
#endif /* defined(CY_PSOC_CREATOR_USED) */
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommStop
****************************************************************************//**
*
* Disables the UART component.
*
*******************************************************************************/
void UART_UartCyBtldrCommStop(void)
{
#if defined(CY_PSOC_CREATOR_USED)
    UART_API(_Disable)();
#else
    Cy_SCB_UART_Disable(CY_DFU_UART_HW, NULL);
#endif /* defined(CY_PSOC_CREATOR_USED) */
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommReset
****************************************************************************//**
*
* Resets the receive and transmit communication buffers.
*
*******************************************************************************/
void UART_UartCyBtldrCommReset(void)
{
    /* Clear RX and TX buffers */
    Cy_SCB_UART_ClearRxFifo(CY_DFU_UART_HW);
    Cy_SCB_UART_ClearTxFifo(CY_DFU_UART_HW);
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommRead
****************************************************************************//**
*
* Allows the caller to read data from the DFU host (the host writes the
* data). The function handles polling to allow a block of data to be completely
* received from the host device.
*
* \param pData   Pointer to a buffer to store received command.
* \param size    Number of bytes to be read.
* \param count   Pointer to the variable that contains number of bytes that were
*                received.
* \param timeout Time to wait before the function returns because of timeout,
*                in milliseconds.
*
* \return
* The status of the operation:
* - \ref CY_DFU_SUCCESS if successful.
* - \ref CY_DFU_ERROR_TIMEOUT if no data has been received.
* - See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
cy_en_dfu_status_t UART_UartCyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status;
    uint32_t byteCount;

    status = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0u))
    {
        status = CY_DFU_ERROR_TIMEOUT;
        /* Wait with timeout 1 ms for packet end */
        byteCount = 0u;
        do
        {
            /* Check packet start */
            if (Cy_SCB_UART_GetNumInRxFifo(CY_DFU_UART_HW) != 0u)
            {
                /* Wait for end of packet */
                do
                {
                    byteCount = Cy_SCB_UART_GetNumInRxFifo(CY_DFU_UART_HW);
                    Cy_SysLib_DelayUs(UART_BYTE_TO_BYTE_TIMEOUT_US);
                }
                while (byteCount != Cy_SCB_UART_GetNumInRxFifo(CY_DFU_UART_HW));

                byteCount = UART_BYTES_TO_COPY(byteCount, size);
                *count = byteCount;
                status = CY_DFU_SUCCESS;

                break;
            }

            Cy_SysLib_Delay(1u);
            --timeout;
        }
        while (timeout != 0u);

        if (status == CY_DFU_SUCCESS)
        {
            /* Get data from RX buffer into DFU buffer */
            (void) Cy_SCB_UART_GetArray(CY_DFU_UART_HW, (void*)pData, byteCount);
        }
    }

    return (status);
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommWrite
****************************************************************************//**
*
* Allows the caller to write data to the DFU host (the host reads the
* data). The function does not use timeout and returns after data has been
* copied into the transmit buffer. The data transmission starts immediately
* after the first data element is written into the buffer and lasts until all
* data elements from the buffer are sent.
*
* \param pData     Pointer to the block of data to be written to the DFU
*                  host.
* \param size      Number of bytes to be written.
* \param count     Pointer to the variable to write the number of bytes
*                  actually written.
* \param timeOut   The time out is not used by this function.
*                  The function returns as soon as data is copied into the
*                  transmit buffer.
* \return
* The status of the operation:
* - \ref CY_DFU_SUCCESS if successful.
* - See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
cy_en_dfu_status_t UART_UartCyBtldrCommWrite(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status;

    status = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0u))
    {
        /* Transmit data. This function does not wait until data is sent. */
        Cy_SCB_UART_PutArrayBlocking(CY_DFU_UART_HW, (void*)pData, size);

        *count = size;
        status = CY_DFU_SUCCESS;

        if (timeout != 0u)
        {
            /* empty */
        }
    }

    return (status);
}


#ifndef CY_DFU_UART_TRANSPORT_DISABLE

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
    return (UART_UartCyBtldrCommRead(buffer, size, count, timeout));
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
    return (UART_UartCyBtldrCommWrite(buffer, size, count, timeout));
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
    UART_UartCyBtldrCommReset();
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
    UART_UartCyBtldrCommStart();
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
    UART_UartCyBtldrCommStop();
}

#endif /* CY_DFU_UART_TRANSPORT_DISABLE */


/* [] END OF FILE */
