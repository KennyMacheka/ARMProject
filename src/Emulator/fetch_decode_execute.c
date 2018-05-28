//
// Created by kenny on 26/05/18.
//
#include <stdint.h>
#include <stdbool.h>
#include "fetch_decode_execute.h"
#include "processor_data_handling.h"
#include "bit_operations_utilities.h"
#include "result_flags.h"

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

#define Cond 0xF0000000
//The four positions in CPSR that can be 1 or 0
#define N 31
#define Z 30
#define C 29
#define V 28

#define HALT 0


//Numbers correspond to binary numbers in spec
//Used to check if an instruction should be executed
#define eq 0x0
#define ne 0x1
#define ge 0xA
#define lt 0xB
#define gt 0xC
#define le 0xD
#define al 0xE

//Opcodes for data processing
#define AND 0x0
#define EOR 0x1
#define SUB 0x2
#define RSB 0x3
#define ADD 0x4
#define TST 0x8
#define TEQ 0x9
#define CMP 0xA
#define ORR 0xC
#define MOV 0xD

//Opcodes for shifting
#define LSL 0x0
#define LSR 0x1
#define ASR 0x2
#define ROR 0x3

//Custom opcodes for the four types of instructions
#define DATA_PROCESSING 0
#define MULTIPLY 1
#define SINGLE_DATA_TRANSFER 2
#define BRANCH 3

//I'll need to create error flags
uint64_t errors = 0;

int dataProcessing(struct ARM_Processor *processor, uint32_t i, uint32_t opCode, uint32_t s,
                   uint32_t rn, uint32_t rd, uint32_t operand2);

int multiply(struct ARM_Processor *processor, uint32_t a, uint32_t s, uint32_t rd, uint32_t rn,
             uint32_t rs, uint32_t rm);

int singleDataTransfer(struct ARM_Processor *processor, uint32_t i, uint32_t p, uint32_t u,
                       uint32_t l, uint32_t rn, uint32_t rd, uint32_t offset);

int branch(struct ARM_Processor *processor, uint32_t offset);


struct execute{
  bool terminate;
  bool ready;
  unsigned int condition;

  int operation;

  union execArgs{
    struct data{
      uint32_t i;
      uint32_t opCode;
      uint32_t s;
      uint32_t rn;
      uint32_t rd;
      uint32_t operand2;
    }dataArgs;

    struct mult{
      uint32_t a;
      uint32_t s;
      uint32_t rd;
      uint32_t rn;
      uint32_t rs;
      uint32_t rm;
    }multArgs;

    struct singleData{
      uint32_t i;
      uint32_t p;
      uint32_t u;
      uint32_t l;
      uint32_t rn;
      uint32_t rd;
      uint32_t offset;
    }singleDataArgs;

    struct brn{
      uint32_t offset;
    }branchArgs;

  }args;
};

struct decode{
  bool ready;
  uint32_t  instruction;
};

struct ARMPipeline{
  struct execute decoded;
  struct decode fetched;
}pipeline;


void fetchDecodeExecute(struct ARM_Processor* processor) {
  //Pipe line does following: executes a decoded instruction
  //decodes a fetched instruction
  //fetches an instruction
  //Instructions are interpreted in big endian format
  pipeline.decoded.terminate = false;
  pipeline.decoded.ready = false;
  pipeline.fetched.ready = false;


  while (1){
    //Get status bits in cpsr register
    uint32_t n = isolateBits(processor->cpsr, N, N, 0);
    uint32_t z = isolateBits(processor->cpsr, Z, Z, 0);
    uint32_t c = isolateBits(processor->cpsr, C, C, 0);
    uint32_t v = isolateBits(processor->cpsr, V, V, 0);

    if (pipeline.decoded.ready){
      //Execute decoded instruction

      if (pipeline.decoded.terminate)
        break;

      bool shouldExecute = false;
      switch (pipeline.decoded.condition){
        case eq:
          if (z == 1)
            shouldExecute = true;
          break;

        case ne:
          if (z == 1)
            shouldExecute = true;
          break;

        case ge:
          if (n == v)
            shouldExecute = true;
          break;

        case lt:
          if (n != v)
            shouldExecute = true;
          break;

        case gt:
          if (z == 0 && n == v)
            shouldExecute = true;
          break;

        case le:
          if (z == 1 || n != v)
            shouldExecute = true;
          break;

        case al:
          shouldExecute = true;
          break;

        //HANDLE DEFAULT CASE
      }


      if (shouldExecute) {
        //No default case
        switch (pipeline.decoded.operation){
          case DATA_PROCESSING:

            dataProcessing(processor, pipeline.decoded.args.dataArgs.i,
                           pipeline.decoded.args.dataArgs.opCode,
                           pipeline.decoded.args.dataArgs.s,
                           pipeline.decoded.args.dataArgs.rn,
                           pipeline.decoded.args.dataArgs.rd,
                           pipeline.decoded.args.dataArgs.operand2);
            break;

          case MULTIPLY:
            multiply(processor, pipeline.decoded.args.multArgs.a,
                     pipeline.decoded.args.multArgs.s,
                     pipeline.decoded.args.multArgs.rd,
                     pipeline.decoded.args.multArgs.rn,
                     pipeline.decoded.args.multArgs.rs,
                     pipeline.decoded.args.multArgs.rm);
            break;

          case SINGLE_DATA_TRANSFER:
            singleDataTransfer(processor, pipeline.decoded.args.singleDataArgs.i,
                               pipeline.decoded.args.singleDataArgs.p,
                               pipeline.decoded.args.singleDataArgs.u,
                               pipeline.decoded.args.singleDataArgs.l,
                               pipeline.decoded.args.singleDataArgs.rn,
                               pipeline.decoded.args.singleDataArgs.rd,
                               pipeline.decoded.args.singleDataArgs.offset);
            break;

          case BRANCH:
            branch(processor, pipeline.decoded.args.branchArgs.offset);
            break;
        }
      }
      pipeline.decoded.ready = false;
    }

    if (pipeline.fetched.ready){
      //If instruction is 0 this means halt, no need to check anything else
      if (pipeline.fetched.instruction == HALT){
        pipeline.decoded.terminate = true;
        pipeline.decoded.ready = true;
        pipeline.fetched.ready = false;
      }

      else {

        pipeline.decoded.condition = isolateBits(pipeline.fetched.instruction, 31, 28, 3);
        uint32_t bit27 = isolateBits(pipeline.fetched.instruction, 27, 27, 0);

        //Branch
        if (bit27 == 1) {
          pipeline.decoded.operation = BRANCH;

          //Offset
          pipeline.decoded.args.branchArgs.offset = isolateBits(pipeline.fetched.instruction, 23, 0, 23);
          pipeline.decoded.ready = true;
          pipeline.fetched.ready = false;
        }

        else{
          int bit26 = isolateBits(pipeline.fetched.instruction, 26, 26, 0);

          //Single data transfer
          if (bit26 == 1){
            //I
            pipeline.decoded.operation = SINGLE_DATA_TRANSFER;
            pipeline.decoded.args.singleDataArgs.i = isolateBits(pipeline.fetched.instruction, 25, 25 , 0);
            //P
            pipeline.decoded.args.singleDataArgs.p = isolateBits(pipeline.fetched.instruction, 24, 24 , 0);
            //U
            pipeline.decoded.args.singleDataArgs.u = isolateBits(pipeline.fetched.instruction, 23, 23 , 0);
            //L
            pipeline.decoded.args.singleDataArgs.l = isolateBits(pipeline.fetched.instruction, 20, 20 , 0);
            //Rn
            pipeline.decoded.args.singleDataArgs.rn = isolateBits(pipeline.fetched.instruction, 19, 16 , 3);
            //Rd
            pipeline.decoded.args.singleDataArgs.rd = isolateBits(pipeline.fetched.instruction, 15, 12 , 3);
            //Offset
            pipeline.decoded.args.singleDataArgs.offset = isolateBits(pipeline.fetched.instruction, 11, 0 , 11);

            pipeline.decoded.ready = true;
            pipeline.fetched.ready = false;
          }

          //data processing or multiply
          else{
            bool isDataProcessing = false;
            bool isMultiply = false;

            uint32_t  bit20 = isolateBits(pipeline.fetched.instruction,20,20,0);
            uint32_t  r1 = isolateBits(pipeline.fetched.instruction,19,16,3);
            uint32_t  r2 = isolateBits(pipeline.fetched.instruction,15,12,3);

            //Check bit 25 is 1
            if (isolateBits(pipeline.fetched.instruction,25,25,0) == 1){
              isDataProcessing = true;
              isMultiply = false;
            }

            else{
              //Assume instruction is data processing
              //Take bits 24-22 inclusive. If != 0, then instruction is not multiply
              if (isolateBits(pipeline.fetched.instruction,24,22,2) != 0){
                isDataProcessing = true;
                isMultiply = false;
              }

              else{
                int multiplySpecialValue = 9;
                //Check bits 7-4
                //If value is not 9, then we have data processing
                uint32_t fourBits = isolateBits(pipeline.fetched.instruction, 7, 4, 3);

                if (fourBits != multiplySpecialValue){
                  isDataProcessing = true;
                  isMultiply = false;
                }

                //Must be multiplying (this is not possible for data procesing)
                //I say invalid for data processing according to p. 7 of spec, where bit 7 is 0 not 1
                else{
                  isDataProcessing = false;
                  isMultiply = true;

                }
              }

              if (isDataProcessing){
                pipeline.decoded.operation = DATA_PROCESSING;

                //I
                pipeline.decoded.args.dataArgs.i = isolateBits(pipeline.fetched.instruction,25,25,0);
                //Opcode
                pipeline.decoded.args.dataArgs.opCode = isolateBits(pipeline.fetched.instruction,24,21,3);
                //S
                pipeline.decoded.args.dataArgs.s = bit20;
                //rn
                pipeline.decoded.args.dataArgs.rn= r1;
                //rd
                pipeline.decoded.args.dataArgs.rd= r2;
                //opcode
                pipeline.decoded.args.dataArgs.opCode = isolateBits(pipeline.fetched.instruction,11,0,11);
                pipeline.decoded.ready = true;
                pipeline.fetched.ready = false;
              }

              //multiply
              else if (isMultiply){
                pipeline.decoded.operation = MULTIPLY;

                //A
                pipeline.decoded.args.multArgs.a = isolateBits(pipeline.fetched.instruction,21,21,0);
                //S
                pipeline.decoded.args.multArgs.s = bit20;

                //Rd
                pipeline.decoded.args.multArgs.rd = r1;

                //Rn
                pipeline.decoded.args.multArgs.rn = r2;

                //Rs
                pipeline.decoded.args.multArgs.rs = isolateBits(pipeline.fetched.instruction,11,8,3);

                //Rm
                pipeline.decoded.args.multArgs.rm = isolateBits(pipeline.fetched.instruction,3,0,3);

                pipeline.decoded.ready = true;
                pipeline.fetched.ready = false;

              }
            }
          }
        }
      }
    }

    pipeline.fetched.instruction = readMemory(processor,processor->pc);
    pipeline.fetched.ready = true;
    processor->pc++;
  }

}


int dataProcessing (struct ARM_Processor *processor, uint32_t i, uint32_t opCode, uint32_t s,
                    uint32_t rn, uint32_t rd, uint32_t operand2){

  //Set C to the value of carry out in any shift operation
  uint32_t carryOut = 0;
  uint32_t  rnContents = processor->registers[rn];

  //Immediate value
  if (i == 1){
    uint32_t rotateAmount = isolateBits(operand2,11,8,3)*2;
    uint32_t imm = isolateBits(operand2,7,0,7);
    operand2 = imm;
    rotateRight(operand2, rotateAmount);
    carryOut = isolateBits(imm, rotateAmount-1, rotateAmount-1, 0);
  }

  else{

    uint32_t shift = isolateBits(operand2,11,4,7);
    uint32_t rm = isolateBits(operand2,3,0,3);
    uint32_t  shiftType = isolateBits(operand2,6,5,1);
    int amountToShift;

    if (isolateBits(operand2,4,4,0) == 0){
      amountToShift = isolateBits(operand2,11,7,4);
    }

    else{
      uint32_t rs = isolateBits(operand2,11,8,3);
      amountToShift = isolateBits(processor->registers[s],7,0,7);
    }

    //No need to shift anything
    if (amountToShift == 0){
      operand2 = processor->registers[rm];
    }

    //Logical left
    else if (shiftType == LSL){
      operand2 = processor->registers[rm] << amountToShift;
      //We want to get LSB of bits that are discarded in left shift
      //This will be the carry out
      carryOut = isolateBits(processor->registers[rm], 32-amountToShift, 32-amountToShift,0);
    }

    //Logical right
    else if (shiftType == LSR){
      operand2 = processor->registers[rm] >> amountToShift;
      carryOut = isolateBits(processor->registers[rm], amountToShift-1, amountToShift-1,0);
    }

    //Arithmetic right
    else if (shiftType == ASR){
      //We'll either have 10000000000000000..00 or 000.00000
      uint32_t sign = isolateBits(processor->registers[rm], 31, 31, 31);
      carryOut = isolateBits(processor->registers[rm], amountToShift-1, amountToShift-1,0);
      operand2 = processor->registers[rm] >> amountToShift;

      //Preserve the sign of the MSB (i.e. if MSB is 1, any new zeros are converted to 1s)
      for (int i = 0; i<amountToShift; i++, sign >>= 1)
        operand2 |= sign;
    }

    //Rotate right
    else if (shiftType == ROR){
      operand2 = rotateRight(processor->registers[rm], amountToShift);
      carryOut = isolateBits(processor->registers[rm], amountToShift-1, amountToShift-1, 0);
    }
  }

  uint32_t result;

  if (opCode == AND || opCode == TST) {
    result = rnContents & operand2;
  }

  else if (opCode == EOR || opCode == TEQ){
    result = rnContents ^ operand2;
  }

  else if (opCode == SUB || opCode == CMP){
    result = rnContents - operand2;

    //Produced a borrow
    if (result > rnContents)
      carryOut = 0;

    else
      carryOut = 1;
  }

  else if (opCode == ADD){
    result = rnContents + operand2;

    //Overflow
    if (result < rnContents)
      carryOut = 1;
    else
      carryOut = 0;
  }

  else if (opCode == RSB){
    result = operand2 - rn;

    if (result > operand2)
      carryOut = 0;

    else
      carryOut = 1;
  }

  else if (opCode == ORR)
    result = rn | operand2;

  else if (opCode == MOV)
    result = operand2;

  //Add a flag to indicate invalid opcode
  else
    return FAILURE;


  if (!(opCode == TST || opCode == TEQ || opCode == CMP))
    processor->registers[rd] = result;

  if (s == 1){
      setBit(&processor->cpsr, C, carryOut);

      if (result == 0)
        setBit(&processor->cpsr, Z, 1);
      else
        setBit(&processor->cpsr, Z, 0);

      setBit(&processor->cpsr, N, isolateBits(result,31,31,31));
  }

  return SUCCESS;
}

//Not finished
int multiply(struct ARM_Processor *processor, uint32_t a, uint32_t s, uint32_t rd, uint32_t rn,
             uint32_t rs, uint32_t rm) {
  uint32_t val = 0;

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

// Not finished
int singleDataTransfer(struct ARM_Processor *processor, uint32_t i, uint32_t p, uint32_t u,
                       uint32_t l, uint32_t rn, uint32_t rd, uint32_t offset){
  if (i) {

  }

  return 0;
}

// Not finished
int branch(struct ARM_Processor *processor, uint32_t offset) {
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
