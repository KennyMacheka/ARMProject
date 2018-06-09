//
// Created by kenny on 25/05/18.
//
#ifndef ARM11_35_BIT_OPERATIONS_UTILITIES_H
#define ARM11_35_BIT_OPERATIONS_UTILITIES_H
#include <stdint.h>
uint32_t *getBits (uint32_t);
uint32_t reverseEndianness (uint32_t );
uint32_t isolateBits (uint32_t, int, int, int );
uint32_t rotateRight (uint32_t, int);
uint32_t rotateLeft(uint32_t , int );
void setBit (uint32_t*, uint32_t, uint32_t);
#endif 

