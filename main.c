///////////////////////////////////////////////////////////////////////////////////////
//NAME:     main.c 
// Baptiste FRITOT / Jeremy VICENTE.
//
// TP5 Code 
//
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
#include <cdef21262.h>
#include <def21262.h>

#include <signal.h> // ajout de la bibliothèque signal
#include <sru.h> // ajout de la bibliothèque DAI interruption


/**************************************************
** Prototyppe 
**************************************************/
extern void InitDAI(void);
extern void Init1835viaSPI(void);
extern void InitSPORT(void);

// Code filtre TP4 
//float rif_FCT(float rx_buff_0_decal, float *coef , float *dlline, int taps );

// Prototype filtre TP5
extern void filtre_rii (float gamma, float buffer[], int N);

void AfficheLEDs(int led_value);
void init_interrupt_DIO(void);
void my_irqP1( int sig_num );

/**************************************************
** Variable 
**************************************************/
int value_led = 0;

int rx_buf[2], tx_buf[2];
// nouvelle variable
float rx_buff_0_decal;
float rx_sortie_filtre;

// tableau pour le pipeline 
float tab1[4800];
float tab2[4800];
float tab3[4800];

// Variable pour le nbr d'échantillon 
int pos_table = 0 ; 
int memoire_tableau=0; 

// Variable avant utilisation des pointeurs :
int var_traitement =0;

// Variable pour le traitement
int flag_traitement = 0;
int flag_permutation = 0;

// Variable pour le SW4
int flag_sw4 =0;
float coef_filtre = 0.125;

//pointeur  pour les tableaux 
float *ptr_E = &tab1[0]; // entree
float *ptr_T = &tab2[0]; // traitement
float *ptr_S = &tab3[0]; // Sortie 
float *ptr_temp; // temporaire

/**************************************************
** Code 
**************************************************/
// IRQ_P1 - Interrupt runtime Sync
// --------------------------------------
void my_irqP1( int sig_num ) 
{
	int IT_register = *pDAI_IRPTL_H;
   
    sig_num = sig_num ;
    if ((IT_register & SRU_EXTMISCB1_INT) != 0) // on vérifie que l'origine de l'IT de DAI est DAI_P1
    {
		/**** Question 1 : Led qui s'affiche en fonction de la fréquence ****/
        //AfficheLEDs(value_led++); // Affiche les leds et incrémente
 
		/**** Question 4 : Traitement dans les tableaux ****/
        /****** Déplacement avec pointeur ******/
		/* if (var_traitement >2) {
            var_traitement=0;
    	}
    	else{
        var_traitement ++;
		// Permet de voir le déplacement dans les tableaux depuis les leds 
        // AfficheLEDs(var_traitement);
    	}*/
		
        /****** Déplacement avec pointeur ******/
        flag_permutation = 0;
       	ptr_temp =  ptr_E; 
       	ptr_E = ptr_S;
       	ptr_S = ptr_T;
       	ptr_T = ptr_temp;
		
		// Permet de controler la permutation des pointeurs
       	flag_permutation =1;
		// permet de sortir le calcul => flag géré dans le main pour le traitement du filtre
       	flag_traitement = 1;
		
       	AfficheLEDs(memoire_tableau);
    }
	
    // question 6, interruption par le bouton poussoir SW4
    if ((IT_register & SRU_EXTMISCB2_INT) != 0) // si interruption sur le sw4
    {
        // Permet de tester l'interruption avec basculement de l'état de la led
        /*if (flag_sw4 == 0){
            
        	AfficheLEDs(1);
        	flag_sw4 = 1;
        }
        else {
           AfficheLEDs(0);
           flag_sw4 = 0; 
        }*/
		
		// on change le coefficient du filtre :
        coef_filtre = coef_filtre + 0.125;
        if (coef_filtre > 1) {
            coef_filtre = 0.125;
        }
    }
}

// SPORT0 and SPORT1 Interrupt Service Routines      
//--------------------------------------------------------------------------------------------
/* ISR counters, for debug purposes to see how many times SPORT DMA interrupts are serviced */
void Reception_rifes(int sig_int) // acquisition
{
//  Recopie l'entrée sur la sortie : Test pour vérifier le fonctionnement
//  tx_buf[0] = rx_buf[0]; 
//	tx_buf[1] = rx_buf[1] ; 

//  Permet de vérifier le bon fonctionnement et défilement de var_traitement
//	AfficheLEDs(var_traitement);
	
	// Vérifie si nous avons bien permuter les tableaux : 
	if (flag_permutation == 1) {
		/*if (var_traitement==0){
	    	// lecture table 1 
			tab1[pos_table] =((rx_buf[0] << 8) >> 8);
			// ecriture table 3 
			tx_buf[0] = tab3[pos_table];
		}
		
		else if (var_traitement==1){
		    // lecture table 2
			tab3[pos_table] =((rx_buf[0] << 8) >> 8);
			// ecriture table 1
			tx_buf[0] = tab2[pos_table];
		}	
	
		else if (var_traitement==2){
		    // lecture table 3 
			tab2[pos_table] =((rx_buf[0] << 8) >> 8);
			// écriture table 2
			tx_buf[0] = tab1[pos_table];
		}*/	
	
		// Entrée => écriture dans le tableau
		*(ptr_E + pos_table) = ((rx_buf[0] << 8) >> 8);
		// Sortie => lecture dans le tableau 
		tx_buf[0] = *(ptr_S + pos_table);
		// Protection contre le débordement
		if (pos_table < 4799) pos_table++;	
	}
}

/**********************************************************/
void AfficheLEDs(int led_value){
//lights as described at the top of the file
    *pPPCTL=0;

    *pIIPP=(int) &led_value;
    *pIMPP=1;
    *pICPP=1;
    *pEMPP=1;
    *pECPP=1;
    *pEIPP=0x1400000;

    *pPPCTL=PPTRAN|PPBHC|PPDUR20|PPDEN|PPEN;
}

// Question 6
// Fonction pour initialiser l'interruption
void init_interrupt_DIO(void){
	
	// Question 1 et modifié par la question 6 (BP)
    *pDAI_IRPTL_PRI = SRU_EXTMISCB1_INT  | SRU_EXTMISCB2_INT; // Démasquer individuellement l'interruption
    *pDAI_IRPTL_RE = SRU_EXTMISCB1_INT  | SRU_EXTMISCB2_INT; // Choisir le front montant pour l'interruption
    *pSYSCTL |= IRQ1EN|IRQ2EN;
    
	// Question 1 
    // Unité de routage : Router (DAI_P1 ----> MISCB1) P1
    SRU(LOW,DAI_PB01_I);
    SRU(DAI_PB01_O,MISCB1_I);
    SRU(LOW,PBEN01_I)
    
	// question 6, interruption par le bouton poussoir SW4
    // Unité de routage : Router (DAI_PB20_I ----> MISCB2) SW4
    SRU(LOW,DAI_PB20_I); // Innitialise SW4 sur niveau bas en entree
    SRU(DAI_PB20_O,MISCB2_I);  // Connect à MISCB2
    SRU(LOW,PBEN20_I);			// Niveau pas 
    
	
	// Fonction qui active l’interruption et désigne la fonction routine d’interruption
	interrupt (SIG_DAIH, my_irqP1);
}

void main(void)
{
    // Enable interrupt nesting.
    asm( "#include <def21262.h>" );
    asm( "bit set mode1 IRPTEN;"  ); // Enable interrupts (globally)
    asm( "LIRPTL = SP0IMSK;"  ); 	// Unmask the SPORT0 ISR

    //pointeur : 
	ptr_E = &tab1[0]; // entree
	ptr_T = &tab2[0]; // traitement
	ptr_S = &tab3[0]; // Sortie 

    // Need to initialize DAI because the sport signals need to be routed
    InitDAI();
    // This function will configure the codec on the kit
    Init1835viaSPI();

    // Finally setup the sport to receive / transmit the data
    InitSPORT();
   
    // Sur une seule interruption 
    // *pDAI_IRPTL_PRI = SRU_EXTMISCB1_INT; // Démasquer individuellement l'interruption
    // *pDAI_IRPTL_RE = SRU_EXTMISCB1_INT; // Choisir le front montant pour l'interruption
    
    // Initialise les interruptions 
    init_interrupt_DIO();

   
    
     // Be in infinite loop and do nothing until done.
    for(;;)
    {
		
		/**
		Nota : Le traitement dans main permet de décharger - Limiter le traitement dans les interruptions 
		**/
        if (flag_traitement == 1){
            
           //var_traitement = 0; 
           memoire_tableau = pos_table;
	    	pos_table = 0; 
	    

			// Question 5 : traitement du filtre
			/***Avant amélioration pointeur ***/ 
           /* if (var_traitement==0){

		    	var_traitement = 1; LisseurD1 (0.125, &tab2[0], memoire_tableau);
			}
	
			else if (var_traitement==1){
			    var_traitement = 2;LisseurD1 (0.125, &tab1[0], memoire_tableau);
			}	
	
			else if (var_traitement==2){
		    	var_traitement = 0; LisseurD1 (0.125, &tab3[0], memoire_tableau);
			}	*/
			
			/***Avant amélioration pointeur T ***/  
            //Lisseur1D (0.5, ptr_T, memoire_tableau);
			
			// Question 6 : changement du coef sur SW4
            Lisseur1D (coef_filtre, ptr_T, memoire_tableau);
			
        	flag_traitement =0;
        }
    }    
}
