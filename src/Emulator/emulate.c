#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "processor_data_handling.h"
#include "../bit_operations_utilities.h"
#include "fetch_decode_execute.h"

int main(int argc, char **argv) {
  /*MEMORY IS IN LITTLE ENDIAN*/
  FILE* file = fopen(argv[1],"rb");
  assert (file != NULL);

  int count = 0;

  struct ARM_Processor processor;
  initialiseProcessor(&processor);

  while (count < MEMORY_LOCATIONS){
    if (feof(file))
        break;

    int blocksToRead = 1;
    uint32_t instruction;
    size_t bytesRead = fread(&instruction,BLOCK_INTERVAL,blocksToRead,file);
    if (bytesRead != blocksToRead)
      break;

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

  //Finished running, printing results
  printf("Registers:\n");
  for (int i = 0; i<GENERAL_REGISTERS; i++)
    printf("$%-3d: %10d (0x%08x)\n", i, processor.registers[i], processor.registers[i]);
  printf("PC  : %10d (0x%08x)\n", processor.registers[PC], processor.registers[PC]);
  printf("CPSR: %10d (0x%08x)\n", processor.registers[CPSR], processor.registers[CPSR]);
  
  printf("Non-zero memory:\n");
  for (int i = 0; i<MEMORY_LOCATIONS; i+=BLOCK_INTERVAL) {
    uint32_t  data = readMemoryLittleEndian(&processor, i);
    if (data != 0)
      printf("0x%08x: 0x%08x\n", i, data);
  }

  free(processor.registers);
  free(processor.memory);
  return EXIT_SUCCESS;
}
