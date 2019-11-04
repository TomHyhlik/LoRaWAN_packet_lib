#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "LoRaWAN_packet.h"
#include "ByteArray.h"
#include "LinkedList_ByteArray.h"


void test_parse_rx_pkt();
void test_create_tx_pkt();


//////////////////////////////////////////////////////////
int main(void)
{
	printf("AppStart...........................................\n");

	test_create_tx_pkt();
	test_parse_rx_pkt();

	printf("AppEND..........................................\n");
	return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////
/*
 * @brief this function demonstrates how node creates a LW 
* 		packet that is ready to be transmitted via radio
 * @note at first must be initialized the node's devAddr and
 * 		encryption keys in the LoRaWAN_node structure
 */
void test_create_tx_pkt()
{
	printf("\nCreating a node's packet\n");

	/* init the node's LW parameters */
	LoRaWAN_node n;
	convert_ASCIIbyteHexString_to_byteArray(n.devAddr, "F61F0126");
	convert_ASCIIbyteHexString_to_byteArray(n.NwSKey, "FD900D8C709F192418ECFDD4280CAC47");
	convert_ASCIIbyteHexString_to_byteArray(n.AppSKey, "689FD0AC7A0F9558B119A01617F41633");
	n.adr = false;
	
	/* create the node's message   */
	char* nodePayload = "This is sample node payload.";
	LoRaWAN_packet p = LWp_make((uint8_t*)nodePayload, strlen(nodePayload), &n, Unconfirmed_Data_Up);

	/* show info about the created packet */
	LWp_printInfo(p);

	printf("This is wrapped LW packet data willing to be transmitted via radio:\n");
	pHex_nl(p.rawData, p.rawData_len);

	/* there is used also dynamic memory alocation, so it must be deleted */
	LWp_delete(&p);	
}

//////////////////////////////////////////////////////////
/*
 * @brief this function demonstrates parsing of received LW packet
 * 		by LW gateway
 */
void test_parse_rx_pkt()
{
	printf("Demonstration of parsing received LW packets by gateway\n");

	/* add known LW nodes */ 
	int LoRaWAN_node_count = 2;
	LoRaWAN_node n[LoRaWAN_node_count];
	convert_ASCIIbyteHexString_to_byteArray(n[0].devAddr, "F61F0126");
	convert_ASCIIbyteHexString_to_byteArray(n[0].NwSKey, "FD900D8C709F192418ECFDD4280CAC47");
	convert_ASCIIbyteHexString_to_byteArray(n[0].AppSKey, "689FD0AC7A0F9558B119A01617F41633");
	convert_ASCIIbyteHexString_to_byteArray(n[1].devAddr, "F17DBE49");
	convert_ASCIIbyteHexString_to_byteArray(n[1].NwSKey, "44024241ed4ce9a68c6a8bc055233fd3");
	convert_ASCIIbyteHexString_to_byteArray(n[1].AppSKey, "ec925802ae430ca77fd3dd73cb2cc588");

	/* add received packet samples to list */
	LinkedList_ByteArray* rx_list = NULL;
	rx_list = ba_append_str(rx_list, "40F61F0126C0EA010884A70201A99C7CE77A4B5EA1B1");
	rx_list = ba_append_str(rx_list, "40F17DBE4900020001954378762B11FF0D");
	// rx_list = ba_append_str(rx_list, "A8BB55DC1258FCB2E8828E1B459101C19ACD");

	/* process the list of received packets */
	while(rx_list != NULL)
	{
		LoRaWAN_packet p = LWp_analyze(rx_list->data, rx_list->length);
		LWp_printInfo(p);

		/* check if the devAddr is known */
		for (int i = 0; i < LoRaWAN_node_count; i++) {
			if (arraysEqual(n[i].devAddr, p.PHY.MAC.FHDR.devAddr, SIZE_DEVADDR)) {

				/* the node is known, now decrypt the payload */
				if(LWp_decrypt(&p, n[i].NwSKey, n[i].AppSKey)){
					pHex_tit("message (decrypted): ", p.appMessage, p.appMessage_len);
				} else {
					printf("ERROR: Failed to decrypt payload\n");
				}
				break;
			}
		}

		printf("\n");


		LWp_delete(&p);
		rx_list = ba_removeFirst(rx_list);
	}
}

