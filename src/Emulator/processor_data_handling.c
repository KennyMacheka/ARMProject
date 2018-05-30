#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "processor_data_handling.h"
#include "bit_operations_utilities.h"
/**Might remove R0-R12 if they are not needed*/
#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12

const int WORD_SIZE = 32;
const int REGISTERS = 17;
const int BLOCK_INTERVAL = 4;
const int GENERAL_REGISTERS = 13;

//65536 bytes in main memory. Word length is 4 bytes, so divide that by 4
const int MEMORY_LOCATIONS = 65536;
const int SP = 13;
const int LP = 14;
const int PC = 15;
const int CPSR = 16;


void initialiseProcessor (struct ARM_Processor* processor){

  processor->memory = calloc(MEMORY_LOCATIONS,sizeof(uint8_t));
  processor->registers = calloc(REGISTERS, sizeof(uint32_t));

  for (int i = 0; i<MEMORY_LOCATIONS; i++){
    processor->memory[i] = 0;
    if (i < REGISTERS)
      processor->registers[i] = 0;
  }
}

void outputInstructions (struct ARM_Processor* processor){

  for (int i = 0; i<MEMORY_LOCATIONS; i+= 4){
    uint32_t *bits = getBits(readMemoryLittleEndian(processor,i));

    for (int j = 0; j<4; j++){
      for (int k=0; k<8; k++)
        printf("%d", bits[j*8 + k]);

      printf(" ");
    }

    if (processor->memory[i] == 0)
      break;

    printf("\n");
    free(bits);

  }
}

uint32_t readMemory (struct ARM_Processor *processor, int location){
  //b for byte
  /*
      Reads 4 8bits of data, starting from location, to location+3
      Then returns result in Big Endian form.
   */
  uint32_t result = 0;
  uint8_t shiftAmount = 0;
  for (int i=0; i<4; i++){
    result += (processor->memory[location+i] << shiftAmount);
    shiftAmount += 8;
  }

  return result;

}

uint32_t readMemoryLittleEndian (struct ARM_Processor *processor, int location){

  uint32_t result = 0;
  uint8_t shiftAmount = 24;
  for (int i=0; i<4; i++){
    result += (processor->memory[location+i] << shiftAmount);
    shiftAmount -= 8;
  }

  return result;
}

void writeToMemory (struct ARM_Processor *processor, uint32_t data, int location){
  //pre : data in big endian
  //LSB is stored first (by the very definition of little endian)

  int start = 0;
  int end = 7;

  for (int i = 0; i<4; i++){
    processor->memory[location+i] = (uint8_t) isolateBits(data,end,start,7);
    start += 8;
    end += 8;
  }

}