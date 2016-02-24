#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "uart2c.h"
#define _XTAL_FREQ 16000000

/*
 * In the pic16f887 the EUSART module is controlled through three registers:
 * Transmit Status and control (TXSTA)
 * Receive Status and control (RCSTA)
 * Baud Rate Control (BAUDCTL)
 */

void inituart(int TX , int RX){
    if(TX==1){
        RCSTAbits.SPEN=1;   //Configure TX/CK I/O pin as an output
        TXSTAbits.SYNC=0;   //Configures UART to work in asynchronous mode 
        BAUDCTLbits.BRG16=0;    //The BRG16 and the BRGH bits are used to reduce the baud rate error.
        TXSTAbits.BRGH=0;       //The BRG16 is also used to acheive slow baud rates for fast oscillar frequencies. 
        SPBRG=0x19;
        TXSTAbits.TXEN=1; //Enables UART transmitter circuitery 
    }
    if(RX==1){
        RCSTAbits.SPEN=1;   //Configure TX/CK I/O pin as an output
        TXSTAbits.SYNC=0;   //Configures UART to work in asynchronous mode
        BAUDCTLbits.BRG16=0;
        TXSTAbits.BRGH=0;
        SPBRG=0x19;
        RCSTAbits.CREN=1; //Enables UART internal receiver circuitery      
    }
}

void WriteUART1(unsigned int data){
    while (TXSTAbits.TRMT==0);
        TXREG = data;
}

void WriteUART1dec2string(unsigned int data){       //OLD FUNCTION. USE WriteDec instead of this
    unsigned int temp;
    temp=data/10000;
    WriteUART1(temp+'0');
    data=data-temp*10000;
    temp=data/1000;
    WriteUART1(temp+'0');
    data=data-temp*1000;
    temp=data/100;
    WriteUART1(temp+'0');
    data=data-temp*100;
    temp=data/10;
    WriteUART1(temp+'0');
    data=data-temp*10;
    WriteUART1(data+'0');
}

void WriteDec(unsigned int data){
    unsigned int temp,pba;
    //WriteStringUART1("DATA1 ");
    //WriteUART1dec2string(data);
   // NewLine();
    int digits=0;
    if(data==0)
        WriteUART1(temp+'0');
    else{
        temp=data;
        while(temp!=0){
            temp/=10;
            ++digits;
        }
    //WriteStringUART1("DIGITS ");
    //WriteUART1dec2string(digits);
    //NewLine();
        while(digits!=0){
   // WriteStringUART1("PBA ");
        pba=pow(10,digits-1);
   // WriteUART1dec2string(pba);
        temp=data/pba;
    //WriteStringUART1("TEMP ");
    //WriteUART1dec2string(temp);
    //NewLine();
        WriteUART1(temp+'0');
        data=data-temp*pba;
    //WriteStringUART1("temp+0 ");
    //WriteUART1dec2string(temp+'0');
    //NewLine();
        --digits;
        }
    }
}

void NewLine(){
    WriteStringUART1("\n\r");
}

unsigned long pow(unsigned int b,int p){
    int i;
    unsigned int res=1;
    for(i=0;i<p;i++){
        res=res*b;
    }
    return res;
}

void WriteStringUART1(const char * s){
    
    while(*s)
            WriteUART1(*s++);
}

void WriteFloat(const float *n){
    int status;
    char *buff;
    status=sprintf(buff,"%f \n",*n);
    WriteStringUART1(buff);
}
//RECEPTION 
char isDataReady(){
    return RCIF;        //If RCIF is equal to 1 then there is one unread character in the FIFO buffer
}

unsigned char ReadCharUART1(){
    while(RCIF==0);
    return RCREG&0xFF;
}

unsigned long RX_IntNum(){
    char character,digit,length=0;
    unsigned long number=0;
    character=ReadCharUART1();
    //WriteUART1dec2string(character);
    //WriteStringUART1("\n\r");
    while(character!=13){
    digit = 0x10;
    if(character>='0' && character<='9')
        digit=character-'0';
    else if(character>='A' && character<='F')
        digit=(character-'A')+0xA;
    else if(character>='a' && character<='f')
        digit=(character-'a')+0xA;
    if(digit<=0xF){
        number=number*0xA+digit;
        length++;
        WriteUART1(character);       
    }
    else if(character==127 && length){
        number/=0xA;
        length--;
        WriteUART1(character);
        }
    character=ReadCharUART1();
    }   //end of while
    WriteStringUART1("\n\r");
    return number;
} 

//Timer1
void inittmr1(void){
    TMR1=0;
    T1CONbits.TMR1ON=0;
    T1CONbits.TMR1CS=0;
    T1CONbits.T1CKPS=0x00;
    T1CONbits.TMR1GE=0;
    T1CONbits.TMR1ON=1;
}

//CCP capture
void initcapture1(void){
   //TRISCbits.TRISC2=1;
   CCP1CONbits.CCP1M=0x05;
   INTCONbits.GIE=1;
  //INTCONbits.PEIE=1;
  //PIE1bits.CCP1IE=1;       //Interrupts for CCP1 capture mode
   inittmr1();
  //PIR1bits.CCP1IF=0;
   WriteStringUART1("Hola23\n\r");
}

void initcapture2(void){
   TRISCbits.TRISC2=1;
   CCP2CONbits.CCP2M=0x04;
   PIE2bits.CCP2IE=1;
}  
