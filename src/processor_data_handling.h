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

/**

 */
struct ARM_Processor{
  uint32_t *memory;
  uint32_t *registers;
  uint32_t cpsr;
  uint32_t pc;
  uint32_t lr;
  uint32_t sp;
};


void initialiseProcessor (struct ARM_Processor*);
void outputInstructions (struct ARM_Processor*);
uint32_t readMemory (struct ARM_Processor*, int);
uint32_t readMemoryLittleEndian (struct ARM_Processor*, int);


#endif //ARM11_35_PROCESSOR_CONSTANTS_H
