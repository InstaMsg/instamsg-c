/*
 * gsm.h
 *
 *  Created on: 11-Aug-2015
 *      Author: namees
 */

#ifndef GSM_H_
#define GSM_H_
/*GSM related API's*/

/*This function enables the hardware pins for the gsm module in the AR501R */
void AtollGSMperipheral_init(void);

/*function used to send a string in to the gsm uart with delay, the string should end with '\0', pucBuffer is the pointer for the string*/
/* and delay use to give the delay(delay =10 for 1 sec) */
/*all commands to the gsm should end with '\r' */
void AtollGSM_StringSend(const char *pucBuffer,short delay);

/*function used to read the reply from the gsm module with a delay*/
/*the replay from the gsm module will copy in to the destinationbuff after the delay given*/
void AtollGSM_stringRead(char destinationbuff[],short delay);

/*api used to activate the gprs connection, should give the apn of the network as input*/
void AtollGPRS_activate(char apn[]);

/**/
int Aroll_gprs_activate(char apn[]);

int Atoll_socketdial(char ipaddress[],char port[]);

int Atoll_socketDisconnect(void);

//int Atoll_Socketdial(char ip_address[],char port[]);

#endif /* GSM_H_ */
