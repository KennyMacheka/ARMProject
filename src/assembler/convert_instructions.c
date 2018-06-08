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
#include "binary_conversion.h"

uint32_t stringToNum (char *);
uint32_t convertBranch (struct tokenedInstruction *, struct symbolTable * , int);
uint32_t convertSingleDataTransfer (struct tokenedInstruction *, struct symbolTable *,
                                    int , size_t, bool);

uint32_t convertDataProcess(struct tokenedInstruction *);
uint32_t convertMultiply(struct tokenedInstruction * );
uint32_t registerValue (char * );
int storeConstant (uint32_t*, uint32_t);
void storeShiftRegister (uint32_t * , char *, char *, char *);
void setRegAndOffset (uint32_t *, char *, char *);


static int storedConstants = 0;
static uint32_t *constants = NULL;

size_t convert(struct assemblyCode *input, FILE *fout){

  struct symbolTable *table = setupTable();
  struct tokenedCode* tokens = setupTokens(input);
  uint32_t *machineCode = malloc(sizeof(uint32_t)*input->numLines);
  uint32_t machineLines = 0;

  for (int i = 0; i<input->numLines; i++){
    if (strchr(input->code[i],':')){
      insert(table, i, input->code[i]);
    }
  }

  for (int i = 0; i<input->numLines; i++){
    char *op = tokens->code[i].line[0];
    if(strchr(op,':'))
      continue;

    else if (op[0] == 'b')
      machineCode[machineLines++] = convertBranch(&tokens->code[i], table, machineLines);

    else if (op[0] == 'm')
      machineCode[machineLines++] = convertMultiply(&tokens->code[i]);

    else if (strcmp(op, "ldr") == 0)
      machineCode[machineLines++] = convertSingleDataTransfer(&tokens->code[i], table, machineLines, input->numLines, true);

    else if (strcmp(op, "str") == 0)
      machineCode[machineLines++] = convertSingleDataTransfer(&tokens->code[i], table, machineLines, input->numLines, false);

    else if (strcmp(op, "andeq") == 0)
      machineCode[machineLines++] = 0;

    else {

      if (strcmp(op, "lsl") == 0){
        //changing lsl Rn, <#expression> to mov Rn, Rn, lsl <#expression>
        char *expr = tokens->code[i].line[2];
        char *rn = tokens->code[i].line[1];
        strcpy(tokens->code[i].line[0],"mov");
        tokens->code[i].line = realloc(tokens->code[i].line, 5);
        tokens->code[i].line[4] = (char *) malloc(strlen(expr));
        strcpy(tokens->code[i].line[4],expr);
        tokens->code[i].line[3] = (char *) malloc(3);
        strcpy(tokens->code[i].line[3], "lsl");
        tokens->code[i].line[2] = realloc(tokens->code[i].line[2],strlen(rn));
        strcpy(tokens->code[i].line[2], rn);
      }

      //Instruction is data processing
      machineCode[machineLines++] = convertDataProcess(&tokens->code[i]);
    }
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

int findInsNum(char *ins) {
  char *opcodes[] = {"add","sub","rsb","and","eor","orr","mov","tst","teq","cmp","mul","mla"};
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
  free(binString);
  return result;
}

size_t convertMultiply(struct assemblyCode *input, FILE *fout) {
  struct symbolTable *table = setupTable();
  struct tokenedCode* tokens = setupTokens(input);
  char *binString;
  binString=multiply_ins_assembler(tokens->code->line,tokens->code->numTokens,findInsNum(tokens->code->line[0]));
  size_t result;
  result = (size_t) strtol(binString, NULL, 2);
  free(binString);
  return result;
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
  setBit(&machineCode, 31, 1);
  setBit(&machineCode, 30, 1);
  setBit(&machineCode, 29, 1);
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
      strcpy(tokens->line[0],"mov");
      return convertDataProcess(tokens);
    }
  }

  //Form ldr/str rd, [...]
  else if (tokens->line[2][0] == '[' && tokens->numTokens == 3){

    setBit(&machineCode, 24, 1);
    uint32_t rn = registerValue(address.line[0]);
    rn <<= 16;
    machineCode |= rn;
    //Remove ']'
    tokens->line[2][strlen(tokens->line[2]) - 1] = '\0';
    tokenInstruction(tokens->line[2] + 1, &address);
    //Of form [RN]
    if (address.numTokens == 1) {
      storeConstant(&machineCode, 0);
    } else if (address.line[1][0] == '#')
      storeConstant(&machineCode, stringToNum(address.line[1] + 1));

    //Shift register
    else {
      int registerPos = 0;

      if (address.line[1][0] == '-') {
        setBit(&machineCode, 23, 0);
        registerPos = 1;
      }

      //No shifting
      if (address.numTokens == 2) {
        storeShiftRegister(&machineCode, address.line[1] + registerPos, NULL, NULL);
      }

      else
        storeShiftRegister(&machineCode, address.line[1] + registerPos, address.line[2], address.line[3]);
    }
  }

  //post indexing
  else{
    int rnPos = 2;
    int regExprPos = 3;
    int shiftName = 4;
    int shiftExprReg = 5;

    tokens->line[rnPos][strlen(tokens->line[rnPos])-1] = '\0';
    uint32_t rn = registerValue(tokens->line[rnPos]+1);
    rn <<= 16;
    machineCode |= rn;

    if (tokens->line[regExprPos][0] == '#'){
      storeConstant(&machineCode, stringToNum(tokens->line[regExprPos]+1));
    }

    else{
      int registerPos = 0;
      if (tokens->line[regExprPos][0] == '-'){
        setBit(&machineCode, 23, 0);
        registerPos = 1;
      }

      //No shifting
      if (tokens->numTokens == 4)
        storeShiftRegister(&machineCode, tokens->line[regExprPos] + registerPos, NULL, NULL);

      else
        storeShiftRegister(&machineCode, tokens->line[regExprPos] + registerPos,
                           tokens->line[shiftName], tokens->line[shiftExprReg]);
    }
  }

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

void setRegAndOffset (uint32_t *machineCode, char *reg, char *expr){
  assert (expr[0] == '#');
  uint32_t  bReg = registerValue(reg);
  bReg <<= 16;
  *machineCode |= bReg;
  storeConstant(machineCode, stringToNum(expr+1));
}

void storeShiftRegister (uint32_t *machineCode, char *baseReg, char *shiftName, char *regOrExpr){

  uint32_t  bReg = registerValue(baseReg);
  *machineCode |= bReg;

  if (!shiftName && !regOrExpr)
    return;

  if (strcmp(shiftName, "lsr") == 0)
    setBit(machineCode,5,1);

  else if (strcmp(shiftName, "asr") == 0)
    setBit(machineCode,6,1);

  else if (strcmp(shiftName, "ror") == 0){
    setBit(machineCode,5,1);
    setBit(machineCode,6,1);
  }


  if (regOrExpr[0] == 'r'){
    setBit(machineCode,4,1);
    uint32_t shiftReg = registerValue(regOrExpr);
    shiftReg <<= 8;
    *machineCode |= shiftReg;
  }

  else{
    uint32_t constant = stringToNum(regOrExpr+1);
    constant <<= 7;
    *machineCode |= constant;
  }

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

uint32_t convertDataProcess(struct tokenedInstruction *tokens) {
  char *binString;
  binString = data_process_ins_assembler(tokens->line,tokens->numTokens,findInsNum(tokens->line[0]));
  uint32_t result = (uint32_t) strtol(binString, NULL, 2);
  return result;
}

uint32_t convertMultiply(struct tokenedInstruction* tokens) {
  char *binString;
  binString= multiply_ins_assembler(tokens->line,tokens->numTokens,findInsNum(tokens->line[0]));
  uint32_t result = (uint32_t) strtol(binString, NULL, 2);
  return result;
}