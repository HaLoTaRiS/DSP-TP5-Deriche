///////////////////////////////////////////////////////////////////////////////////////
//NAME:     initSPORT.c (Block-based Talkthrough)
//PURPOSE:  Talkthrough framework for sending and receiving samples to the AD1835.
//
//USAGE:    This file initializes the SPORTs for DMA Chaining
//
////////////////////////////////////////////////////////////////////////////////////////
#include <cdef21262.h>
#include <def21262.h>
#include <signal.h>
#include <sru.h> // ajout de la bibliothèque DAI interruption
/*
   Here is the mapping between the SPORTS and the DACS
   ADC -> DSP  : SPORT0A : IIS
   DSP -> DAC1 : SPORT1A : IIS
   DSP -> DAC2 : SPORT1B : IIS
   DSP -> DAC3 : SPORT2A : IIS
   DSP -> DAC4 : SPORT2B : IIS
*/

unsigned int PCI = 0x00080000 ;

// TCB blocks for Chaining
// Each block will be used for:
//      Filling from the ADC
//      Processing filled data
//      Sending to DAC
//
//Each one is doing only one of these steps for each SPORT interrupt.


extern int rx_buf[2], tx_buf[2] ;

extern void Reception_rifes(int sig_int);
extern void my_irqP1(int sig_int);

//Set up the TCBs to rotate automatically
int tx_tcb[4] = {0,2,1,(int) tx_buf};
int rx_tcb[4] = {0,2,1,(int) rx_buf};

void InitSPORT()
{
    interrupt (SIG_SP1,SIG_IGN); //Tools Anomaly sets SP1 int with all Even SPORTS
    
    //ajout des fonctions d'interruption SP0 SP1
    interrupt (SIG_SP0, Reception_rifes);
  	interrupt (SIG_SP1, SIG_IGN);
  	
  	// ajout de la fonction PIO
  	//interrupt (SIG_DAIH, my_irqP1);
    	
    *pSPMCTL01 = 0;
    *pSPMCTL23 = 0;

	*pSPCTL0 = 0 ;
    *pSPCTL1 = 0 ;
    *pSPCTL2 = 0 ;

    // Configure SPORT 0 (A) for input from ADC
    //------------------------------------------------------------
    *pSPCTL0 = (BHD | OPMODE | SLEN24 | SPEN_A | SCHEN_A | SDEN_A);

    // Enabling Chaining
   	rx_tcb[0] = (((int)rx_tcb + 3) & 0x7FFFF) + PCI;
	*pCPSP0A = (int)rx_tcb + 3 + PCI;


	
	// Configure SPORT 2 for output to DAC 4 (channel B) ==> write to DAC4
    //-------------------------------------------------------------------
    *pSPCTL2 |= (SPTRAN | OPMODE | SLEN24 | SPEN_B | BHD | SCHEN_B | SDEN_B);
    	    
	// Enabling Chaining (SPORT 2B, voir initDAI.c) 
    //---------------------------------------------
    tx_tcb[0] = (((int)tx_tcb + 3) & 0x7FFFF) + PCI;
	*pCPSP2B = (int)tx_tcb + 3  + PCI;
}

                                                     
