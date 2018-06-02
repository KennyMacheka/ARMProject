#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "processor_data_handling.h"
#include "../Utilities/bit_operations_utilities.h"
#include "result_flags.h"

#define GPIO_LOCATION_1 0x20200000
#define GPIO_LOCATION_2 0x20200004
#define GPIO_LOCATION_3 0x20200008
#define GPIO_LOCATION_1_2 0x00000000
#define GPIO_LOCATION_2_2 0x00000004
#define GPIO_LOCATION_3_2 0x00000008

const int WORD_SIZE = 32;
const int REGISTERS = 17;
const int BLOCK_INTERVAL = 4;
const int GENERAL_REGISTERS = 13;

//65536 bytes in main memory
const int MEMORY_LOCATIONS = 65536;
const int GPIO_LOCATIONS = 12;
const int SP = 13;
const int LP = 14;
const int PC = 15;
const int CPSR = 16;


void initialiseProcessor(struct ARM_Processor* processor) {

  processor->memory = calloc(MEMORY_LOCATIONS+GPIO_LOCATIONS,sizeof(uint8_t));
  processor->registers = calloc(REGISTERS, sizeof(uint32_t));

  for (int i = 0; i<MEMORY_LOCATIONS; i++) {
    processor->memory[i] = 0;
    if (i < REGISTERS)
      processor->registers[i] = 0;
  }
}

void outputInstructions(struct ARM_Processor* processor) {

  for (int i = 0; i<MEMORY_LOCATIONS; i+= 4) {
    uint32_t *bits = getBits(readMemoryLittleEndian(processor,i));

    for (int j = 0; j<4; j++) {
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

int transformGPIOLoc(int location) {
  return location - GPIO_LOCATION_1 + MEMORY_LOCATIONS - 12;
}

int gpioInMemory (int location){
  //pre- location is one of the 3 gpio addresses
  switch(location){
    case GPIO_LOCATION_1:
      return MEMORY_LOCATIONS + GPIO_LOCATION_1_2;

    case GPIO_LOCATION_2:
      return MEMORY_LOCATIONS + GPIO_LOCATION_2_2;

    case GPIO_LOCATION_3:
      return MEMORY_LOCATIONS + GPIO_LOCATION_3_2;

    default:
      return 0;
  }
}
uint32_t readMemory(struct ARM_Processor *processor, int location) {
  //b for byte
  /*
      Reads 3 8bits of data, starting from location, to location+3
      Then returns result in Big Endian form.
   */

  if (location >= GPIO_LOCATION_1 && location <= GPIO_LOCATION_2)
    return (uint32_t) location;

  else if (location >= MEMORY_LOCATIONS){
    printf("Error: Out of bounds memory access at address 0x%08x\n", location);
    return FAILURE;
  }


  uint32_t result = 0;
  uint8_t shiftAmount = 0;
  for (int i=0; i<4; i++) {
    result += (processor->memory[location+i] << shiftAmount);
    shiftAmount += 8;
  }
  return result;

}

uint32_t readMemoryLittleEndian(struct ARM_Processor *processor, int location) {

  if (location >= GPIO_LOCATION_1 && location <= GPIO_LOCATION_2)
    location = gpioInMemory(location);

  else if (location >= MEMORY_LOCATIONS){
    printf("Error: Out of bounds memory access at address 0x%08x\n", location);
    return FAILURE;
  }

  uint32_t result = 0;
  uint8_t shiftAmount = 24;
  for (int i=0; i<4; i++) {
    result += (processor->memory[location+i] << shiftAmount);
    shiftAmount -= 8;
  }

  return result;
}

void writeToMemory(struct ARM_Processor *processor, uint32_t data, int location) {
  //pre : data in big endian
  //LSB is stored first (by the very definition of little endian)

  if (location >= GPIO_LOCATION_1 && location <= GPIO_LOCATION_2)
    location = gpioInMemory(location);

  else if (location >= MEMORY_LOCATIONS){
    printf("Error: Out of bounds memory access at address 0x%08x\n", location);
    return;
  }

  int start = 0;
  int end = 7;

  for (int i = 0; i<4; i++) {
    processor->memory[location+i] = (uint8_t) isolateBits(data,end,start,7);
    start += 8;
    end += 8;
  }
}
