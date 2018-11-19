/*
 * LoRaWAN_packet.h
 *
 *  Created on: Jul 3, 2018
 *      Author: hyhlik
 */

#ifndef LORAWAN_PACKET_H_
#define LORAWAN_PACKET_H_

/*------------------- INCLUDES -----------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "aes.h"

/*------------------- ENUMS -----------------------*/

/* LoRaWAN message type obtained from MHDR */
typedef enum {
	Join_request, Join_accept, Unconfirmed_Data_Up, Unconfirmed_Data_Down,
	Confirmed_Data_Up, Confirmed_Data_Down, Rejoin_request, Proprietary
} Mtype_t;


/*------------------- DEFINES -----------------------*/


/* size aka number of bytes */
#define SIZE_DEVADDR		4
#define SIZE_KEY			16

#define SIZE_MIC			4

#define SIZE_BLOCK			16
//#define PAYLOAD_MAXLEN		256


/* positions in the packet data array */
#define POS_MHDR			0
#define POS_DEVADDR			1
#define POS_FCTRL			POS_DEVADDR  + SIZE_DEVADDR
#define POS_FCNT			POS_FCTRL +1

#define POS_FOPTS			8
#define POS_FPORT			8

/* max sizes */
#define MAXSIZE_FOPTS		16


/* default device's configuration*/
static const uint8_t NwSKey_default[] =
{ 0xFD, 0x90, 0x0D, 0x8C, 0x70, 0x9F, 0x19, 0x24, 0x18, 0xEC, 0xFD, 0xD4, 0x28, 0x0C, 0xAC, 0x47};
static const uint8_t AppSKey_default[]	=
{ 0x68, 0x9F, 0xD0, 0xAC, 0x7A, 0x0F, 0x95, 0x58, 0xB1, 0x19, 0xA0, 0x16, 0x17, 0xF4, 0x16, 0x33};
static const uint8_t devAddr_default[] = 
{ 0x12, 0x34, 0x56, 0x78};


/*------------------- STRUCTURES -----------------------*/

/// LoRaWAN substructure
typedef struct {
	uint8_t devAddr[SIZE_DEVADDR];	// Device address
	uint8_t FCtrl;					// Frame control
	uint16_t FCnt;					// Frame counter
	uint8_t* FOpts;					// Frame Options
	uint8_t FOpts_len;
} FHDR_t;

/// LoRaWAN substructure
typedef struct {
	FHDR_t FHDR;					// Frame Header
	uint8_t FPort;					// Frame port
	uint8_t* FRMPayload;			// encrypted app message
	uint8_t FRMPayload_len;
} MAC_t;

/// LoRaWAN substructure
typedef struct {
	uint8_t MHDR;           		// MAC header
	MAC_t MAC;
	uint8_t MIC[SIZE_MIC];			// Message Integrity Check (calculated using NwSKey)
} PHY_t;

/// main LoRaWAN structure
typedef struct {
	PHY_t PHY;				// PHY payload in structure

	uint8_t* rawData;       // the raw packet bytes
	uint8_t rawData_len;

	uint8_t* appMessage;	// decrypted app message
	uint8_t appMessage_len;

	uint8_t ack;            // acknowlegment
	uint8_t adr;            // adaptive data rate
	uint8_t direction;      // 0 = up, 1 = down

	Mtype_t Mtype;
} LoRaWAN_packet;


// LoRaWAN node
typedef struct {
	uint8_t devAddr[SIZE_DEVADDR];
	uint8_t NwSKey[SIZE_KEY];
	uint8_t AppSKey[SIZE_KEY];
	uint16_t FCnt;
} LoRaWAN_node;


/*------------------- DECLARATIONS -----------------------*/

void LWp_delete(LoRaWAN_packet* p);

LoRaWAN_packet LWp_analyze(uint8_t* packet, uint8_t packet_len);
LoRaWAN_packet LWp_make(uint8_t* data, uint8_t data_len, LoRaWAN_node* n);

void LWp_printInfo(LoRaWAN_packet p);
void LWp_xcrypt(uint8_t* out, uint8_t* payload, uint8_t payload_len,
		const uint8_t* key, uint8_t* devAddr, uint8_t dir, uint16_t FCnt);
bool LWp_decrypt(LoRaWAN_packet* p, const uint8_t* NwSKey, const uint8_t* AppSKey);
uint8_t* LWp_calcMIC(LoRaWAN_packet* p, const uint8_t* NwSKey);



#endif /* LORAWAN_PACKET_H_ */
