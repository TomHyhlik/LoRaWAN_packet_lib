#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ByteArray.h"
#include "LinkedList_ByteArray.h"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
LinkedList_ByteArray* ba_append_str(LinkedList_ByteArray* ba, char* newData)
{
	int data_len = strlen(newData) / 2;
	uint8_t* data = malloc(data_len);

	int stat = convert_ASCIIbyteHexString_to_byteArray(data, newData);
	if(stat != -1){
		ba = ba_append(ba, data, data_len);
	}
	free(data);

	return ba;
}

////////////////////////////////////////////////////////////////////////////////
/* @brief append new element to the end of list
 * @param ba is the list where the element will be appended
 * @param newData is pointer to array which will be added to the last element of the list
 * @param size is the length of the array
 * @return pointer to the first element of the list
 */
LinkedList_ByteArray* ba_append(LinkedList_ByteArray* ba, uint8_t* newData, uint8_t size)
{
	if(ba == NULL){
		ba = ba_prepend(ba, newData, size);
		return ba;
	}

//    printf("len: %d, Append: ", (int)size);
//    pHex_nl(newData, size);

	/* get to the last node and it's pointer */
	LinkedList_ByteArray* cursor = ba;

	while(cursor->next != NULL)
		cursor = cursor->next;

	/* create a new node and put the data in there	*/
	LinkedList_ByteArray* newByteArray = ba_CreateNewArray(newData, size, NULL);

	cursor->next = newByteArray;

	return ba;
}


////////////////////////////////////////////////////////////////////////////////
/*
 * @brief append array if it's not included in the list already
 */
LinkedList_ByteArray* ba_appendIfNotIncluded(LinkedList_ByteArray* ba, uint8_t* newData, uint8_t size)
{
	int position = ba_find(ba, newData, size);

	if(position == -1){
		ba = ba_append(ba, newData, size);
	}
	return ba;
}

////////////////////////////////////////////////////////////////////////////////
/* @brief prepend new element to the list
 * @param ba is the list where the new element will be appended
 * @param newData points to array which is put to the element
 * @return the new first element of the list
 * @param size is the length of the array and is also copied to the element
 * @ATTENTION this function changes the starting element of the list,
 * 			so it must be called as following example bellow:
 * 			ba = ba_removeFirst(ba);
 */
LinkedList_ByteArray* ba_prepend(LinkedList_ByteArray* ba, uint8_t* newData, uint8_t size)
{
	LinkedList_ByteArray* newArray = ba_CreateNewArray(newData, size, ba);
	if(newArray != NULL){
		ba = newArray;
	} else {
		printf("ERROR: %s\r\n", __func__);

	}
	return ba;
}

////////////////////////////////////////////////////////////////////////////////
/* @note this function should not be used by user, but only by this lib's code
 * @brief create a new element of the list
 * @return pointer to the first element of the list
 */
LinkedList_ByteArray* ba_CreateNewArray(uint8_t* newData, uint8_t newData_len, LinkedList_ByteArray* next)
{
	LinkedList_ByteArray* ba_new = malloc(sizeof(LinkedList_ByteArray));
	if(ba_new == NULL){
		printf("Error: creating a new array\n");
		return NULL;
	}
	ba_new->data = malloc(newData_len);

	memcpy(ba_new->data, newData, newData_len);
	ba_new->length = newData_len;
	ba_new->next = next;

	return ba_new;
}

////////////////////////////////////////////////////////////////////////////////
/* @brief for debug purpose, print all the elements of the list
 * @param ba the list
 */
void ba_print(LinkedList_ByteArray* ba)
{
	LinkedList_ByteArray* cursor = ba;
	int position = 0;

	printf("Number of arrays in the list: %d.\n", ba_size(ba));

	while(cursor != NULL)
	{
		printf("no: %d, len: %d, data: ", position++, cursor->length);
		pHex_nl(cursor->data, cursor->length);
		cursor = cursor->next;
	}
}

////////////////////////////////////////////////////////////////////////////////
/* @brief return the number of elements in the list
 */
int ba_size(LinkedList_ByteArray * ba)
{
	int size = 0;
	LinkedList_ByteArray* cursor = ba;

	while(cursor != NULL)
	{
		size++;
		cursor = cursor->next;
	}
	return size;
}
////////////////////////////////////////////////////////////////////////////////
/* @brief remove the first element of the list
 * @return the new first element of the list
 * @ATTENTION this function changes the starting element of the list,
 * 			so it must be called as following example bellow:
 * 			ba = ba_removeFirst(ba);
 */
LinkedList_ByteArray* ba_removeFirst(LinkedList_ByteArray* ba)
{
    if(ba == NULL)
        return NULL;

    LinkedList_ByteArray *firstElement = ba;
    ba = ba->next;
    firstElement->next = NULL;

    /* is this the last node in the list */
    if(firstElement == ba)
        ba = NULL;

    free(firstElement->data);	// free the dynamically alocated array in the element
    free(firstElement);			// free the the element

    return ba;
}

////////////////////////////////////////////////////////////////////////////////
/* @brief remove the last element of the list
 * @return pointer to the first element of the list
 */
LinkedList_ByteArray* ba_removeLast(LinkedList_ByteArray* ba)
{
	if(ba == NULL)
		return NULL;

	LinkedList_ByteArray* cursor = ba;
	LinkedList_ByteArray* penultimate = NULL;

	/* go to the last element of the list */
	while(cursor->next != NULL){
		penultimate = cursor;
		cursor = cursor->next;
	}
	/* The "penultimate" will be now the last element of the list */
	if(penultimate != NULL){
		penultimate->next = NULL;
	}

	free(cursor->data);		// free the dynamically alocated array in the element
	free(cursor);			// free the the element

	return ba;
}
////////////////////////////////////////////////////////////////////////////////
/* @brief remove the whole list
 * @return NULL pointer
 */
LinkedList_ByteArray* ba_removeAll(LinkedList_ByteArray* ba)
{
	while(ba != NULL){
		ba = ba_removeFirst(ba);
	}
    return ba;
}


////////////////////////////////////////////////////////////////////////////////
/* @brief find bytearray in the list
 * @param ba is the byte array to be searched in
 * @param data is the searched array
 * @param len is the length of searched array
 * @return -1 if not found, otherwise return the position in the array
 */
int ba_find(LinkedList_ByteArray* ba, uint8_t* data, uint8_t len)
{
	if(ba != NULL)
	{
		LinkedList_ByteArray* cursor = ba;
		int position = 0;

		/* for each element in the list */
		while(cursor->next != NULL)
		{
			/* first check if the length equals */
			if(cursor->length == len)
			{
				/* now check if the data are the same */
				for(int i = 0; i < len; i++)
				{
					if(data[i] != cursor->data[i]){
						break;
					} else if (i == len - 1){
						/* we found it! */
						return position;
					}
				}
			}
			/* go to next */
			position++;
			cursor = cursor->next;
		}
	}
	return -1;
}
