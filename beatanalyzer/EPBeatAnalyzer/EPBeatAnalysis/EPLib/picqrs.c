/*****************************************************************************
FILE:  picqrs.c
AUTHOR:	Patrick S. Hamilton
  ___________________________________________________________________________

picqrs.c: A PIC based QRS detector.
Copywrite (C) 2002 Patrick S. Hamilton

This file is free software; you can redistribute it and/or modify it under
the terms of the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.

This software is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Library General Public License for more
details.

You should have received a copy of the GNU Library General Public License along
with this library; if not, write to the Free Software Foundation, Inc., 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the author by e-mail (pat@eplimited.com) or postal mail
(Patrick Hamilton, E.P. Limited, 35 Medford St., Suite 204 Somerville,
MA 02143 USA).  For updates to this software, please visit our website
(http://www.eplimited.com).
  __________________________________________________________________________

  This program performs QRS detection using a PIC 16F877 processor running
at 3.6864 MHz.

  As written, ECG samples are fed to the PIC over its RS232 connection
(at 19.2 kbaud), and the QRS detector returns detections over the RS232
connection.  This facilitates testing of the PIC base detector with data
from the MIT/BIH arrhythmia database.  A PC program can read data from the
MIT/BIH database and send it out to the PIC.  Detections received from the
PIC can then be compared to the annotated beat locations.  Implementing a
useful hardware beat detector would require substituting A-to-D input from
an ECG amplifier for the serial input data.

  We implemented and tested this program with the CCS PCM C compiler
version 3.093

***********************************************************************/

#include <16F877.h>
#device PIC16F877 *=16
#fuses HS,NOWDT,NOPROTECT,NOBROWNOUT,NOLVP,PUT
#use delay(clock=3686400)
#use rs232(baud=19200, xmit=PIN_C6, rcv=PIN_C7)

// Prototypes.

byte bgetc(void) ;
int SyncRx(int8 in, int16 *out) ;
void SendInt(int16 x) ;
signed int16 hpfilt( signed int16 datum, int init ) ;
signed int16 lpfilt( signed int16 datum ,int init) ;
signed int16 deriv1( signed int16 x0, int init ) ;
int16 mvwint(signed int16 datum, int init) ;
signed int16 PICQRSDet(signed int16, int init) ;
signed int16 Peak( signed int16 datum, int init ) ;
void UpdateQ(int16 newQ) ;
void UpdateRR(int16 newRR) ;
void UpdateN(int16 newN) ;


// Serial input buffer

byte buffer[64];

byte next_in = 0;
byte next_out = 0;

#define bkbhit (next_in!=next_out)


// Time interval constants.

#define MS80	16
#define MS95	19
#define MS150	30
#define MS200	40
#define MS360	72
#define MS450	90
#define MS1000	200
#define MS1500	300

#define WINDOW_WIDTH	MS80
#define FILTER_DELAY	21 + MS200


/***********************************************
 Main function.
***********************************************/

main()
	{
	char c ;
	signed int16 x ;

	output_low(PIN_D1) ;	// Turn of the buzzer.

	// Turn on interrupt for buffered input.
	
   	enable_interrupts(global);
   	enable_interrupts(int_rda);

	// Initialize filters.

	PICQRSDet(0,1) ;

	// Main loop.

	while(TRUE)
		{
		if(bkbhit)
			{
			c = bgetc() ;
			if(SyncRx(c,&x) != 0)
				{
				output_high(PIN_D1) ;
				x = PICQRSDet(x,0) ;
				output_low(PIN_D1) ;
				SendInt(x) ;
				}
			}
		}
	}

	
/***************************************************************************
* SyncRx handles communication synchronization of input data recieved
* over the RS232 connection.  SyncRx expects data to be transmitted in
* 21 byte frames composed of a synchronization character followed by 10
* 16-bit data values.  Data values are transmitted MSB first followed by
* the LSB.
*
* SyncRx returns a 1 to indicate that a datum has been received. The
* data value is returned in *out.
****************************************************************************/

#define SYNC_CHAR 0x55
#define RESET_CHAR 0xAA
#define FRAME_LGTH 10

int SyncRx(int8 in, int16 *out)
	{
	static int SyncState = 0, frameCount = 0 ;
	static int16 Datum1 ;
	int16 temp ;
	
	// Wait for a sync character.

	if(SyncState == 0)
		{

		// If a sync character is detected,
		// start receiving a frame.

		if(in == SYNC_CHAR)
			{
			SyncState = 1 ;
			frameCount = 0 ;
			}

		// If a reset character is detected,
		// reset the QRS detector, and start
		// receiving a frame.

		else if(in == RESET_CHAR)
			{
			SyncState = 1 ;
			frameCount = 0 ;
			PICQRSDet(0,1) ;
			}
			
		return(0) ;
		}
			

	// Store LSB.

	else if(SyncState == 1)
		{
		Datum1 = in ;
		Datum1 <<= 8 ;
		SyncState = 2 ;
		return(0) ;
		}

	// Put LSB together with the MSB and return new value.

	else
		{
		temp = in ;
		Datum1 |= temp ;
		if(++frameCount == FRAME_LGTH)
			SyncState = 0 ;
		else
			SyncState = 1 ;
		*out = Datum1 ;
		return(1) ;
		}

	return(0) ;
	}

/**********************************************************************
* SendInt transmitts a 16-bit integer, MSB first followed by the LSB.
***********************************************************************/

void SendInt(int16 x)
	{
	int8 c ;
	c = (x >> 8) & 0xFF ;
	putc(c) ;
	c = x & 0xFF ;
	putc(c) ;
	}

// Global values for QRS detector.

int16 Q0 = 0, Q1 = 0, Q2 = 0, Q3 = 0, Q4 = 0, Q5 = 0, Q6 = 0, Q7 = 0 ;
int16 N0 = 0, N1 = 0, N2 = 0, N3 = 0, N4 = 0, N5 = 0, N6 = 0, N7 = 0 ;
int16 RR0=0, RR1=0, RR2=0, RR3=0, RR4=0, RR5=0, RR6=0, RR7=0 ;
int16 QSum = 0, NSum = 0, RRSum = 0 ;
int16 det_thresh, sbcount ;

int16 tempQSum, tempNSum, tempRRSum ;

int16 QN0=0, QN1=0 ;
int Reg0=0 ;

/******************************************************************************
*  PICQRSDet takes 16-bit ECG samples (5 uV/LSB) as input and returns the
*  detection delay when a QRS is detected.  Passing a nonzero value for init
*  resets the QRS detector.
******************************************************************************/
 
signed int16 PICQRSDet(signed int16 x, int init)
	{
	static int16 tempPeak, initMax ;
	static int8 preBlankCnt=0, qpkcnt=0, initBlank=0 ;
	static int16 count, sbpeak, sbloc ;
	int16 QrsDelay = 0 ;
	int16 temp0, temp1 ;

	if(init)
		{		
		hpfilt(0,1) ;
		lpfilt(0,1) ;
		deriv1(0,1) ;
		mvwint(0,1) ;
		Peak(0,1) ;
		qpkcnt = count = sbpeak = 0 ;
		QSum = NSum = 0 ;

		RRSum = MS1000<<3 ;
		RR0=RR1=RR2=RR3=RR4=RR5=RR6=RR7=MS1000 ;

		Q0=Q1=Q2=Q3=Q4=Q5=Q6=Q7=0 ;		
		N0=N1=N2=N3=N4=N5=N6=N7=0 ;
		NSum = 0 ;

		return(0) ;
		}

	x = lpfilt(x,0) ;
	x = hpfilt(x,0) ;
	x = deriv1(x,0) ;
	if(x < 0) x = -x ;
	x = mvwint(x,0) ;
	x = Peak(x,0) ;


	// Hold any peak that is detected for 200 ms
	// in case a bigger one comes along.  There
	// can only be one QRS complex in any 200 ms window.

	if(!x && !preBlankCnt)
		x = 0 ;

	else if(!x && preBlankCnt)		// If we have held onto a peak for
		{				// 200 ms pass it on for evaluation.
		if(--preBlankCnt == 0)
			x = tempPeak ;
		else x = 0 ;
		}

	else if(x && !preBlankCnt)		// If there has been no peak for 200 ms
		{			// save this one and start counting.
		tempPeak = x ;
		preBlankCnt = MS200 ;
		x = 0 ;
		}

	else if(x)				// If we were holding a peak, but
		{				// this ones bigger, save it and
		if(x > tempPeak)		// start counting to 200 ms again.
			{
			tempPeak = x ;
			preBlankCnt = MS200 ;
			x = 0 ;
			}
		else if(--preBlankCnt == 0)
			x = tempPeak ;
		else x = 0 ;
		}

	// Initialize the qrs peak buffer with the first eight
	// local maximum peaks detected.

	if( qpkcnt < 8 )
		{
		++count ;
		if(x > 0) count = WINDOW_WIDTH ;
		if(++initBlank == MS1000)
			{
			initBlank = 0 ;
			UpdateQ(initMax) ;
			initMax = 0 ;
			++qpkcnt ;
			if(qpkcnt == 8)
				{

				RRSum = MS1000<<3 ;
				RR0=RR1=RR2=RR3=RR4=RR5=RR6=RR7=MS1000 ;

				sbcount = MS1500+MS150 ;
				}
			}
		if( x > initMax )
			initMax = x ;
		}

	else
		{
		++count ;
		
		// Check if peak is above detection threshold.

		if(x > det_thresh)
			{
			UpdateQ(x) ;

			// Update RR Interval estimate and search back limit

			UpdateRR(count-WINDOW_WIDTH) ;
			count=WINDOW_WIDTH ;
			sbpeak = 0 ;
			QrsDelay = WINDOW_WIDTH+FILTER_DELAY ;
			}
		
		// If a peak is below the detection threshold.

		else if(x != 0)
			{
			UpdateN(x) ;

			QN1=QN0 ;
			QN0=count ;

			if((x > sbpeak) && ((count-WINDOW_WIDTH) >= MS360))
				{
				sbpeak = x ;
				sbloc = count-WINDOW_WIDTH ;
				}

			}
			
		// Test for search back condition.  If a QRS is found in
		// search back update the QRS buffer and det_thresh.

		if((count > sbcount) && (sbpeak > (det_thresh >> 1)))
			{
			UpdateQ(sbpeak) ;

			// Update RR Interval estimate and search back limit

			UpdateRR(sbloc) ;

			QrsDelay = count = count - sbloc;
			QrsDelay += FILTER_DELAY ;
			sbpeak = 0 ;
			}
		}
					

	return(QRSDelay) ;
	}
	

/**************************************************************************
*  UpdateQ takes a new QRS peak value and updates the QRS mean estimate
*  and detection threshold.
**************************************************************************/

void UpdateQ(int16 newQ)
	{

	QSum -= Q7 ;
	Q7=Q6; Q6=Q5; Q5=Q4; Q4=Q3; Q3=Q2; Q2=Q1; Q1=Q0;
	Q0=newQ ;
	QSum += Q0 ;

	det_thresh = QSum-NSum ;
	det_thresh = NSum + (det_thresh>>1) - (det_thresh>>3) ;

	det_thresh >>= 3 ;
	}


/**************************************************************************
*  UpdateN takes a new noise peak value and updates the noise mean estimate
*  and detection threshold.
**************************************************************************/

void UpdateN(int16 newN)
	{
	NSum -= N7 ;
	N7=N6; N6=N5; N5=N4; N4=N3; N3=N2; N2=N1; N1=N0; N0=newN ;
	NSum += N0 ;

	det_thresh = QSum-NSum ;
	det_thresh = NSum + (det_thresh>>1) - (det_thresh>>3) ;

	det_thresh >>= 3 ;
	}

/**************************************************************************
*  UpdateRR takes a new RR value and updates the RR mean estimate
**************************************************************************/

void UpdateRR(int16 newRR)
	{
	RRSum -= RR7 ;
	RR7=RR6; RR6=RR5; RR5=RR4; RR4=RR3; RR3=RR2; RR2=RR1; RR1=RR0 ;
	RR0=newRR ;
	RRSum += RR0 ;

	sbcount=RRSum+(RRSum>>1) ;
	sbcount >>= 3 ;
	sbcount += WINDOW_WIDTH ;
	}
	

/*************************************************************************
*  lpfilt() implements the digital filter represented by the difference
*  equation:
*
* 	y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[n-5] + x[n-10]
*
*	Note that the filter delay is five samples.
*
**************************************************************************/

signed int16 lpfilt( signed int16 datum ,int init)
	{
	static signed int16 y1 = 0, y2 = 0 ;
	static signed int16 d0,d1,d2,d3,d4,d5,d6,d7,d8,d9 ;
	signed int16 y0 ;
	signed int16 output ;

	if(init)
		{
		d0=d1=d2=d3=d4=d5=d6=d7=d8=d9=0 ;
		y1 = y2 = 0 ;
		}
	
	y0 = (y1 << 1) - y2 + datum - (d4<<1) + d9 ;
	y2 = y1;
	y1 = y0;
	if(y0 >= 0) output = y0 >> 5;
	else output = (y0 >> 5) | 0xF800 ;

	d9=d8 ;
	d8=d7 ;
	d7=d6 ;
	d6=d5 ;
	d5=d4 ;
	d4=d3 ;
	d3=d2 ;
	d2=d1 ;
	d1=d0 ;
	d0=datum ;
						
	return(output) ;
	}

/******************************************************************************
*  hpfilt() implements the high pass filter represented by the following
*  difference equation:
*
*	y[n] = y[n-1] + x[n] - x[n-32]
*	z[n] = x[n-16] - y[n] ;
*
*  Note that the filter delay is 15.5 samples
******************************************************************************/

#define HPBUFFER_LGTH	32

signed int16 hpfilt( signed int16 datum, int init )
	{
	static signed int16 y=0 ;
	static signed int16 data[HPBUFFER_LGTH] ;
	static int ptr = 0 ;
	signed int16 z ;
	int halfPtr ;

	if(init)
		{
		for(ptr = 0; ptr < HPBUFFER_LGTH; ++ptr)
			data[ptr] = 0 ;
		ptr = 0 ;
		y = 0 ;
		return(0) ;
		}

	y += datum - data[ptr];

	halfPtr = ptr-(HPBUFFER_LGTH/2) ;
	halfPtr &= 0x1F ;

	
	z = data[halfPtr] ;		// Compensate for CCS shift bug.
	if(y >= 0) z -= (y>>5) ;
	else z -= (y>>5)|0xF800 ;


	data[ptr] = datum ;
	ptr = (ptr+1) & 0x1F ;

	return( z );
	}

/*****************************************************************************
*  deriv1 and deriv2 implement derivative approximations represented by
*  the difference equation:
*
*	y[n] = 2*x[n] + x[n-1] - x[n-3] - 2*x[n-4]
*
*  The filter has a delay of 2.
*****************************************************************************/

signed int16 deriv1( signed int16 x0, int init )
	{
	static signed int16 x1, x2, x3, x4 ;
	signed int16 output;
	if(init)
		x1 = x2 = x3 = x4 = 0 ;

	output = x1-x3 ;
	if(output < 0) output = (output>>1) | 0x8000 ;	// Compensate for shift bug.
	else output >>= 1 ;

	output += (x0-x4) ;
	if(output < 0) output = (output>>1) | 0x8000 ;
	else output >>= 1 ;
	
	
	x4 = x3 ;
	x3 = x2 ;
	x2 = x1 ;
	x1 = x0 ;
	return(output);
	}


/*****************************************************************************
* mvwint() implements a moving window integrator, averaging
* the signal values over the last 16
******************************************************************************/

int16 mvwint(signed int16 datum, int init)
	{
	static unsigned int16 sum = 0 ;
	static unsigned int d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15 ;

	if(init)
		{
		d0=d1=d2=d3=d4=d5=d6=d7=d8=d9=d10=d11=d12=d13=d14=d15=0 ;
		sum = 0 ;
		}
	sum -= d15 ;

	d15=d14 ;
	d14=d13 ;
	d13=d12 ;
	d12=d11 ;
	d11=d10 ;
	d10=d9 ;
	d9=d8 ;
	d8=d7 ;
	d7=d6 ;
	d6=d5 ;
	d5=d4 ;
	d4=d3 ;
	d3=d2 ;
	d2=d1 ;
	d1=d0 ;
	if(datum >= 0x0400) d0 = 0x03ff ;
	else d0 = (datum>>2) ;
	sum += d0 ;

	return(sum>>2) ;
	}


/**************************************************************
* peak() takes a datum as input and returns a peak height
* when the signal returns to half its peak height, or it has been
* 95 ms since the peak height was detected. 
**************************************************************/


signed int16 Peak( signed int16 datum, int init )
	{
	static signed int16 max = 0, lastDatum ;
	static int timeSinceMax = 0 ;
	signed int16 pk = 0 ;

	if(init)
		{
		max = 0 ;
		timeSinceMax = 0 ;
		return(0) ;
		}
		
	if(timeSinceMax > 0)
		++timeSinceMax ;

	if((datum > lastDatum) && (datum > max))
		{
		max = datum ;
		if(max > 2)
			timeSinceMax = 1 ;
		}

	else if(datum < (max >> 1))
		{
		pk = max ;
		max = 0 ;
		timeSinceMax = 0 ;
		}

	else if(timeSinceMax > MS95)
		{
		pk = max ;
		max = 0 ;
		timeSinceMax = 0 ;
		}
	lastDatum = datum ;
	return(pk) ;
	}


/***********************************************************************
* Serial input interrupt service routine.  Input is stored in a FIFO.
************************************************************************/

#int_rda
void serial_isr()
	{
	buffer[next_in]=getc() ;
 	next_in=(next_in+1) & 0x3F;
	}


/************************************************************************
* bgetc() (buffered getc()) returns the next value in the serial input FIFO.
*************************************************************************/

byte bgetc() 
	{
   	byte c;

   	c=buffer[next_out];
   	next_out=(next_out+1) & 0x3F;
   	return(c);
	}






	
	
			


