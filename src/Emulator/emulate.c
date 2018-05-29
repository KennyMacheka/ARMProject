#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "processor_data_handling.h"
#include "bit_operations_utilities.h"
#include "fetch_decode_execute.h"

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

#define N 0x80000000
#define Z 0x40000000
#define C 0X20000000
#define V 0x10000000

#define Cond 0xF0000000


int main(int argc, char **argv) {
  /**MEMORY IS IN LITTLE ENDIAN*/
  printf("object code to emulate: %s\n", argv[1]);
  FILE* file = fopen(argv[1],"rb");
  assert (file != NULL);

  int count = 0;

  struct ARM_Processor processor;
  initialiseProcessor(&processor);

  while (count < MEMORY_LOCATIONS){
    if (feof(file))
        break;

    uint32_t instruction;
    fread(&instruction,BLOCK_INTERVAL,1,file);
    /**Reading binary file (which is in little endian) causes its contents to be in big endian
       So we need to reverse this so data is stored correctly in little endian
       This doesn't really matter to provide an accurate simulation
       However, I've kept main memory in little endian form to represent a true
       likeness of the ARM processor*/
    writeToMemory(&processor, instruction, count);
    count+= BLOCK_INTERVAL;
  }
  fclose(file);

  fetchDecodeExecute(&processor);
  /*
  printf("Number of instructions: %d\n", count);
  outputInstructions(&processor);*/


  printf("Registers:\n");
  //Finished running
  for (int i = 0; i<GENERAL_REGISTERS; i++)
    printf("R%d: %d\n", i, processor.registers[i]);

  printf("sp: %d\nlr: %d\npc: %d\ncpsr: %d\n", processor.sp, processor.lr,
          processor.pc, processor.cpsr);

  printf ("Non Zero memory: \n");

  printf("\n");

  for (int i = 0; i<MEMORY_LOCATIONS; i+=BLOCK_INTERVAL){
    uint32_t  data = readMemoryLittleEndian(&processor, i);
    if (data != 0)
      printf("Address %08x: %08x\n", i, data);
  }
  return EXIT_SUCCESS;
}
