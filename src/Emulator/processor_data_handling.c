#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "processor_data_handling.h"
#include "bit_operations_utilities.h"



const int WORD_SIZE = 32;
const int REGISTERS = 17;
const int GENERAL_REGISTERS = 13;

//65536 bytes in main memory. Word length is 4 bytes, so divide that by 4
const int MEMORY_LOCATIONS = 16384;

void initialiseProcessor (struct ARM_Processor* processor){

  processor->memory = calloc(MEMORY_LOCATIONS,sizeof(uint32_t));
  processor->registers = calloc(GENERAL_REGISTERS, sizeof(uint32_t));

  for (int i = 0; i<MEMORY_LOCATIONS; i++){
    processor->memory[i] = 0;
    if (i < GENERAL_REGISTERS)
      processor->registers[i] = 0;
  }

  processor->cpsr = 0;
  processor->pc = 0;
  processor->lr = 0;
  processor->sp = 0;
}

void outputInstructions (struct ARM_Processor* processor){

  for (int i = 0; i<MEMORY_LOCATIONS; i++){
    uint32_t *bits = getBits(processor->memory[i]);

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
  //Returns contents of memory location in big endian form
  return reverseEndianness(processor->memory[location]);
}

uint32_t readMemoryLittleEndian (struct ARM_Processor *processor, int location){
  return processor -> memory[location];
}