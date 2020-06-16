/*  Radio signal clock - DCF77 Module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
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
static int  dcf77Year=2025, dcf77Month=1, dcf77Day=1, dcf77Hour=0, dcf77Minute=0, dcf77wochenTag=0;       //dcf77 Date and time as integer values

const char* WOCHENTAG[] = {"MON","TUE","WEN","THU","FRI","SAT","SUN"};

// Prototypes of functions simulation DCF77 signals, when testing without
// a DCF77 radio signal receiver
void initializePortSim(void);                   // Use instead of initializePort() for simulator testing
char readPortSim(void);                         // Use instead of readPort() for simulator testing

// ****************************************************************************
// Initialize the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     -
//
// NOT USED IN THE CORONA EDITION
//
//void initializePort(void)
//{
// --- Add your code here ----------------------------------------------------
// --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? ---
//}

// ****************************************************************************
// Read the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     0 if signal is Low, >0 if signal is High
//
// NOT USED IN THE CORONA EDITION
//
//char readPort(void)
//{
// --- Add your code here ----------------------------------------------------
// --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? ---
//    return -1;
//}

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

    (void) sprintf(datum, "%s - %02d.%02d.%04d",WOCHENTAG[dcf77wochenTag-1], dcf77Day, dcf77Month, dcf77Year);
    writeLine(datum, 1);
}

// ****************************************************************************
//  Read and evaluate DCF77 signal and detect events
//  Must be called by user every 10ms
//  Parameter:  Current CPU time base in milliseconds
//  Returns:    DCF77 event, i.e. second pulse, 0 or 1 data bit or minute marker
DCF77EVENT sampleSignalDCF77(int currentTime)
{   DCF77EVENT event = NODCF77EVENT;
   
    static char oldState = 0;   //Wert der letzen Flanke
    char currentState = 0;      //Wert der aktuellen Flanke
    static int startTime = 0;   //Zeitpunkt der fallenden Flanke
    int diffTime = 0;           //Verstrichene Zeit zwischen currentTime und  
    
#ifdef SIMULATOR
    currentState = readPortSim();			// Sample simulated DCF77 signal
#else
    currentState = readPort();				// Sample DCF77 signal
#endif   

    if (oldState == currentState)           //keine Flankenänderung
    {
        event = NODCF77EVENT;   
    } 
    else if ((oldState == 1)&&(currentState == 0))   //fallende Flanke
    {
        setLED(0x02);   //LED 2 einschalten
        diffTime = currentTime - startTime;

        if((diffTime >= 900) && (diffTime <= 1100))         //nach einer Sekunde 
        {
            event = VALIDSECOND;
        }
        else if((diffTime >= 1900) && (diffTime <= 2100))   //nach zwei Sekunde 
        {
            event = VALIDMINUTE;
        }
        else    //Fehlerfall
        {
            event = INVALID;       
        }
        startTime = currentTime;
    }
    else    //steigende Flanke
    {   
        clrLED(0x02);
        diffTime = currentTime - startTime;
        if((diffTime >= 70) && (diffTime <= 130))               //100 ms = low = 0
        {	                 
            event =  VALIDZERO;	
        } 
        else if((diffTime >= 170) && (diffTime <= 230))         //200 ms = high = 1
        {	     
            event =   VALIDONE;
        } 
        else    	//Fehlerfall 
        {
            event =  INVALID;
        }		     
    }

    oldState = currentState;    //für den nächsten Durchlauf speichern
    return event;
}

// ****************************************************************************
// Process the DCF77 events
// Contains the DCF77 state machine
// Parameter:   Result of sampleSignalDCF77 as parameter
// Returns:     -
void processEventsDCF77(DCF77EVENT event)
{
    static char signal[59] = {0};   //dc77 Signal über eine Minute
    static int counter = 0;         //aktuelle Position
    int error = 0;                  //kaputt
    int paritaet = 0;
    int sum = 0;
    int i = 0;
    switch(event) 
    {     
        /*LED Setzen fehlt noch*/ 
        case VALIDSECOND:  	
            counter++; 
        break;	  	        
        case VALIDMINUTE:
            if((!error)&&(counter == 58))
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
                dcf77Year = signal[50] + signal[51]*2 + signal[52]*4 + signal[53]*8	+ signal[54]*10	+ signal[55]*20+ signal[56]*40	+ signal[57]*80 +2000; //+2000 fÃ¼r Jahr 2000
                //Wochentag 
                dcf77wochenTag = signal[42] + signal[43]*2 + signal[44]*4;

                //Daten Plausibel ?
                if ((dcf77Hour < 0) || (dcf77Hour > 23))    {error = 1;}
                if ((dcf77Minute < 0) || (dcf77Minute > 59))    {error = 1;}
                if ((dcf77Day < 0) || (dcf77Day > 31))  {error = 1;}
                if ((dcf77wochenTag <1)||(dcf77wochenTag >7))  {error = 1;}
                if ((dcf77Month < 0) || (dcf77Month > 12))  {error = 1;}
                // Paritätsprüfung
               
                //Bit 28
                for(i = 21; i <=28; i++){
                    sum += signal[i];
                }
                paritaet = sum % 2;
                //Bit 35
                for(i = 29; i <=35; i++){
                    sum += signal[i];
                }
                paritaet += sum % 2;                
                //Bit 58
                for(i = 36; i <=58; i++){
                     sum += signal[i];
                }
                paritaet += sum %2;

                if(paritaet>0||error) //Fehlerfall
                { 
                    clrLED (0x08);      //LED B3
                }
                else    //Passt alles
                {
                    setLED (0x08);      //LED B3
                    displayDateDcf77();
                    setClock((char)dcf77Day,(char)dcf77Minute,(char)0);
                }
            }
            else
            {
                clrLED(0x04);       //LED B2
                clrLED(0x08);       //LED B3
            }
            counter = 0;
           
        break;                           
        case VALIDZERO:                  
            signal[counter]= 0 ;                       
        break;                              
        case VALIDONE:                 
            signal[counter]=1;              
        break;                             
        case INVALID:                      
            error=1; 
            setLED(0x04);                                        
        break;   
        case NODCF77EVENT:

        break;                         
        default:   
             error=1;                      
        break;                          
    }  	
}

