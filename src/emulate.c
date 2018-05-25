#include <stdlib.h>
#include <stdio.h>

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

const int WORD_SIZE = 32;
const int REGISTERS = 17;
const int GENERAL_REGISTERS = 13;
const int MEMORY_LOCATIONS = 65536;

struct ARM_Processor{
  int memory[MEMORY_LOCATIONS];
  int registers[GENERAL_REGISTERS];
  unsigned int cpsr;
  unsigned int pc;
  unsigned int lr;
  unsigned int sp;
};

int dataProcessing(struct ARM_Processor *processor,
                   int I, int opCode, int s, int rn, int rd, int operand2) {
  return 0;
}

int multiply(struct ARM_Processor *processor, int a, int s, int rd, int rn, int rs, int rm) {
  int res = 0;

  if (a) {
    res = processor->registers[rn];
  }

  res += processor->registers[rm] * processor->registers[rs];

  if (s) {
    if (res < 0) {
      processor->cpsr = (processor->cpsr | N) & ~Z;
    } else {
      processor->cpsr = processor->cpsr & ~N;
      if (res == 0) {
        processor->cpsr = processor->cpsr | Z;
      } else {
        processor->cpsr = processor->cpsr & ~Z;
      } 
    }
  }

  processor->registers[rd] = res;
  return 0;
}

int singleDataTransfer(struct ARM_Processor *processor,
                       int i, int p, int u, int l, int rn, int rd, int offset) {
  return 0;
}

int branch(struct ARM_Processor *processor, int offset) {
  // TODO WITH KENNY
  return 0;
}

void initialiseProcessor (struct ARM_Processor* processor){
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

int main(int argc, char **argv) {
  /**MEMORY IS IN LITTLE ENDIAN*/
  printf("object code to emulate: %s\n", argv[1]);
  int readFile[MEMORY_LOCATIONS][WORD_SIZE];
  FILE* file = fopen(argv[1],"rb");
  int count = 0;

  struct ARM_Processor processor;

  while (count < MEMORY_LOCATIONS){
    if (feof)
        break;
    //USES LITTLE ENDIAN
    int word;
    fread(processor.memory+count,WORD_SIZE/8,1,file);
    count++;
  }

  return EXIT_SUCCESS;
}

