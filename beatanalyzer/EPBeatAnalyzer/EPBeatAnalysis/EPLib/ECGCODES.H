/* file: ecgcodes.h	T. Baker and G. Moody	  June 1981
			Last revised:  19 March 1992		dblib 7.0
ECG annotation codes

Copyright (C) Massachusetts Institute of Technology 1992. All rights reserved.
*/

#ifndef db_ECGCODES_H	/* avoid multiple definitions */
#define db_ECGCODES_H

#define	NOTQRS	0	/* not-QRS (not a getann/putann code) */
#define NORMAL	1	/* normal beat */
#define	LBBB	2	/* left bundle branch block beat */
#define	RBBB	3	/* right bundle branch block beat */
#define	ABERR	4	/* aberrated atrial premature beat */
#define	PVC	5	/* premature ventricular contraction */
#define	FUSION	6	/* fusion of ventricular and normal beat */
#define	NPC	7	/* nodal (junctional) premature beat */
#define	APC	8	/* atrial premature contraction */
#define	SVPB	9	/* premature or ectopic supraventricular beat */
#define	VESC	10	/* ventricular escape beat */
#define	NESC	11	/* nodal (junctional) escape beat */
#define	PACE	12	/* paced beat */
#define	UNKNOWN	13	/* unclassifiable beat */
#define	NOISE	14	/* signal quality change */
#define ARFCT	16	/* isolated QRS-like artifact */
#define STCH	18	/* ST change */
#define TCH	19	/* T-wave change */
#define SYSTOLE	20	/* systole */
#define DIASTOLE 21	/* diastole */
#define	NOTE	22	/* comment annotation */
#define MEASURE 23	/* measurement annotation */
#define	BBB	25	/* left or right bundle branch block */
#define	PACESP	26	/* non-conducted pacer spike */
#define RHYTHM	28	/* rhythm change */
#define	LEARN	30	/* learning */
#define	FLWAV	31	/* ventricular flutter wave */
#define	VFON	32	/* start of ventricular flutter/fibrillation */
#define	VFOFF	33	/* end of ventricular flutter/fibrillation */
#define	AESC	34	/* atrial escape beat */
#define SVESC	35	/* supraventricular escape beat */
#define	NAPC	37	/* non-conducted P-wave (blocked APB) */
#define	PFUS	38	/* fusion of paced and normal beat */
#define PQ	39	/* PQ junction (beginning of QRS) */
#define	JPT	40	/* J point (end of QRS) */
#define RONT	41	/* R-on-T premature ventricular contraction */

/* ... annotation codes between RONT+1 and ACMAX inclusive are user-defined */

#define	ACMAX	49	/* value of largest valid annot code (must be < 50) */

#endif
