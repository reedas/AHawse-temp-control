#include "mbed.h"
#include "cy_pdl.h"
#include "cycfg_capsense.h"
#include "cycfg.h"
#include "temperatureThread.h"

static Semaphore capsense_sem;


/*****************************************************************************
* Function Name: CapSense_InterruptHandler()
******************************************************************************
* Summary:
*  Wrapper function for handling interrupts from CSD block.
*
*****************************************************************************/
static void CapSense_InterruptHandler(void)
{
    Cy_CapSense_InterruptHandler(CYBSP_CSD_HW, &cy_capsense_context);
}


/*****************************************************************************
* Function Name: CapSenseEndOfScanCallback()
******************************************************************************
* Summary:
*  This function releases a semaphore to indicate end of a CapSense scan.
*
* Parameters:
*  cy_stc_active_scan_sns_t* : pointer to active sensor details.
*
*****************************************************************************/
static void CapSenseEndOfScanCallback(cy_stc_active_scan_sns_t * ptrActiveScan)
{
    capsense_sem.release();
}


void capsenseThread(void)
{

    /* Configure AMUX bus for CapSense */
    init_cycfg_routing();
    /* Configure PERI clocks for CapSense */
    Cy_SysClk_PeriphAssignDivider(PCLK_CSD_CLOCK, CYBSP_CSD_CLK_DIV_HW, CYBSP_CSD_CLK_DIV_NUM);
    Cy_SysClk_PeriphDisableDivider(CYBSP_CSD_CLK_DIV_HW, CYBSP_CSD_CLK_DIV_NUM);
    Cy_SysClk_PeriphSetDivider(CYBSP_CSD_CLK_DIV_HW, CYBSP_CSD_CLK_DIV_NUM, 0u);
    Cy_SysClk_PeriphEnableDivider(CYBSP_CSD_CLK_DIV_HW, CYBSP_CSD_CLK_DIV_NUM);
    

    /* Initialize the CSD HW block to the default state. */
    Cy_CapSense_Init(&cy_capsense_context);

    const cy_stc_sysint_t CapSense_ISR_cfg =
    {
        .intrSrc = CYBSP_CSD_IRQ,
        .intrPriority = 4u
    };
    /* Initialize CapSense interrupt */
    Cy_SysInt_Init(&CapSense_ISR_cfg, &CapSense_InterruptHandler);
    NVIC_ClearPendingIRQ(CapSense_ISR_cfg.intrSrc);
    NVIC_EnableIRQ(CapSense_ISR_cfg.intrSrc);

    /* Initialize the CapSense firmware modules. */
    Cy_CapSense_Enable(&cy_capsense_context);
    Cy_CapSense_RegisterCallback(CY_CAPSENSE_END_OF_SCAN_E, CapSenseEndOfScanCallback, &cy_capsense_context);

    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);  // Launch the initial scan   
    
    uint32_t prevBtn0Status = 0u; 
    uint32_t prevBtn1Status = 0u;
     
    while(1)
    {

        capsense_sem.acquire();
        if (CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context))
        {
            Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);
            uint32_t currBtn0Status = Cy_CapSense_IsSensorActive(CY_CAPSENSE_BUTTON0_WDGT_ID, CY_CAPSENSE_BUTTON0_SNS0_ID, &cy_capsense_context);        
            uint32_t currBtn1Status = Cy_CapSense_IsSensorActive(CY_CAPSENSE_BUTTON1_WDGT_ID, CY_CAPSENSE_BUTTON1_SNS0_ID, &cy_capsense_context);       

            if(currBtn0Status != prevBtn0Status && currBtn0Status==1 )
            {
                tempSendDeltaSetpointF(-0.1);
            }
            prevBtn0Status = currBtn0Status;
    
            if(currBtn1Status != prevBtn1Status && currBtn1Status == 1)
            {
                tempSendDeltaSetpointF(0.1);
            } 
            prevBtn1Status = currBtn1Status;

            Cy_CapSense_ScanAllWidgets(&cy_capsense_context);  
        } 

    }
}
