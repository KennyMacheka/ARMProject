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
const int GENERAL_REGISTERS = 13;
const int MEMORY_LOCATIONS = 65536;

struct ARM_Processor{
  int memory[MEMORY_LOCATIONS];
  int registers[GENERAL_REGISTERS];
  unsigned int cpsr;
  unsigned int pc;
};

int dataProcessing(struct ARM_Processor *processor,
                   int i, int opCode, int s, int rn, int rd, int operand2) {
  // setup operand2

  int carryOut = 0;

  if (i) {
    int imm = operand2 & 0xFF;
    int rotate = (operand2 & 0xF00) >> 7;
    operand2 = imm;
    for (int i = 0; i < rotate; i++) {
      int rightMost = operand2 & 1;
      operand2 >>= 1;
      operand2 += rightMost << 31;
      carryOut = rightMost;
    }
  } else {
    int shift = (operand2 & 0xFF0) >> 4;
    int rm = operand2 & 0xF;

    // shift specified by register
    int amountToShift;
    int shiftType = shift & 6;
    if (shift & 1) {
      int rs = shift >> 4;
      amountToShift = processor->registers[rs] & 0xFF;
    } else {
      amountToShift = shift >> 3;
    }

    if (shiftType == 0) {
      operand2 = processor->registers[rm] << (amountToShift - 1);
      carryOut = operand2 & 0x80000000;
      if (carryOut) {
        carryOut = 1;
      }
      operand2 <<= 1;
    } else if (shiftType == 1) {
      operand2 = processor->registers[rm] >> amountToShift - 1;
      carryOut = operand2 & 1;
      operand2 >>= 1;
    } else if (shiftType == 2) {
      int sign = processor->registers[rm] & 0x80000000;
      int magnitude = processor->registers[rm] & 0x7FFFFFFF >> amountToShift - 1;
      carryOut = magnitude & 1;
      magnitude >>= 1;
      operand2 = magnitude;
      if (sign) {
        int signExtend = 0;
        for (int i = sign; i < (sign / (amountToShift * 2)); i >> 1) {
          signExtend += i;
        }
        operand2 += signExtend;
      }
    } else {
      operand2 = processor->registers[rm];
      for (int i = 0; i < amountToShift; i++) {
        int rightMost = operand2 & 1;
        operand2 >>= 1;
        operand2 += rightMost << 31;
        carryOut = rightMost;
      }
    }
  }

  int val = processor->registers[rn];

  if (opCode == 0 || opCode == 8) {
    val &= operand2;
  } else if (opCode == 1 || opCode == 9) {
    val ^= operand2;
  } else if (opCode == 2 || opCode == 10) {
    val -= operand2;
  }

  if (opCode <= 4 || opCode == 12 || opCode == 13) {
    if (opCode == 3) {
      val = operand2 - val;
    } else if (opCode == 4) {
      val += operand2;
    } else if (opCode == 12) {
      val |= operand2;
    } else if (opCode == 13) {
      val = operand2;
    }
    processor->registers[rd] = val;
  }

  // arithmetic
  if (opCode == 0 || opCode == 2 || opCode == 3 || opCode == 9) {
    if (val > (1 << 31) || val < 0) {
      carryOut = 1;
    } else {
      carryOut = 0;
    }
  }

  if (s) {
    if (carryOut) {
      processor->cpsr = processor->cpsr | C;
    } else {
      processor->cpsr = processor->cpsr & ~C;
    }

    if (val < 0) {
      processor->cpsr = (processor->cpsr | N) & ~Z;
    } else {
      processor->cpsr = processor->cpsr & ~N;
      if (val == 0) {
        processor->cpsr = processor->cpsr | Z;
      } else {
        processor->cpsr = processor->cpsr & ~Z;
      }
    }
  }
  
  return 0;
}

int multiply(struct ARM_Processor *processor, int a, int s, int rd, int rn, int rs, int rm) {
  int val = 0;

  if (a) {
    val = processor->registers[rn];
  }

  val += processor->registers[rm] * processor->registers[rs];

  if (s) {
    if (val < 0) {
      processor->cpsr = (processor->cpsr | N) & ~Z;
    } else {
      processor->cpsr = processor->cpsr & ~N;
      if (val == 0) {
        processor->cpsr = processor->cpsr | Z;
      } else {
        processor->cpsr = processor->cpsr & ~Z;
      } 
    }
  }

  processor->registers[rd] = val;
  return 0;
}

// offset is unsigned
int singleDataTransfer(struct ARM_Processor *processor,
                       int i, int p, int u, int l, int rn, int rd, int offset) {
  if (i) {
  
  }
  
  return 0;
}

// offset can be negative (two's complement)
int branch(struct ARM_Processor *processor, int offset) {
  offset <<= 2;
  int mask = 0x02000000;
  if (mask & offset) {
    offset += 0xFFC00000;
  }

  // the offset will take into account the effect of the pipeline?
  // TODO
  //

  processor->pc = processor->memory[processor->pc + offset];
  return 0;
}

void initialiseProcessor (struct ARM_Processor* processor){

  processor->memory = calloc(MEMORY_LOCATIONS,sizeof(int));
  processor->registers = calloc(GENERAL_REGISTERS, sizeof(int));

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

int *getBits (int x){
  int *bits = malloc(sizeof(int)*32);
  int one = 1 << 31;


  for (int i = 0; i<32; i++){
    if ((x & one) == one )
      bits[i] = 1;
    else
      bits[i] = 0;

    x = x << 1;

  }

  return bits;

}

void outputInstructions (struct ARM_Processor processor){

  for (int i = 0; i<MEMORY_LOCATIONS; i++){
    int *bits = getBits(processor.memory[i]);

    for (int j = 0; j<4; j++){
      for (int k=0; k<8; k++)
        printf("%d", bits[j*8 + k]);

      printf(" ");
    }

    if (processor.memory[i] == 0)
      break;

    printf("\n");
    free(bits);

  }
}

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
    //USES LITTLE ENDIAN
    fread(processor.memory+count,WORD_SIZE/8,1,file);
    count++;
  }
  fclose(file);

  printf("Number of instructions: %d\n", count);
  outputInstructions(processor);

  //At this point I've loaded all of the instructions into memory
  //Now I have to run everything via fetch-decode execute cycle
  //First thing to do is to fetch
  //Then attempt to decode current instruction
  //Then attempt to execute


  return EXIT_SUCCESS;
}
