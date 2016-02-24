//UART
void inituart(int TX , int RX);
void WriteUART1(unsigned int data);
void WriteUART1dec2string(unsigned int data);
unsigned long RX_IntNum();
unsigned char ReadCharUART1();
void WriteDec(unsigned int data);
void NewLine();
unsigned long pow(unsigned int b,int p);
void WriteFloat(const float *n);
char isDataReady();
void WriteStringUART1(const char * s);
void initcapture1(void);
void inittmr1(void);

