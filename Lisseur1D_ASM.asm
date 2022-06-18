
/***************************************************************************
File Name : Lisseur1D_ASM.asm
***************************************************************************/
.segment /dm seg_dmda;
	.var _rang = 0;
.endseg;
/****************************/
.SEGMENT /PM seg_pmco;
.GLOBAL _lisseur1D_FCT;
_lisseur1D_FCT: 	/********************************
			R4 = sample
			R8 = &coeffs[0] =====> I12
			R12 = &dline[0] =====> I4
			stack = TAPS	=====> R1
		*********************************/
	R1 = DM(1, I6);	/* TAPS */
	L4 = R1; B4 = R12;	/*delay line buffer pointer initialization*/
	L12 = R1; B12 = R8;	/*coefficent buffer pointer initalization*/

	R2 = DM(_rang);
	R12 = R12 + R2;
	R2 = R2 + 1, I4 = R12;
	R0 = R1 - R2;
	IF EQ R2 = R0;
	R1 = R1 - 1;
	DM(_rang) = R2;	

	/*set r12 =0 and store input sample in dline*/
	r12=r12 xor r12, dm(i4,m5)=f4;	

	/* r8=0 and grab data from dline and coef*/
	r8=r8 xor r8, f0=dm(i4,m7), f4=pm(i12,m14); 

	/*set loop to iterate taps-1 times*/
	lcntr=r1, do macs until lce;	
		/* perform mult. accumlate and fetch data*/
	macs:	f12=f0*f4, f8=f8+f12, f0=dm(i4,m7), f4=pm(i12,m14); 
	
	/*perform mult on last pieces of data and 2nd to last add*/
	f12=f0*f4, f8=f8+f12;	

	/*perform last add and store result in f0*/
	f0=f8+f12;		

	restore_state : 
		I12=DM(M7, I6);
		JUMP(M14, I12) (DB);
		I7=I6;
		I6=DM(0, I6);
		
_lisseur1D_FCT.end:

.ENDSEG;
