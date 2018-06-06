//
// Created by kenny on 05/06/18.
//

#include <string.h>
#include <stdint.h>
#include "convert_instructions.h"
#include "symbol_table_tokens.h"
#include "../Utilities/bit_operations_utilities.h"


void convert(struct assemblyCode *input){

  //Each p

  struct symbolTable *table = setupTable();
  struct tokenedCode* tokens = setupTokens(input);

  for (int i = 0; i<input->numLines; i++){
    if (strchr(input->code[i],':')){
      insert(table, i, input->code[i]);
    }
  }

  for (int i = 0; i<input->numLines; i++){
    char *op = tokens->code->line[0];
    uint32_t machineCode;
    if (op[0] == 'b'){
      machineCode = handleBranch(tokens->code, table, i);
    }
  }
}


uint32_t handleBranch (struct tokenedInstruction *tokens, struct symbolTable* table, int pos){

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




  return machineCode;

}