/** Baptiste FRITOT / Jeremy VICENTE **/
/** Filtre deriche **/


#include <stdio.h>
#include <stdlib.h>

/* Prototype de la fonction rii.c */
/**********************************/

void filtre_rii (float gamma, float buffer[], int N);


/************************************************
Ecrire une nouvelle fonction filtre_rii () qui réalise le traitement 
complet en appliquant les deux parties causal et anti-causal.
***********************************************/
void filtre_rii (float gamma, float buffer[], int N)
{
	int i;
	float sn_1=0.;
	
	for (i=0; i<N; i++) {
		buffer[i] = sn_1 = gamma*buffer[i] + (1-gamma)*sn_1;

		
	}
	
	sn_1=0.;
	for (i=N-1; i>=0; i--) {
		buffer[i] = sn_1 = gamma*buffer[i] + (1-gamma)*sn_1;
	}
}



