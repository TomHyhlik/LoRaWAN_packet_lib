/*
 * ByteArray.c
 *
 *  Created on: Jul 23, 2018
 *      Author: hyhlik
 *
 *
 *      description:
 *      this source file contains basic functions for byte arrays
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "ByteArray.h"
#include "main.h"



////////////////////////////////////////////////////
/* prints array hex numbers without \n at the end
 */
void pHex(uint8_t* str, int len)
{
	if(str == NULL){
		printf("Empty array\n");
		return;
	}
    for (int i = 0; i < len; ++i)
        printf("%.2X", str[i]);
}
////////////////////////////////////////////////////
/* prints array hex numbers with \n at the end
 */
void pHex_nl(uint8_t* str, int len)
{
	pHex(str, len);
    printf("\n");
}
////////////////////////////////////////////////////
void pAscii(uint8_t* str, int len)
{
	if(str == NULL){
		printf("Empty array\n");
		return;
	}
	for(int i = 0; i < len; i++){
		printf("%c", str[i]);
	}
}
////////////////////////////////////////////////////
void pAscii_nl(uint8_t* str, int len)
{
	pAscii(str, len);
	printf("\n");
}
////////////////////////////////////////////////////
void pHex_tit(char* title, uint8_t* data, int data_len)
{
	printf("%s", title);
	pHex_nl(data, data_len);
}

////////////////////////////////////////////////////
/*  @brief coppy part of array to the target location
 *  @param array: da array
 *  @param target: the target memory location
 *  @param start: the starting position of array
 * @param stop: the stop position of array
 * @param endian: oprions: BIG and LITTLE
 */
void arrayCpy(uint8_t* target, uint8_t* array, int start, int stop, int endian)
{
  int i;
  switch(endian)
  {
  case LITTLE:
    for(i = 0 ; start < stop+1 ; i++, start++)
      target[i] = array[start];
  break;
  case BIG:
    for(i = 0 ; start < stop+1 ; i++, stop--)
      target[i] = array[stop];
  break;
  default:
    printf("arrayCpy: invalid input\n");
  }
}


////////////////////////////////////////////////////
/* @brief algorithm is according to the ASCII table
 * @param in points to input string
 * @param out points to output byte array, should be half size of the "in"
 * @return length of the output array, -1 if error 
 */
int convert_ASCIIbyteHexString_to_byteArray(uint8_t* out, char* in)
{
	int out_point = 0;
	int number;
	int out_len = strlen(in)/2;
	memset(out, 0, out_len);

	/* for each character */
	for (int i = 0 ; i < strlen(in); i++)
	{
		/* 1...9 */
		if(in[i] >= 48 && in[i] <= 57){
			number = in[i] - 48;
		}
		/* A...F */
		else if (in[i] >= 65 && in[i] <= 70){
			number = in[i] - 65 + 10;
		}
		/* a...f */
		else if (in[i] >= 97 && in[i] <= 102){
			number = in[i] - 97 + 10;
		}
		else {
			printf("Invalid character: \"%d\", at possition: %d\n", in[i], i);
			return -1;
		}
		/* in case of an odd or even character */
		if(i%2 == 1){
			out[out_point] |= number;
			out_point++;
		} else {
			out[out_point] |= number << 4;
		}
	}
	return out_len;
}

////////////////////////////////////////////////////
bool arraysEqual(uint8_t* array1, uint8_t* array2, int length)
{
//		printf("Compairing: ");
//		pHex(array1, length);
//		printf(" and ");
//		pHex_nl(array2, length);

	for(int i = 0; i < length; i++){
		if(array1[i] != array2[i]){
			return false;
		}
	}

	return true;
}

bool arraysEqualL(uint8_t* array1, uint8_t* array2, int len1, int len2)
{
	pHex_nl(array1, len1);
	pHex_nl(array2, len2);
	if(len1 != len2)
		return false;

	for(int i = 0; i < len1; i++){
		if(array1[i] != array2[i]){
			return false;
		}
	}
	return true;
}
///////////////////////////////////////////////////////////
uint8_t calcCRC(uint8_t* data, uint8_t data_len)
{
	uint8_t calculatedCRC = 0;
	/* debug */
//	printf("--------calculating crc from data: ");
//	pHex_nl(data, data_len);

	for(int i = 0; i < data_len; i++)
		calculatedCRC ^= data[i];

	return calculatedCRC;
}
///////////////////////////////////////////////////////////
bool arrayIsZeros(uint8_t* array, int len)
{
	for(int i = 0; i < len; i++)
	{
		if(array[i] != 0)
		{
			return false;
		}
	}
	return true;
}

