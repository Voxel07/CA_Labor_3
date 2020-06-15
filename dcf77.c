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
//Golbale Variable für den Wochentag
WOCHENTAG wochenTag;

// Modul internal global variables
static int  dcf77Year=2025, dcf77Month=1, dcf77Day=1, dcf77Hour=0, dcf77Minute=0 ,dec77Wochentag = 0;       //dcf77 Date and time as integer values
static int startSignal = 0;

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
    asm MOVB #$00,DDRH                              // setze Data Direction Register H als Eingang
}

// ****************************************************************************
// Read the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     0 if signal is Low, >0 if signal is High
char readPort(void)
{

    //Ich glaube das brauchen wir nicht.

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
    
#ifdef SIMULATOR
    currentSignal = readPortSim();
#else
    currentSignal = readPort();
#endif    


    static char letzteFlanke = 0;       //speichert den Wert der letzen Flanke
    char aktuelleFlanke = 0;     //speichert den Wert der aktuellen Flanke
    static int ersteMin = 0;            //für den ersten Durchlauf
    unsigned int tPulse = 0, tLow = 0;                   //Speichert den aktuellen Wert
    static unsigned int tPulseSpeicher = 0, tLowSpeicher = 0;   //Speichert den Wert für den nächsten durchlauf
    
    
    aktuelleFlanke = readPortSim();  

    if (letzteFlanke == aktuelleFlanke)                 //keine Flankenänderung
    {
      event = NODCF77EVENT;   
    } 
    else if((letzteFlanke == 1) && (aktuelleFlanke==0)) //fallende Flanke
    {
        tPulseSpeicher = currentTime;

        if(tPulse != 0){ //Sinnvoll ?
            // Berechnung einer Periode
            tPulseSpeicher = currentTime - tPulseSpeicher;  //Speichern für den nächsten Durchgang
            tPulse = tPulseSpeicher + tLow;                 //Speicherun um damit zu arbeiten
        }

        if((tPulse >= 900) && (tPulse <= 1100))         //nach einer Sekunde 
        {
            event = VALIDSECOND;
        }
        else if((tPulse >= 1900) && (tPulse <= 2100))   //nach zwei Sekunde 
        {
            if (ersteMin==0){
                ersteMin = 1;
            }
            //LED anmachen
            event = VALIDMINUTE;
        }
        else
        {
            event = INVALID;
            //LED ausmachen
        }
        

    }
    else if((letzteFlanke == 1) && (aktuelleFlanke==0)) //steigende Flanke
    {
        if(tLowSpeicher != 0){
            //Berechnung der Low Zeit
            tLowSpeicher = currentTime - tLowSpeicher;  //Speichern für den nächsten Durchgang
            tLow = tLowSpeicher;                        //Speicherun um damit zu arbeiten
        }
        if((tLow >= 70) && (tLow <= 130))               //100 ms = low = 0
        {	                 
			event =  VALIDZERO;	
		} 
        else if((tLow >= 170) && (tLow <= 230))         //200 ms = high = 1
        {	     
			event =   VALIDONE;
		} 
        else 
        {
		    event =  INVALID;
		}		     
    }

    letzteFlanke = aktuelleFlanke;  //für den nächsten Durchlauf speichern
    return event;
}

// ****************************************************************************
// Process the DCF77 events
// Contains the DCF77 state machine
// Parameter:   Result of sampleSignalDCF77 as parameter
// Returns:     -
void processEventsDCF77(DCF77EVENT event)
{

    static char signal[59]={0};
    static int counter = 0;
    int Fehler = 0;

    if(startSignal == 1) 
    {
        if(counter<59) {
            switch(event) 
            {     
                /*LED Setzen fehlt noch*/ 
                case VALIDSECOND:  		  	        
                case VALIDMINUTE:  
                    signal[counter]= ?;               
                    counter++; 
                break;                           
                case VALIDZERO:                  
                    signal[counter]=0;               
                    counter++;  		                
                break;                              
                case VALIDONE:                 
                    signal[counter]=?;              
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
            dcf77Minute = signal[21] + signal[22]*2 + signal[23]*4 + signal[24]*8+ signal[25]*10 + signal[26]*20 + signal[27]*40;			
            //  Stunden
            dcf77Hour = signal[29] + signal[30]*2 + signal[31]*4 + signal[32]*8 + signal[33]*10	+ signal[34]*20;	  		  	
            //  Tag
            dcf77Day = signal[36] + signal[37]*2 + signal[38]*4 + signal[39]*8 + signal[40]*10 + signal[41]*20;	  	
            //  Monat
            dcf77Month = signal[45] + signal[46]*2 + signal[47]*4 + signal[48]*8 + signal[49]*10;		
            //  Jahr
            dcf77Year = signal[50] + signal[51]*2 + signal[52]*4 + signal[53]*8	+ signal[54]*10	+ signal[55]*20+ signal[56]*40	+ signal[57]*80 +2000; //+2000 für Jahr 2000
            
            //Wochentag fehlt noch
            dec77Wochentag = signal[42] + signal[43]*2 + signal[44]*4;

            wochenTag = dec77Wochentag; //Enum wochenTag den aktuellen Wert zuweisen

            /* Parität prüfen */

            
            /*Werte prüfen*/
            if ((dcf77Hour < 0) || (dcf77Hour > 23))    {Fehler = 1;}
            if ((dcf77Minute < 0) || (dcf77Minute > 59))    {Fehler = 1;}
            if ((dcf77Day < 0) || (dcf77Day > 31))  {Fehler = 1;}
            if ((wochenTag <1)||(wochenTag >7))  {Fehler = 1;}
            if ((dcf77Month < 0) || (dcf77Month > 12))  {Fehler = 1;}
        }
    }
        	


}

