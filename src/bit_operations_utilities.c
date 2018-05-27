//
// Created by kenny on 25/05/18.
//
#include <stdlib.h>
#include <assert.h>
#include "bit_operations_utilities.h"

uint32_t *getBits (uint32_t x){

  uint32_t numBits = sizeof(uint32_t)*8;
  uint32_t *bits = malloc(sizeof(uint32_t)*numBits);
  uint32_t one = ((uint32_t) 1) << (numBits-1);

  for (int i = 0; i<numBits; i++){
    if ((x & one) == one )
      bits[i] = 1;
    else
      bits[i] = 0;

    x = x << 1;
  }

  return bits;
}

uint32_t isolateBits (uint32_t num, int end, int start, int newPos){
  //This function isolates a specific segment of bits in a 32 bit word
  //It then moves the most significant bit of this isolated segment to a specific position

  //start is a less significant position  than end
  assert(end>=start);

  //Gets bits from position start and end inclusive
  //Puts bits in a new position, adding trailing and leading zeros where necessary

  /**
        Method of going about this:
            1.Move the  required isolated bits as far right as possible
            2.Get a bit mask of 1s and move to the right up to the most significant bit of the
            required isolated bits

            3.Use the bitwise & on these two.

            4. Move the result to the left until MSB is position newPos
   */

   int numBits = (end-start)+1;
   unsigned bitMask = ~((unsigned int) (0));
   int endBit =sizeof(uint32_t)*8 - 1;

   //Move required isolated bits to the far right
   num >>= start;

   //Move bitmask so the only 1s left are in line with isolated bits
   bitMask >>= (endBit-end)+start;

   num = num & bitMask;

   //Now we move the bits to the position newPos
   //Currently, the MSB has position numBits-1
   //So we move it by newPos-(numBits-1) to get MSB to newPos
   num <<= newPos-(numBits-1);

   return num;

}

uint32_t reverseEndianness (uint32_t x){

  uint32_t xSplit[4];
  int start = 0;
  int end = 7;
  int newPos = 31;

  for (int i = 0; i<4; i++){
    xSplit[i] = isolateBits(x,end,start,newPos);
    start += 8;
    end += 8;
    newPos -= 8;
  }

  return xSplit[0]|xSplit[1]|xSplit[2]|xSplit[3];

}
