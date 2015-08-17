/*this file explains the common functions used in atoll library*/


/*To setup the running frequency of the microcontroller to 50Mhz*/
void AtollrunningFrq_init(void);

/*Initialise the uart0 with different baud rate ,intstatus should be 1 to activate interrupt*/
/* and intstatus should be 0 to deactivate interrupt for the uart0, should keep intstatus as 1 if the uart is using for modbus communication */
void AtollUart0_Init(int baudrate,char intstatus);

/*Function used to send a string in to uart 0, string should end with '\0'*/
void AtollUart0_StringSend(const char *pucBuffer);

/*Initialise the uart6 with different baud rate ,intstatus should be 1 to activate interrupt*/
/* and intstatus should be 0 to deactivate interrupt for the uart6,intstatus should be 1 for modbus communication */
void AtollUart6_Init(int baudrate, char intstatus);

/*Function used to send a string in to uart 6, string should end with '\0'*/
void AtollUart6_StringSend(const char *pucBuffer);

/*Function used to enable the 4 digital inuts,inx_state shoud be '1' to enable the hardware pin  */
/*for the each input else inx_state shoud be '0'   */
void Atoll_gpio_input_init(char in1_state,char in2_state,char in3_state,char in4_state);

/*function used to read the digital input status of a specific pin,value of gpiopin should be 1 for*/
/*digital in1, 2 for digital in2,3 for digitalin3 and 4 for digital in 4  , thhis function will return the */
/*intiger value 1 if the input is high state(6.5v to 12v) and will return the value 0 if in low state(below 6.5v)*/
int Atoll_GpioinputRead(char gpiopin);

/*function used to enable the hardware pin for the 4digital output,outputpinx should be 1 to enable and should be 0 to disable*/
void Atoll_gpio_out_init(char outputpin1,char outputpin2,char outputpin3,char outputpin4);

/*function used to activate the open collector output individually,outputpin shoud be 1 foe ditial out1, 2 for digital out 2*/
/*3 for difital ot3 and 4 for digital out4 , the parameter 'value' shoud be 1  activate the open collector output and shoud be 0 to deactivate */
void Atoll_outputWrite(char outputpin,char value);

/*function used to give the delay,delayval,10 means around 1 sec*/
void Mydelay(int delayval);

/*function used to enable the hardware pin for ADC inputs,adcinx should be 1 to enable the adc input and should be 0 to disable adc*/
void Atoll_ADC_Init(char adcin1,char adcin2,char adcin3,char adcin4);

/*this function will return the adc value for the corresponding analog input,adc_inputpin should be 1 for analog in1 and 2 for analog in2*/
/*3 for analog in 3 and 4 for analog in4*/
int AtollAdcread(char adc_inputpin);

