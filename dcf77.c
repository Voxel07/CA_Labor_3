/*  Radio signal clock - DCF77 Module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, Jan 30, 2020
    Modified: 
*/

/*
; N O T E:  T H I S  M O D U L E  I S  N O T  F U L L Y  F U N C T I O N A L
;
; The file contains function prototypes for functions you have to implement.   
; See the sections marked as "--- Add your code here --- --- ??? ---".
;
*/


#include <hidef.h>                                      // Common defines
#include <mc9s12dp256.h>                                // CPU specific defines
#include <stdio.h>

#include "dcf77.h"
#include "led.h"
#include "clock.h"
#include "lcd.h"

// Global variable holding the last DCF77 event
DCF77EVENT dcf77Event = NODCF77EVENT;

// Modul internal global variables
static int  dcf77Year=2025, dcf77Month=1, dcf77Day=1, dcf77Hour=0, dcf77Minute=0;       //dcf77 Date and time as integer values


// Prototypes of functions simulation DCF77 signals, when testing without
// a DCF77 radio signal receiver
void initializePortSim(void);                   // Use instead of initializePort() for testing
char readPortSim(void);                         // Use instead of readPort() for testing

// ****************************************************************************
// Initialize the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     -
void initializePort(void)
{
// --- Add your code here ----------------------------------------------------
// --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? ---
}

// ****************************************************************************
// Read the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     0 if signal is Low, >0 if signal is High
char readPort(void)
{
// --- Add your code here ----------------------------------------------------
// --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? ---
    return -1;
}

// ****************************************************************************
//  Initialize DCF77 module
//  Called once before using the module
void initDCF77(void)
{   setClock((char) dcf77Hour, (char) dcf77Minute, 0);
    displayDateDcf77();

#ifdef SIMULATOR
    initializePortSim();
#else
    initializePort();
#endif    
}

// ****************************************************************************
// Display the date derived from the DCF77 signal on the LCD display, line 1
// Parameter:   -
// Returns:     -
void displayDateDcf77(void)
{   char datum[32];

    (void) sprintf(datum, "%02d.%02d.%04d", dcf77Day, dcf77Month, dcf77Year);
    writeLine(datum, 1);
}

// ****************************************************************************
//  Read and evaluate DCF77 signal and detect events
//  Must be called by user every 10ms
//  Parameter:  Current CPU time base in milliseconds
//  Returns:    DCF77 event, i.e. second pulse, 0 or 1 data bit or minute marker
DCF77EVENT sampleSignalDCF77(int currentTime)
{   DCF77EVENT event = NODCF77EVENT;
    char currentSignal;
    
#ifdef SIMULATOR
    currentSignal = readPortSim();
#else
    currentSignal = readPort();
#endif    

// --- Add your code here ----------------------------------------------------
// --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? ---

    return event;
}

// ****************************************************************************
// Process the DCF77 events
// Contains the DCF77 state machine
// Parameter:   Result of sampleSignalDCF77 as parameter
// Returns:     -
void processEventsDCF77(DCF77EVENT event)
static char Signal[59]={0};
static int counter = 0;
int Fehler = 0;
{
    if(counter<59) {
        switch(event) {     
        
            /*LED Setzen fehlt noch*/ 
            case VALIDSECOND:  		  	        
            case VALIDMINUTE:  
                Signal[counter]= ?;               
                counter++; 
                break;                           
                
            case VALIDZERO:                  
                Signal[counter]=0;               
                counter++;  		                
                break;                          
                
            case VALIDONE:                 
                Signal[counter]=?;              
                counter++;                     
                break;                          
                
            case INVALID:                      
                startSignal=0;                   
                counter=0; 
                Fehler=1;                      
                break;                          
                
            default:                        
                break;                          
        }  	
        }
        else /*Wenn eine Komplette Periode erhalten wrude.*/
        {
        /*Zeit zusammen bauen*/
	    
        //  Minuten
	    dcf77Minute = Signal[21] + Signal[22]*2 + Signal[23]*4 + Signal[24]*8+ Signal[25]*10 + Signal[26]*20 + Signal[27]*40;			
	  	//  Stunden
	  	dcf77Hour = Signal[29] + Signal[30]*2 + Signal[31]*4 + Signal[32]*8 + Signal[33]*10	+ Signal[34]*20;	  		  	
	  	//  Tag
	  	dcf77Day = Signal[36] + Signal[37]*2 + Signal[38]*4 + Signal[39]*8 + Signal[40]*10 + Signal[41]*20;	  	
	  	//  Monat
	  	dcf77Month = Signal[45] + Signal[46]*2 + Signal[47]*4 + Signal[48]*8 + Signal[49]*10;		
	  	//  Jahr
	  	dcf77Year = Signal[50] + Signal[51]*2 + Signal[52]*4 + Signal[53]*8	+ Signal[54]*10	+ Signal[55]*20+ Signal[56]*40	+ Signal[57]*80 +2000; //+2000 für Jahr 2000
		
		

        /* Parität prüfen */
        
        /*Zeit prüfen*/
         if ((dcf77Hour < 0) || (dcf77Hour > 23)){
            Fehler = 1;
		  }
		
		  if ((dcf77Minute < 0) || (dcf77Minute > 59)){
             Fehler = 1;
		  }
          if ((dcf77Day < 0) || (dcf77Day > 31)){
            Fehler = 1;
		  }
		
		  if ((dcf77Month < 0) || (dcf77Month > 12)){
             Fehler = 1;
		  }
        }
        	

   displayDateDcf77();
}

