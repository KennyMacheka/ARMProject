//
// Created by kenny on 05/06/18.
//
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "convert_instructions.h"
#include "symbol_table_tokens.h"
#include "../Utilities/bit_operations_utilities.h"
#include "../Emulator/processor_data_handling.h"
#include "binary_conversion.h"

uint32_t stringToNum (char *);
uint32_t convertBranch (struct tokenedInstruction *, struct symbolTable* , int);
uint32_t convertSingleDataTransfer (struct tokenedInstruction *, struct symbolTable*,
                                    int , size_t, bool);
uint32_t registerValue (char* );
int storeConstant (uint32_t*, uint32_t);
void storeShiftRegister (uint32_t* , char *, char *, char *);

static int storedConstants = 0;
static uint32_t *constants = NULL;

size_t convert(struct assemblyCode *input, FILE *fout){

  struct symbolTable *table = setupTable();
  struct tokenedCode* tokens = setupTokens(input);
  uint32_t *machineCode = malloc(sizeof(uint32_t)*input->numLines);
  size_t machineLines = 0;


  for (int i = 0; i<input->numLines; i++){
    if (strchr(input->code[i],':')){
      insert(table, i, input->code[i]);
    }
  }


  for (int i = 0; i<input->numLines; i++){
    char *op = tokens->code->line[0];
    if(strchr(op,':'))
      continue;

    else if (op[0] == 'b')
      machineCode[machineLines++] = convertBranch(tokens->code, table, machineLines);

    else if (strcmp(op, "ldr") == 0)
      machineCode[machineLines++] = convertSingleDataTransfer(tokens->code, table, machineLines, input->numLines, true);

    else if (strcmp(op, "str") == 0)
      machineCode[machineLines++] = convertSingleDataTransfer(tokens->code, table, machineLines, input->numLines, false);

  }


  freeSymbolTable(&table);
  freeTokenedCode(&tokens);

  if (machineLines+storedConstants > input->numLines){
    machineCode = realloc(machineCode, input->numLines +(machineLines+storedConstants - input->numLines));
  }

  for (int i = 0; i<storedConstants; i++)
    machineCode[input->numLines+i] = constants[i];

  //Check endianness of this
  return fwrite(machineCode, 4, machineLines, fout);

}

uint32_t convertBranch (struct tokenedInstruction *tokens, struct symbolTable* table, int pos){

  uint32_t pcAhead = 8;
  uint32_t machineCode = 0;
  setBit(&machineCode, 27, 1);
  setBit(&machineCode, 25, 1);

  char *opcode = tokens->line[0];
  char *branchTo = tokens->line[1];

  //Elements in opcodes and bits correspond
  char *opcodes[] = {"beq", "bne", "bge", "blt", "bgt", "ble", "bal", "b"};
  char *bits[] =  {"0000", "0001", "1010", "1011", "1100", "1101", "1110"};

  for (int i = 0; i<7; i++){
    if (strcmp(opcode, opcodes[i]) == 0){
      for (int j = 0; j<4; j++){
        if (bits[i][j] == '1')
          setBit(&machineCode, (uint32_t )31-j, 1);
      }
    }
  }

  uint32_t  offset;
  uint32_t  address;
  if (strchr(branchTo,':')){
    address = (uint32_t) get(table, branchTo);
  }

  else{
    address = stringToNum(branchTo);
  }

  address = ((uint32_t) pos) - address - pcAhead;
  //We now have a signed offset (I used uint32_t as bit shifting is defined by the C standard)
  address >>= 2;

  for (int i = 23; i>= 0; i++){
    setBit(&machineCode,i,isolateBits(address, i, i, 0));
  }

  return machineCode;

}

uint32_t convertSingleDataTransfer (struct tokenedInstruction *tokens, struct symbolTable* table,
                                  int pos, size_t numInstructions, bool isLoading){

  uint32_t  machineCode = 0;
  setBit(&machineCode, 26, 1);
  //set to add by default
  setBit(&machineCode, 23, 1);

  struct tokenedInstruction address;
  uint32_t  offset;
  //Set L bit
  if (isLoading){
    setBit(&machineCode, 20, 1);
  }

  //Set Rd bits (note, register value is representable by 4 bits max)
  uint32_t rd = registerValue(tokens->line[1]);
  rd <<= 12;
  machineCode |= rd;

  //Check operand 2 is an expression
  if (tokens->line[2][0] == '=' && isLoading){
    setBit(&machineCode, 24, 1);
    uint32_t constant = stringToNum(tokens->line[2]+1);

    //Otherwise use mov
    if (constant > 0xFF){
      int dataPos = ((int)numInstructions-table->size)+storedConstants;
      constants = realloc(constants,sizeof(uint32_t)*(storedConstants+1));
      constants[storedConstants++] = constant;

      offset = ((uint32_t )storedConstants-1-pos) + 8;
      uint32_t pc = (uint32_t) PC;
      //PC is rn
      pc <<= 16;
      machineCode |= pc;
      storeConstant(&machineCode, offset);
    }

    //else, treat as mov (move data processing function into this file)
    else{

    }
  }

  //Form ldr/str rd, [...]
  else if (tokens->line[2][0] == '[' && tokens->numTokens == 2){
    setBit(&machineCode, 24, 1);
    //Remove ']'
    tokens->line[2][strlen(tokens->line[2])-1] = '\0';
    tokenInstruction(tokens->line[2]+1, &address);
    uint32_t  rn = registerValue(address.line[0]);
    rn <<= 16;
    machineCode |= rn;
    //Of form [RN]
    if (address.numTokens == 1){
      storeConstant(&machineCode, 0);
    }

    else if (address.line[1][0] == '#')
      storeConstant(&machineCode,stringToNum(address.line[1]+1));

    //Shift register
    else{
      int registerPos = 0;

      if (address.line[1][0] == '-') {
        setBit(&machineCode, 23, 0);
        registerPos = 1;
      }

      //No shifting
      if (address.numTokens == 2){
        storeShiftRegister(&machineCode, address.line[1] + registerPos, NULL, NULL);
      }

      else
        storeShiftRegister(&machineCode, address.line[1] + registerPos, address.line[2], address.line[3]);

    }

  }

  /*
    //Post indexing, of form [RN], <#expr> or [RN], shift Reg
  else{
    tokens->line[0][strlen(tokens->line[0])-1] = '\0';
    uint32_t rn = registerValue(tokens->line[0]+1);
    rn <<= 16;
    machineCode |= rn;


  }*/



  return machineCode;

}

int storeConstant (uint32_t* machineCode, uint32_t val){

  setBit(machineCode, 25, 1);
  //We need to store 8 bit value (val) into 12 bit location
  uint32_t rotateAmount = 0;
  uint32_t seenValue = val;
  while (val > 255 && rotateAmount % 2 != 0){
    val = rotateLeft(val,1);
    rotateAmount ++;
    if (val == seenValue){
      printf("Cannot store value.\n");
      return 1;
    }
  }

  rotateAmount /= 2;
  rotateAmount <<= 8;

  *machineCode |= rotateAmount;
  *machineCode |= seenValue;

  return 0;

}

void storeShiftRegister (uint32_t* machineCode, char *baseReg, char *shiftName, char *regOrExpr){



}

uint32_t registerValue (char* r){
  /**Move constants in Emulator/processor_data_handling.c in separate file.*/
  assert (r != NULL);

  if (r[0] == 'r')
    return stringToNum(r+1);

  else if (strcmp("PC", r) == 0)
    return (uint32_t) PC;

  else if (strcmp("SP", r) == 0)
    return (uint32_t) SP;

  else if (strcmp("LP", r) == 0)
    return (uint32_t) LP;

  else if (strcmp("CPSR", r) == 0)
    return (uint32_t) CPSR;


  return UINT32_MAX;

}

uint32_t stringToNum (char *str){

  long num;

  //hex
  if (strchr(str, 'x') || strchr(str,'X')){
    num = strtol(str, NULL, 16);
  }

  //dec
  else{
    num = strtol(str, NULL, 10);
  }

  return (uint32_t) num;
}



int findInsNum(char *ins) {
  char *opcodes[] = {"add","sub","rsb","and","eor","orr","mov","tst","teq","cmp","mul","mla"};//sdt
  for(int i = 0; i < 12; i++) {
    if(strcmp(ins, opcodes[i]) == 0) {
      return i;
    }
  }
  return -1;
}

size_t convertDataProcess(struct assemblyCode *input, FILE *fout) {
  struct symbolTable *table = setupTable();
  struct tokenedCode* tokens = setupTokens(input);
  char *binString;
  binString=data_process_ins_assembler(tokens->code->line,tokens->code->numTokens,findInsNum(tokens->code->line[0]));
  size_t result;
  result = (size_t) strtol(binString, NULL, 2);
  return result;
}

size_t convertMultiply(struct assemblyCode *input, FILE *fout) {
  struct symbolTable *table = setupTable();
  struct tokenedCode* tokens = setupTokens(input);
  char *binString;
  binString=multiply_ins_assembler(tokens->code->line,tokens->code->numTokens,findInsNum(tokens->code->line[0]));
  size_t result;
  result = (size_t) strtol(binString, NULL, 2);
  return result;
}