//
// Created by kenny on 05/06/18.
//
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "convert_instructions.h"
#include "symbol_table_tokens.h"
#include "../Utilities/bit_operations_utilities.h"
#include "binary_conversion.h"

uint32_t handleBranch (struct tokenedInstruction *, struct symbolTable* , int);

size_t convertBranch(struct assemblyCode *input, FILE *fout){

  struct symbolTable *table = setupTable();
  struct tokenedCode* tokens = setupTokens(input);
  uint32_t machineCode[input->numLines];
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

    else if (op[0] == 'b'){
      machineCode[machineLines++] = handleBranch(tokens->code, table, i);
    }
  }

  //Check endianness of this
  return fwrite(machineCode, 4, machineLines, fout);

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

size_t convertSDT(struct assemblyCode *input, FILE *fout) {
  //TODO
}


uint32_t handleBranch (struct tokenedInstruction *tokens, struct symbolTable* table, int pos){

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
    //hex
    if (strchr(branchTo, 'x') || strchr(branchTo,'X')){
      address = strtol(branchTo, NULL, 16);
    }

    //dec
    else{
      address = strtol(branchTo, NULL, 10);
    }
  }

  address = ((uint32_t) pos) - address - pcAhead;
  //We now have a signed offset (I used uint32_t as bit shifting is defined by the C standard)
  address >>= 2;

  for (int i = 23; i>= 0; i++){
    setBit(&machineCode,i,isolateBits(address, i, i, 0));
  }

  return machineCode;

}