#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "main.h"
#include "aes.h"
#include "cmac.h"
#include "LoRaWAN_packet.h"
#include "ByteArray.h"



////////////////////////////////////////////////////////////////////////////
/* @brief analyze LoRaWAN packet, the structure LoRaWAN_packet is filled with
 * data obtained from the packet string, even decrypted data
 * @return pointer to dynamically alocated LoRaWAN_packet structure
 */
LoRaWAN_packet LWp_analyze(uint8_t* packet, uint8_t packet_len)
{
	LoRaWAN_packet p;

	/* now fill the whole LoRaWAN_packet structure from the payload array */
	p.rawData_len = packet_len;
	p.rawData = malloc(p.rawData_len);
	if(p.rawData != NULL){
		memcpy(p.rawData, (uint8_t*)packet, p.rawData_len);
	}

	/* PHY struct*/
	p.PHY.MHDR = p.rawData[POS_MHDR];
	uint8_t mic_pos = p.rawData_len - SIZE_MIC;
	memcpy(p.PHY.MIC, &p.rawData[mic_pos], SIZE_MIC);

	/* FHDR struct */
	memcpy(p.PHY.MAC.FHDR.devAddr, &p.rawData[POS_DEVADDR], SIZE_DEVADDR);
	p.PHY.MAC.FHDR.FCtrl = p.rawData[POS_FCTRL];
	p.PHY.MAC.FHDR.FCnt = (uint16_t)((p.rawData[POS_FCNT +1] << 8) | p.rawData[POS_FCNT]);
	memcpy(p.PHY.MAC.FHDR.devAddr, &packet[POS_DEVADDR], SIZE_DEVADDR);

	p.PHY.MAC.FHDR.FOpts_len = p.PHY.MAC.FHDR.FCtrl & 0x0F;
	if(p.PHY.MAC.FHDR.FOpts_len != 0 && p.PHY.MAC.FHDR.FOpts_len <= MAXSIZE_FOPTS){
		p.PHY.MAC.FHDR.FOpts = malloc(p.PHY.MAC.FHDR.FOpts_len +1);
		if(p.PHY.MAC.FHDR.FOpts != NULL){
			memcpy(p.PHY.MAC.FHDR.FOpts, &p.rawData[POS_FOPTS], p.PHY.MAC.FHDR.FOpts_len);
		}
	} else {
		p.PHY.MAC.FHDR.FOpts = NULL;
	}

	/* MAC struct */
	p.PHY.MAC.FPort = p.rawData[POS_FPORT + p.PHY.MAC.FHDR.FOpts_len];
	uint8_t FRMPayload_pos = POS_FOPTS + 1 + p.PHY.MAC.FHDR.FOpts_len;
	p.PHY.MAC.FRMPayload_len =  p.rawData_len - FRMPayload_pos - SIZE_MIC;

	p.PHY.MAC.FRMPayload = NULL;
	p.PHY.MAC.FRMPayload = malloc(p.PHY.MAC.FRMPayload_len);
	if (p.PHY.MAC.FRMPayload != NULL){
		memcpy(p.PHY.MAC.FRMPayload, &p.rawData[FRMPayload_pos], p.PHY.MAC.FRMPayload_len);
	}

	/* other */
	if((p.PHY.MAC.FHDR.FCtrl & 0x20) == 0x20) p.ack = 1; else p.ack = 0;
	if((p.PHY.MAC.FHDR.FCtrl & 0x80) == 0x80) p.adr = 1; else p.adr =  0;

	/* Mtype - obtained from MHDR */
	p.Mtype = (p.PHY.MHDR >> 5) & 7;
	switch(p.Mtype)
	{
		case Unconfirmed_Data_Up:
		case Confirmed_Data_Up:
			p.direction = 0;
			break;
		case Unconfirmed_Data_Down:
		case Confirmed_Data_Down:
			p.direction = 1;
		case Join_request:
		case Join_accept:
		case Rejoin_request:
		case Proprietary:
			break;
	}
	


	/* appMessage is the same length as the ten of encrypted payload */
	p.appMessage_len = p.PHY.MAC.FRMPayload_len;
	p.appMessage = NULL;

	return (p);
}

////////////////////////////////////////////////////////////////////////////
/* @brief create packte to be transmitted with some default parameters
 * @param data is the data to be put in the LoRaWAN packet
 * @param n is node containing devAddr and encryption keys
 * @algorithm: create LoRaWAN_packet struct, fill it with the default values and devAddr,
 * 		encrypt the message wit the node's AppKey, fill the packet rawData payload,
 * 		calculate MIC and add it to the rawData payload. Return the packet structure.
 * 		the payload array p->rawData can be now sent via LoRa RF interface
 */
LoRaWAN_packet LWp_make(uint8_t* data, uint8_t data_len, LoRaWAN_node* n, Mtype_t mtype)
{
	LoRaWAN_packet p;

	/* ad the clean message to the struct */
	p.appMessage_len = data_len;
	p.appMessage = malloc(p.appMessage_len);
	if(p.appMessage != NULL)
		memcpy(p.appMessage, data, p.appMessage_len);

	/* At first fill the PHY structure with substructures */

	/* FHDR */
	memcpy(p.PHY.MAC.FHDR.devAddr, n->devAddr, SIZE_DEVADDR);
	p.PHY.MAC.FHDR.FCtrl = 0x40;
	if (n->adr) {
		p.PHY.MAC.FHDR.FCtrl |= 0x80;
	}
	if (n->ack) {
		p.PHY.MAC.FHDR.FCtrl |= 0x20;
	}
	p.PHY.MAC.FHDR.FCnt = n->FCnt++;		// !! incrementing frame counter !!
	p.PHY.MAC.FHDR.FOpts = NULL;
	p.PHY.MAC.FHDR.FOpts_len = 0;
	/* MAC */
	p.PHY.MAC.FPort = 0x08;
	p.PHY.MAC.FRMPayload_len = data_len;
	/* PHY */
	p.Mtype = mtype;
	p.PHY.MHDR = (0x00 | (p.Mtype << 5));
	switch(p.Mtype)
	{
	case Unconfirmed_Data_Up:
	case Confirmed_Data_Up:
		p.direction = 0;
		break;
	case Unconfirmed_Data_Down:
	case Confirmed_Data_Down:
		p.direction = 1;
	case Join_accept:
	case Rejoin_request:
	case Proprietary:
	case Join_request:
		break;	
	}

	if((p.PHY.MAC.FHDR.FCtrl & 0x20) == 0x20) p.ack = 1; else p.ack = 0;
	if((p.PHY.MAC.FHDR.FCtrl & 0x80) == 0x80) p.adr = 1; else p.adr =  0;

	/* frame payload (encrypted message) */
	p.PHY.MAC.FRMPayload = malloc(data_len);
	if(p.PHY.MAC.FRMPayload != NULL){
		LWp_xcrypt(p.PHY.MAC.FRMPayload, p.appMessage, p.PHY.MAC.FRMPayload_len,
				n->AppSKey, p.PHY.MAC.FHDR.devAddr, p.direction, p.PHY.MAC.FHDR.FCnt);
	}

	/* now fill the rawData from these data in the PHY structure (without MIC) */
	uint8_t FRMPayload_pos;			// position of frm payload
	FRMPayload_pos = POS_FOPTS + 1 + p.PHY.MAC.FHDR.FOpts_len;	
	p.rawData_len = FRMPayload_pos + p.appMessage_len + SIZE_MIC;
	p.rawData = malloc(p.rawData_len);
	if(p.rawData != NULL){
		p.rawData[POS_MHDR] = p.PHY.MHDR;
		memcpy(&p.rawData[POS_DEVADDR], p.PHY.MAC.FHDR.devAddr, SIZE_DEVADDR);
		p.rawData[POS_FCTRL] = p.PHY.MAC.FHDR.FCtrl;
		memcpy(&p.rawData[POS_FCNT], &p.PHY.MAC.FHDR.FCnt, sizeof(p.PHY.MAC.FHDR.FCnt));
		p.rawData[POS_FPORT + p.PHY.MAC.FHDR.FOpts_len] = p.PHY.MAC.FPort;
		memcpy(&p.rawData[FRMPayload_pos], p.PHY.MAC.FRMPayload, p.PHY.MAC.FRMPayload_len);
	}

	/* At last, calculate the MIC and add it to the structure and the rawData */
	memcpy(p.PHY.MIC, LWp_calcMIC(&p, n->NwSKey), SIZE_MIC);
	memcpy(&p.rawData[p.rawData_len - SIZE_MIC], p.PHY.MIC, SIZE_MIC);

	return p;
}
////////////////////////////////////////////////////////////////////////////
/* @brief Prints the info from the LoRaWAN_packet structure
 */
void LWp_printInfo(LoRaWAN_packet p)
{
	printf("Message type: ");
	switch(p.Mtype)
	{
		case Join_request:
			printf("Join-request\n");
			break;
		case Join_accept:
			printf("Join-accept\n");
			break;
		case Unconfirmed_Data_Up:
			printf("Unconfirmed Data Up\n");
			break;
		case Unconfirmed_Data_Down:
			printf("Unconfirmed Data Down\n");
			break;
		case Confirmed_Data_Up:
			printf("Confirmed Data Up\n");
			break;
		case Confirmed_Data_Down:
			printf("Confirmed Data Down\n");
			break;
		case Rejoin_request:
			printf("Rejoin-request\n");
			break;
		case Proprietary:
			printf("Proprietary\n");
			break;
		default:
			printf("Unknown: %X\n", p.Mtype);
	}

	if(p.rawData != NULL)
		pHex_tit("Packet rawData: ", p.rawData, p.rawData_len);

	/* device*/
	pHex_tit("Device Address: ", p.PHY.MAC.FHDR.devAddr, SIZE_DEVADDR);
	printf("FCnt: %u\n", p.PHY.MAC.FHDR.FCnt);

	/* message data */
	if(p.PHY.MAC.FRMPayload != NULL){
		pHex_tit("message (encrypted): ", p.PHY.MAC.FRMPayload, p.PHY.MAC.FRMPayload_len);
	}
	if(p.appMessage != NULL){
		pHex_tit("message (decrypted): ", p.appMessage, p.appMessage_len);
		printf("message (decrypted ASCII): ");
		pAscii_nl(p.appMessage, p.appMessage_len);
	}

	printf("MHDR: %.2X; ", p.PHY.MHDR);
	printf("FCtrl: %.2X; ", p.PHY.MAC.FHDR.FCtrl);
	printf("FPort: %.2x; ", p.PHY.MAC.FPort);
	pHex_tit("MIC: ", p.PHY.MIC, SIZE_MIC);

	if(p.PHY.MAC.FHDR.FOpts != NULL)
		pHex_tit("FOpts: ", p.PHY.MAC.FHDR.FOpts, p.PHY.MAC.FHDR.FOpts_len);


	printf("adaptive data rate: ");
	if(p.adr)	printf("true"); else 	printf("false");
	printf("; ack: ");
	if(p.ack)	printf("true");	else	printf("false");

	printf("\r\n\n");
}
////////////////////////////////////////////////////////////////////////////
/*
 * @brief delete the LoRaWAN_packet structure
 */
void LWp_delete(LoRaWAN_packet* p)
{
	free(p->PHY.MAC.FHDR.FOpts);
	free(p->PHY.MAC.FRMPayload); 	// this causes error
	free(p->appMessage);
	free(p->rawData);
	p->PHY.MAC.FHDR.FOpts = NULL;
	p->PHY.MAC.FRMPayload = NULL;
	p->appMessage = NULL;
	p->rawData = NULL;
}
////////////////////////////////////////////////////////////////////////////
/*
 * @brief decrypt the payload (fill the p.appMessage buffer)
 * @note using the keys in the systemDonfig struct
 * @param pointer to the packet
 */
bool LWp_decrypt(LoRaWAN_packet* p, const uint8_t* NwSKey, const uint8_t* AppSKey)
{
	/* MIC calculation */
	if(!arraysEqual( p->PHY.MIC, LWp_calcMIC(p, NwSKey), SIZE_MIC)){
		printf("Decryption error: Invalid MIC.\n");
		return false;
	}

	/* App message decryptin */
	/* Select encryption key depending on the FPort, normally it's AppSKey */
	uint8_t usedKey[SIZE_KEY];
	if(p->PHY.MAC.FPort > 0){
		memcpy(usedKey, AppSKey, SIZE_KEY);
	} else {
		memcpy(usedKey, NwSKey, SIZE_KEY);
	}

	p->appMessage = calloc(sizeof(uint8_t), p->appMessage_len);
	/* decrypt message */
	if(p->appMessage != NULL){
		LWp_xcrypt(p->appMessage, p->PHY.MAC.FRMPayload, p->PHY.MAC.FRMPayload_len,
				usedKey, p->PHY.MAC.FHDR.devAddr, p->direction, p->PHY.MAC.FHDR.FCnt);
	} else {
		printf("ERROR: %s, 3\n", __func__);
		return false;
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////
/*
 * @brief calculation of MIC using NwSKey and check if it equals to the MIC in 
 * 		packet payload
 * @return pointer to the calculated mic
 */
uint8_t* LWp_calcMIC(LoRaWAN_packet* p, const uint8_t* NwSKey)
{
	// printf("\nCalculating MIC\n");

	/* create msg buffer */
	uint8_t msg_len = p->rawData_len - SIZE_MIC;
	uint8_t* msg = malloc(msg_len);
	memcpy(msg, p->rawData, msg_len);
	// printf("msg: ");
	// pHex_nl(msg, msg_len);

	/* create B0 buffer */
	uint8_t B0[SIZE_BLOCK];
	memset(B0, 0, sizeof(B0));
	B0[0] = 0x49;
	B0[5] = p->direction;
	memcpy(&B0[6], p->PHY.MAC.FHDR.devAddr, SIZE_DEVADDR);
	memcpy(&B0[10], &p->PHY.MAC.FHDR.FCnt, sizeof(p->PHY.MAC.FHDR.FCnt));
	B0[15] = msg_len;
	// printf("B0: ");
	// pHex_nl(B0, sizeof(B0));

	/* buffer for B0 | msg */
	uint8_t buff_len = msg_len + sizeof(B0);
	uint8_t* buff = malloc(buff_len);
	memcpy(buff, B0, sizeof(B0));
	memcpy(&buff[sizeof(B0)], msg, msg_len);
	// printf("macBuff ");
	// pHex_nl(buff, buff_len);	

	/* buffer for the AES output */
	uint8_t mac[SIZE_BLOCK];

	/* coppy key */
	uint8_t key[SIZE_KEY];
	memcpy(key, NwSKey, SIZE_KEY);

	/* encrypt */
	AES_CMAC(key, buff, (int)buff_len,  mac);
	// printf("MAC: ");
	// pHex_nl(mac, sizeof(mac));

	/* the calculated MIC is first 4 bytes of MAC */
	static uint8_t calculatedMIC[SIZE_MIC];
	memcpy(calculatedMIC, mac, SIZE_MIC);

	// pHex_nl(calculatedMIC, SIZE_MIC);
	// pHex_nl(p->PHY.MIC, SIZE_MIC);

	free(buff);
	free(msg);
	
	return calculatedMIC;
}
////////////////////////////////////////////////////////////////////////////
/* @brief encrypt/decrypt algorithm of App message
 * @parameters are values needed for the encryption and out is the output buffer
 */
void LWp_xcrypt(uint8_t* out, uint8_t* payload, uint8_t payload_len,
		const uint8_t* key, uint8_t* devAddr, uint8_t dir, uint16_t FCnt)
{
	uint8_t S[16];
	uint8_t A[16];
	uint8_t out_buffer[payload_len + 10];

	/* fill the A block */
	memset(A, 0, sizeof(A));
	A[0] = 1;
	A[5] = dir;
	A[6] = devAddr[0];
	A[7] = devAddr[1];
	A[8] = devAddr[2];
	A[9] = devAddr[3];
	A[10] = FCnt & 0xFF;
	A[11] = (FCnt >> 8) & 0xFF;

	// printf("A: ");
	// pHex_nl(A, 16);

  double k = ceil((double)payload_len /16);
  /* for each 16-Byte block of payload, A is the block to be encrypted,
  stream S is the output which will be xored with the payload */
  for(int i = 1 ; i <= k ; i++)
  {
    A[15] = i;
    AES128_ECB_encrypt(A, key, S);

    for(int j = 0 ; j < 16 ; j++){
    	out_buffer[j+(16*(i-1))] = payload[j+(16*(i-1))] ^ S[j];
    }
  }
  memcpy(out, out_buffer, payload_len);
}
////////////////////////////////////////////////////////////////////////////


