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

const int WORD_SIZE = 32;
const int REGISTERS = 17;
const int GENERAL_REGISTERS = 12;
const int MEMORY_LOCATIONS = 65536;

struct ARM_Processor{
  int memory[MEMORY_LOCATIONS];
  int registers[GENERAL_REGISTERS];
  unsigned int cpsr;
  unsigned int pc;
  unsigned int lr;
  unsigned int sp;
};

void initialiseProcessor (struct ARM_Processor* processor){
  for (int i = 0; i<MEMORY_LOCATIONS; i++){
    processor->memory[i] = 0;
    if (i < GENERAL_REGISTERS)
      processor->registers = 0;
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
    count ++;
  }

  return EXIT_SUCCESS;
}

int instruction(char instruction[]) {
  if (instruction[27] == '1') { // 1--
    branch(instruction);
  } else { // 0--
    if (instruction[26] == '1') { // 01-
      singleDataTransfer(instruction);
    } else { // 00-
      if (instruction[25] == '0') { // 000
        multiply(instruction);
      } else if (instruction[25 == '1']) { // 001
        dataProcessing(instruction);
      }
    }
  }
}

int singleDataTransfer(char instruction[]) {
  // cond
  char cond[4];
  for (int i = 0; i < 4; ++i) {
    cond[i] = instruction[31-i];
  }

  // offset
  char offset[12];
  if (cond[25] == '1') {
    // use shifted register
    // TODO
  } else if (cond[25] == '0') {
    // immediate offset
    for (int i = 0; i < 12; i++){
      offset[i] = instruction[11-i];
    }
  }

}

int branch(char instruction[]) {
  // cond
  char cond[4];
  for (int i = 0; i < 4; ++i) {
    cond[i] = instruction[31-i];
  }

  // offset
  char offset[32];
  offset[0] = '0';
  offset[1] = '0';
  sign = instruction[23];
  for (int i = 2; i < 32; ++i) {
    if (i <= 23 {
      offset[i] = instruction[23-i];
    } else {
      offset[i] = sign;
    }
  }

  if (1) { // TODO: Compare cond with CPSR register
    // TODO: offset PC
  }

  return 0;
}

int binCharArrToInt(int size, char arr[]) {
  int base = 1;
  int res = 0;
  for (int i = size; i >= 0; i--) {
    if (arr[i] == '1') {
      res = res + base;
    }
    base*2;
  }
  return res;
}
