/*
 * service.c
 *
 *  Created on: 26 Mar 2022
 *      Author: Juan Gomez
 */
#include <stdio.h>
#include <stdlib.h>
#include "bluenrg1_gap.h"
#include "bluenrg1_gatt_aci.h"
#include "service.h"

//custom UUID service
uint8_t SERVICE_UUID[16] = {0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x9e, 0xb1, 0xe0, 0xf2, 0x73, 0xd9};
uint8_t CHAR_UUID[16] = {0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x9e, 0xb1, 0xe1, 0xf2, 0x73, 0xd9};

uint16_t my_service_handle, my_char_handle;

uint8_t connected = 0;
uint8_t set_connectable = 1;
uint16_t connection_handle = 0;
uint8_t notification_enabled = 0;

tBleStatus add_siemple_service(void){

	tBleStatus ret;
	Service_UUID_t service_uuid;
	Char_UUID_t char_uuid;

	BLUENRG_memcpy(service_uuid.Service_UUID_128, SERVICE_UUID, 16);

	//add service
	ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE, 7, &my_service_handle);

	BLUENRG_memcpy(char_uuid.Char_UUID_128, CHAR_UUID, 16);

	// add custom characteristic
	ret = aci_gatt_add_char(my_service_handle, UUID_TYPE_128, &char_uuid, 2, CHAR_PROP_READ, ATTR_PERMISSION_NONE,
											GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP, 0, 0, &my_char_handle);

	return ret;
}

void update_data(int16_t new_data){

	tBleStatus ret;

	//update characteristic value
	ret = aci_gatt_update_char_value(my_service_handle, my_char_handle, 0, 2, (uint8_t*)&new_data);
	if(ret != BLE_STATUS_SUCCESS){
		printf("aci_gatt_update_char_value : FAILED \n\r");
	}
}

/* These functions are depreceated
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle){

	connected = 1;
	connection_handle = handle;

	prinf("Connection complete \n\r");
}

void GAP_DisconnectionComplete_CB(void){

	prinf("Disconnection complete \n\r");
}
*/

void Read_Request_CB(uint16_t handle){

	int data;
	//data = 450;

	data = 450 + ((uint64_t)rand() * 100) / 1000;

	update_data(data);

	if(connection_handle != 0){

		aci_gatt_allow_read(connection_handle);
	}
}

void aci_gatt_read_permit_req_event(uint16_t Connection_Handle, uint16_t Attribute_Handle, uint16_t Offset){

	Read_Request_CB(Attribute_Handle);
}

void hci_le_connection_complete_event(uint8_t Status, uint16_t Connection_Handle, uint8_t Role, uint8_t Peer_Address_Type,
																	uint8_t Peer_Address[6], uint16_t Conn_Interval, uint16_t Conn_Latency,
																	uint16_t Supervision_Timeout, uint8_t Master_Clock_Accuracy)
{
	connected = 1;
	connection_handle = Connection_Handle;
}

void hci_disconnection_complete_event(uint8_t Status, uint16_t Connection_Handle, uint8_t Reason){

	connected = 1;
	set_connectable = 1;
	connection_handle = 0;

	printf("Disconnected \n\r");
}

void APP_UserEvRx(void *pData){

	uint32_t i;

	hci_spi_pckt *hci_pckt = (hci_spi_pckt*)pData;

	//process event packet
	if(hci_pckt->type == HCI_EVENT_PKT){

		//Get data from packet
		hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

		//process meta data
		if(event_pckt->evt == EVT_LE_META_EVENT){

			//get meta data
			evt_le_meta_event *evt = (void*)event_pckt->data;

			//process each meta data event
			for(i = 0; i < (sizeof(hci_le_meta_events_table) / sizeof(hci_le_meta_events_table_type)); i++){

				if(evt->subevent == hci_le_meta_events_table[i].evt_code){

					hci_le_meta_events_table[i].process((void*)evt->data);
				}
			}
		}

		//process vendor event
		else if(event_pckt->evt == EVT_VENDOR){

			//get meta data
			evt_blue_aci *blue_evt = (void*)event_pckt->data;

			//process each vendor event
			for(i = 0; i < (sizeof(hci_vendor_specific_events_table) / sizeof(hci_vendor_specific_events_table_type)); i++){

				if(blue_evt->ecode == hci_vendor_specific_events_table[i].evt_code){

					hci_vendor_specific_events_table[i].process((void*)blue_evt->data);
				}
			}
		}

		//process other events
		else{

			for(i = 0; i < (sizeof(hci_events_table) / sizeof(hci_events_table_type)); i++){

				if(event_pckt->evt == hci_events_table[i].evt_code){

					hci_events_table[i].process((void*)event_pckt->data);
				}
			}
		}
	}
}












