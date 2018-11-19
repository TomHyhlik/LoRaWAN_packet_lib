/*
 * ByteArray.h
 *
 *  Created on: Jul 23, 2018
 *      Author: hyhlik
 */

#ifndef BYTEARRAY_H_
#define BYTEARRAY_H_

#include <stdbool.h>


/////////////////////////////////////////////////
/* @brief used in arrayCpy() function
 */
enum {
  BIG, LITTLE
} endian;


void pHex(uint8_t* str, int len);
void pHex_nl(uint8_t* str, int len);
void pAscii_nl(uint8_t* str, int len);
void pAscii(uint8_t* str, int len);
void pHex_tit(char* title, uint8_t* data, int data_len);

void arrayCpy(uint8_t* target, uint8_t* array, int start, int stop, int endn);

int convert_ASCIIbyteHexString_to_byteArray(uint8_t* out, char* in);

bool arraysEqual(uint8_t* array1, uint8_t* array2, int length);
bool arraysEqualL(uint8_t* array1, uint8_t* array2, int len1, int len2);
bool arrayIsZeros(uint8_t* array, int len);

uint8_t getXor(uint8_t* array, uint8_t len);





////////////// byte array list
//#define BA_MAX_LENGTH  20
//#define BA_LIST_SIZE   20
//typedef struct {
//    uint8_t data[BA_MAX_LENGTH];
//    uint8_t length;
//} ByteArray;
//
//
//bool ba_append(ByteArray* ba, uint8_t* data, uint8_t length);
//int ba_findPlace(ByteArray* ba);
//void ba_popFront(ByteArray *ba);





#endif /* BYTEARRAY_H_ */
