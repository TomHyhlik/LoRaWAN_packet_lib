#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "LoRaWAN_packet.h"
#include "ByteArray.h"
#include "LinkedList_ByteArray.h"



//////////////////////////////////////////////////////////
void test_create_tx_pkt()
{
	/* create node */
	LoRaWAN_node n;
	convert_ASCIIbyteHexString_to_byteArray(n.devAddr, "F61F0126");
	convert_ASCIIbyteHexString_to_byteArray(n.NwSKey, "FD900D8C709F192418ECFDD4280CAC47");
	convert_ASCIIbyteHexString_to_byteArray(n.AppSKey, "689FD0AC7A0F9558B119A01617F41633");

	/* create the packet structure */
	uint8_t* message = "01234567890123456789abcdefghijklmnopqrstuvz";
	LoRaWAN_packet p = LWp_make(message, strlen(message), &n);
	LWp_printInfo(p);
	/* todo */

	LWp_delete(&p);							// delete the packet
}

//////////////////////////////////////////////////////////
/*
 * @brief this function takes examples of LoRaWAN packets
 * 	and decodes and decrypts it
 */
void test_anl_rx_pkt()
{
	int LoRaWAN_node_count = 2;
	LoRaWAN_node n[LoRaWAN_node_count];
	LinkedList_ByteArray* rx_list = NULL;

	/* add known nodes */ 
	convert_ASCIIbyteHexString_to_byteArray(n[0].devAddr, "F61F0126");
	convert_ASCIIbyteHexString_to_byteArray(n[0].NwSKey, "FD900D8C709F192418ECFDD4280CAC47");
	convert_ASCIIbyteHexString_to_byteArray(n[0].AppSKey, "689FD0AC7A0F9558B119A01617F41633");
	convert_ASCIIbyteHexString_to_byteArray(n[1].devAddr, "F17DBE49");
	convert_ASCIIbyteHexString_to_byteArray(n[1].NwSKey, "44024241ed4ce9a68c6a8bc055233fd3");
	convert_ASCIIbyteHexString_to_byteArray(n[1].AppSKey, "ec925802ae430ca77fd3dd73cb2cc588");

	/* add packets raw data to list */
	rx_list = ba_append_str(rx_list, "40F61F0126C0EA010884A70201A99C7CE77A4B5EA1B1");
	rx_list = ba_append_str(rx_list, "40F17DBE4900020001954378762B11FF0D");
	// rx_list = ba_append_str(rx_list, "A8BB55DC1258FCB2E8828E1B459101C19ACD");

	/* process these list of packets */
	/* while the list is not empty */
	while(rx_list != NULL)
	{
		/* put the packet raw data into structure */
		LoRaWAN_packet p = LWp_analyze(rx_list->data, rx_list->length);
		LWp_printInfo(p);

		/* check which devAddr it is */
		for (int i = 0; i < LoRaWAN_node_count; i++){
			if (arraysEqual(n[i].devAddr, p.PHY.MAC.FHDR.devAddr, SIZE_DEVADDR)){
				/* we found the devAddr, now decrypt with apropriate keys... */
				if(LWp_decrypt(&p, n[i].NwSKey, n[i].AppSKey)){
					pHex_tit("message (decrypted): ", p.appMessage, p.appMessage_len);
				} else {
					printf("ERROR: Unable to decrypt payload\n");
				}
				break;
			}
		}

		LWp_delete(&p);							// delete the packet
		rx_list = ba_removeFirst(rx_list);		// go to the next packet
		printf("\n");
	}
	
}

//////////////////////////////////////////////////////////
int main(void)
{
	printf("AppStart...........................................\n");

	test_anl_rx_pkt();
	// test_create_tx_pkt();

	printf("AppEND..........................................\n");
	return EXIT_SUCCESS;
}



// expected decrypted app messages:
/* decrypted data: 010E6D7E0500FFFF7E */
/* decrypted data: 74657374 */


/*
Rx -> LoRaWAN, pktCntr: 13
RSSI: -33, SNR: 9, length: 22
Packet raw:  40 F6 1F 01 26 C0 8F 05 08 91 82 71 72 4B C6 59 7D 79 83 B0 B2 26
devAddr:  F6 1F 01 26
MHDR: 40
FCtrl: c0
FOpts len: 00
FCnt:  8F 05
adaptive data rate: 1
ack: 0
FOpts:
FPort: 08
direction: UP
MIC:  83 B0 B2 26
Payload (encrypted):  91 82 71 72 4B C6 59 7D 79
Payload (decrypted):  01 0E 6D 7E 05 00 FF FF 7E
Sensor type: RHF1S001
temperature: 28.0 C, humidity: 55 %
period: 10 s, RSSI: -33 dBm, SNR: 9 dB, battery voltage: 2.7 V
*/
