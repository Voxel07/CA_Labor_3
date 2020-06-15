/*  Header for DCF77 module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, Jan 30, 2020
    Modified: 
*/


// Data type for DCF77 signal events
typedef enum { NODCF77EVENT=0, VALIDZERO, VALIDONE, VALIDSECOND, VALIDMINUTE, INVALID } DCF77EVENT;
//Enum für den Wochentag
typedef enum {Mon=1 ,Tue,Wen,Thu,Fri,Sat,Sun} WOCHENTAG;

// Global variable holding the last DCF77 event
extern DCF77EVENT dcf77Event;
extern WOCHENTAG wochenTag;

// Public functions, for details see dcf77.c
void initDCF77(void);
void displayDateDcf77(void);
DCF77EVENT sampleSignalDCF77(int currentTime);
void processEventsDCF77(DCF77EVENT event);




