//
// Created by kenny on 25/05/18.
//

#ifndef ARM11_35_PROCESSOR_CONSTANTS_H
#define ARM11_35_PROCESSOR_CONSTANTS_H
#include <stdint.h>

extern const int WORD_SIZE;
extern const int REGISTERS;
extern const int GENERAL_REGISTERS;
extern const int MEMORY_LOCATIONS;
extern const int GPIO_LOCATIONS;
extern const int BLOCK_INTERVAL;
extern const int SP;
extern const int LP;
extern const int PC;
extern const int CPSR;

struct ARM_Processor{
  uint8_t *memory;
  uint32_t *registers;
};

void initialiseProcessor (struct ARM_Processor*);
void outputInstructions (struct ARM_Processor*);
uint32_t readMemory (struct ARM_Processor*, int);
uint32_t readMemoryLittleEndian (struct ARM_Processor*, int);
void writeToMemory (struct ARM_Processor*, uint32_t, int);
#endif
