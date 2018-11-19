#include <stdint.h>

#ifndef LINKEDLIST_BYTEARRAY_H_
#define LINKEDLIST_BYTEARRAY_H_


typedef struct LinkedList_ByteArray{
    uint8_t* data;
    uint8_t length;
    struct LinkedList_ByteArray* next;
} LinkedList_ByteArray;


void pHex_nl(uint8_t* str, int len);
void pHex(uint8_t* str, int len);

LinkedList_ByteArray* ba_CreateNewArray(uint8_t* newData, uint8_t newData_len, LinkedList_ByteArray* next);
LinkedList_ByteArray* ba_append(LinkedList_ByteArray* ba, uint8_t* newData, uint8_t size);
LinkedList_ByteArray* ba_append_str(LinkedList_ByteArray* ba, char* newData);

LinkedList_ByteArray* ba_prepend(LinkedList_ByteArray* ba, uint8_t* newData, uint8_t size);
LinkedList_ByteArray* ba_removeLast(LinkedList_ByteArray* ba);
LinkedList_ByteArray* ba_removeFirst(LinkedList_ByteArray* ba);
LinkedList_ByteArray* ba_removeAll(LinkedList_ByteArray* ba);
int ba_find(LinkedList_ByteArray* ba, uint8_t* data, uint8_t len);
LinkedList_ByteArray* ba_appendIfNotIncluded(LinkedList_ByteArray* ba, uint8_t* newData, uint8_t size);


void ba_print(LinkedList_ByteArray* ba);
int ba_size(LinkedList_ByteArray * ba);

#endif /* LINKEDLIST_BYTEARRAY_H_ */
