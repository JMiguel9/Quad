#include <xc.h>
#include "pic16f887.h"
#include "uart2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>

//defines
#define _XTAL_FREQ 16000000
#define bitTest(var, bitno) ((var) & (0x01ULL<< (bitno)))
#define bitset(var, bitno)  ((var) |=1ULL << (bitno))
#define bitclr(var, bitno)  ((var) &= ~(1ULL << (bitno)))

//PIC configuration 
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = ON          // Code Protection bit (Program memory code protection is enabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

volatile unsigned int t1,t2,t3,PULSE_WIDTH,PERIOD;
volatile float DUTY_CYCLE;
volatile char EDGE=0;       //if EDGE ==1 falling edge 

void delay_ms(int n){
    int i;
    for(i=1;i<=n;i++){
        __delay_ms(10);
    }
}
//volatile int EDGE=0;

void interrupt ISR(){
    if(PIR2bits.CCP2IF && PIE2bits.CCP2IE){
        PIR2bits.CCP2IF=0;
        if(EDGE==0){
        t1=CCPR2;
        CCP2CONbits.CCP2M=0x0;
        CCP2CONbits.CCP2M=0x4;  //falling edge   
        EDGE=1;
        }else if(EDGE==1){
            t2=CCPR2;
            CCP2CONbits.CCP2M=0x0;
            PULSE_WIDTH=t2-t1;
            CCP2CONbits.CCP2M=0x5;  //rising edge 
            EDGE=2;
        }else if(EDGE==2){
            PIE2bits.CCP2IE=0;
            t3=CCPR2;
            PERIOD=t3-t1;
            DUTY_CYCLE=(float)PULSE_WIDTH/(float)PERIOD*100;
            TMR1=0x0000;
            EDGE=0;
        }
    } 
}

void initPWM(){
    PR2=0x63;   //20KHz
    CCPR1L=0x4B; 
    CCP1CONbits.DC1B=0x00;  //DT  de 75%
    T2CONbits.T2CKPS=0x00;  //Prescaler 1
    T2CONbits.TMR2ON=1;
    CCP1CON=0x0C;   //Single output, PWM mode.  
    TRISCbits.TRISC2=0;
}

void initADC(){
    //channel 0, PORTA pin 0
    ADCON0bits.ADCS1=1;
    ADCON0bits.ADCS0=0;
    ADCON1bits.VCFG0=0; //Voltage reference pin VSS
    ADCON1bits.VCFG1=0; //Voltage reference pin VDD
    ADCON0bits.CHS=0x0000;  //channel 0
    ADCON1bits.ADFM=1;  //right justified
    ADCON0bits.ADON=1;  //ADC on
}
void ADCstart(){
    ADCON0bits.GO=1;
}

void initTMR1(){
    T1CON=0x01;     //Prescaler 2
}

void initCapture(){
    TRISCbits.TRISC1=1;
    INTCONbits.GIE=1;
    INTCONbits.PEIE=1;
    PIE2bits.CCP2IE=0;
    CCP2CONbits.CCP2M=0x5;  //rising edge 
    PIE2bits.CCP2IE=1;
    initTMR1(); 
}

int main(){
    char pba;
    uint16_t conv;
    unsigned long long DT_CYCLE,TEMP_conv;
    ANSEL=0x01;
    ANSELH=0x00;
    TRISAbits.TRISA0=1; //input
    TRISBbits.TRISB0=0; //output
    PORTBbits.RB0=1;
    inituart(1,1);
    initPWM();
    initADC();
    delay_ms(10);
    initCapture();
    WriteStringUART1("BIENVENIDO\t");
    while(1){  
        //NewLine();
        ADCstart();       
        while(ADCON0bits.GO_DONE);
        conv=(ADRESH<<8)|ADRESL;
        DT_CYCLE=conv*400ULL/1023ULL;
        //WriteStringUART1("DT_CYCLE\t");
        //WriteDec(DT_CYCLE);
       // NewLine();
        TEMP_conv=DT_CYCLE;
        //TEMP_conv=0x0F;
        if(bitTest(TEMP_conv,1))
            CCP1CONbits.DC1B1=1;
        else
            CCP1CONbits.DC1B1=0;
        if(bitTest(TEMP_conv,0))
            CCP1CONbits.DC1B0=1;
        else
            CCP1CONbits.DC1B0=0;
        //TEMP_conv>>=1;
        DT_CYCLE>>=2;
        CCPR1L=DT_CYCLE;
        //WriteStringUART1("bit1\t");
        pba=CCP1CONbits.DC1B1;
        //WriteDec(pba);   
        //NewLine();
        //WriteStringUART1("bit2\t");
        pba=CCP1CONbits.DC1B0;
        //WriteDec(pba);  
       // NewLine();
        WriteStringUART1("t1\t");
        WriteUART1dec2string(t1);
        NewLine();
        WriteStringUART1("t2\t");
        WriteUART1dec2string(t2);
        NewLine();
        WriteStringUART1("t3\t");
        WriteUART1dec2string(t3);
        NewLine();
        WriteStringUART1("PULSE_WIDTH\t");
        WriteUART1dec2string((int)DUTY_CYCLE);
        NewLine();
        WriteStringUART1("DT_CYCLE\t");
        WriteUART1dec2string(DT_CYCLE);      
        NewLine();
        PIE2bits.CCP2IE=1;
        WriteStringUART1("TEST\t");
        NewLine();
        /*if(isDataReady()){
        num=RX_IntNum();
        WriteDec(num);
        WriteStringUART1("\n\r");*/
        delay_ms(100);
    }
}




