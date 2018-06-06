//
// Created by kenny on 05/06/18.
//
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "convert_instructions.h"
#include "symbol_table_tokens.h"
#include "../Utilities/bit_operations_utilities.h"

uint32_t stringToNum (char *);
uint32_t convertBranch (struct tokenedInstruction *, struct symbolTable* , int);
uint32_t convertSingleDataTransfer (struct tokenedInstruction *, struct symbolTable*,
                                    int , size_t, bool);

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
      machineCode[machineLines++] = convertBranch(tokens->code, table, i);

    else if (strcmp(op, "ldr") == 0)
      machineCode[machineLines++] = convertSingleDataTransfer(tokens->code, table, i, input->numLines, true);

    else if (strcmp(op, "str") == 0)
      machineCode[machineLines++] = convertSingleDataTransfer(tokens->code, table, i, input->numLines, false);
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

  //Set L bit
  if (isLoading){
    setBit(&machineCode, 20, 1);
  }

  //Set Rd bits (note, register value is representable by 4 bits max)
  uint32_t rd = stringToNum(tokens->line[1]+1);
  rd <<= 12;
  machineCode |= rd;

  //Check operand 2 is an expression
  if (tokens->line[2][0] == '=' && isLoading){
    uint32_t constant = stringToNum(tokens->line[2]+1);

    //Otherwise use mov
    if (constant > 0xFF){
      int dataPos = ((int)numInstructions-table->size)+storedConstants;
      constants = realloc(constants,sizeof(uint32_t)*(storedConstants+1));
      constants[storedConstants++] = constant;

      //Set PC and offset here, no need to convert to string

    }

    //else, treat as mov (move data processing function into this file)
    else{

    }

    /**
        Here, Rn is the pC, so our offset just needs to be address of constant - pos - 8?
        This constant needs to be put at the end of the program (as far to the end as possible).
        Then I think offset is calculated by address of that value - current address - 8 (PC ahead by 8 bytes)
        The end of the program can be calculated by how many instructions I have in total - the number of lebels
     */

  }

  else if



  return machineCode;

}

uint32_t registerValue (char* r){
  assert (r != NULL);

  if (r[0] == 'r')
    return stringToNum(r+1);

  else if (strcmp("PC", r) == 0)
    return 15;

  //Do other registers
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