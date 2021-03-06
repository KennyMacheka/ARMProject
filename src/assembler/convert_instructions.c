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
#include "../Utilities/bit_operations_utilities.h"
#include "../Emulator/processor_data_handling.h"
#include "symbol_table_tokens.h"


//8 bits (whole words are being processed at a time here)
#define PC_AHEAD 2
#define MEMORY_ADDRESSES_PER_WORD 4

uint32_t shortenBits(uint32_t, int);

uint32_t stringToNum(char *);

uint32_t convertBranch(struct tokenedInstruction *, struct symbolTable *, int);

uint32_t convertSingleDataTransfer(struct tokenedInstruction *, struct symbolTable *,
                                   int, size_t, bool);

uint32_t convertDataProcess(struct tokenedInstruction *);

uint32_t convertMultiply(struct tokenedInstruction *);

uint32_t registerValue(char *);

int storeConstant(uint32_t *, uint32_t);

void storeShiftRegister(uint32_t *, char *, char *, char *);

void setRegAndOffset(uint32_t *, char *, char *);

void outputMachineCode(uint32_t *, int);


static int storedConstants = 0;
static uint32_t *constants = NULL;

size_t convert(struct assemblyCode *input, FILE *fout) {

  struct symbolTable *table = setupTable();
  struct tokenedCode *tokens = setupTokens(input);
  uint32_t *machineCode = malloc(sizeof(uint32_t) * input->numLines);
  uint32_t machineLines = 0;
  char *colon;
  for (int i = 0; i < input->numLines; i++) {
    colon = strchr(input->code[i], ':');
    if (colon) {
      *colon = '\0';
      insert(table, machineLines, input->code[i]);
    } else
      machineLines++;
  }

  machineLines = 0;

  for (int i = 0; i < input->numLines; i++) {
    char *op = tokens->code[i].line[0];

    if (strchr(op, ':'))
      continue;

    else if (op[0] == 'b')
      machineCode[machineLines] = convertBranch(&tokens->code[i], table, machineLines);

    else if (strcmp(op, "mul") == 0 || strcmp(op, "mla") == 0)
      machineCode[machineLines] = convertMultiply(&tokens->code[i]);

    else if (strcmp(op, "ldr") == 0)
      machineCode[machineLines] = convertSingleDataTransfer(&tokens->code[i], table, machineLines, input->numLines,
                                                            true);

    else if (strcmp(op, "str") == 0)
      machineCode[machineLines] = convertSingleDataTransfer(&tokens->code[i], table, machineLines, input->numLines,
                                                            false);

    else if (strcmp(op, "andeq") == 0)
      machineCode[machineLines] = 0;

    else {

      if (strcmp(op, "lsl") == 0) {
        //changing lsl Rn, <#expression> to mov Rn, Rn, lsl <#expression>
        char *expr = tokens->code[i].line[2];
        char *rn = tokens->code[i].line[1];
        strcpy(tokens->code[i].line[0], "mov");
        tokens->code[i].line = realloc(tokens->code[i].line, sizeof(char *) * 5);
        tokens->code[i].line[3] = NULL;
        tokens->code[i].line[4] = NULL;
        tokens->code[i].line[4] = (char *) calloc(strlen(expr) + 1, 1);
        strcpy(tokens->code[i].line[4], expr);
        tokens->code[i].line[3] = (char *) calloc(4, 1);
        strcpy(tokens->code[i].line[3], "lsl");
        tokens->code[i].line[2] = realloc(tokens->code[i].line[2], strlen(rn) + 1);
        strcpy(tokens->code[i].line[2], rn);
        tokens->code[i].numTokens = 5;
      }

      //Instruction is data processing
      machineCode[machineLines] = convertDataProcess(&tokens->code[i]);
    }
    machineLines++;
  }


  int minusLabels = machineLines;
  int machineCodeSize;
  if (machineLines + storedConstants > input->numLines) {
    machineLines = input->numLines + (machineLines + storedConstants - input->numLines);
    machineCode = realloc(machineCode, sizeof(uint32_t) * machineLines);
    machineCodeSize = machineLines;
  } else {
    machineCodeSize = machineLines + storedConstants;
  }

  for (int i = 0; i < storedConstants; i++)
    machineCode[minusLabels + i] = constants[i];


  freeSymbolTable(&table);
  freeTokenedCode(&tokens);
  return fwrite(machineCode, 4, machineCodeSize, fout);

}

int findInsNum(char *ins) {
  char *opcodes[] = {"add", "sub", "rsb", "and", "eor", "orr", "mov", "tst", "teq", "cmp", "mul", "mla"};
  for (int i = 0; i < 12; i++) {
    if (strcmp(ins, opcodes[i]) == 0) {
      return i;
    }
  }
  return -1;
}

uint32_t convertDataProcess(struct tokenedInstruction *tokens) {
  uint32_t result = 0;
  setBit(&result, 31, 1);
  setBit(&result, 30, 1);
  setBit(&result, 29, 1);

  enum INS_TYPE {
      COMPUTES_RESULT,
      SINGLE_OPERAND,
      SET_CPSR
  } type;

  switch (findInsNum(tokens->line[0])) {
    case 3://and
      type = COMPUTES_RESULT;
      break;

    case 4://eor
      type = COMPUTES_RESULT;
      setBit(&result, 21, 1);
      break;

    case 1://sub
      type = COMPUTES_RESULT;
      setBit(&result, 22, 1);
      break;

    case 2://rsb
      type = COMPUTES_RESULT;
      setBit(&result, 22, 1);
      setBit(&result, 21, 1);
      break;

    case 0://add
      type = COMPUTES_RESULT;
      setBit(&result, 23, 1);
      break;

    case 5://orr
      type = COMPUTES_RESULT;
      setBit(&result, 24, 1);
      setBit(&result, 23, 1);
      break;

    case 6://mov
      type = SINGLE_OPERAND;
      setBit(&result, 24, 1);
      setBit(&result, 23, 1);
      setBit(&result, 21, 1);
      break;

    case 7://tst
      type = SET_CPSR;
      setBit(&result, 24, 1);
      break;

    case 8://teq
      type = SET_CPSR;
      setBit(&result, 24, 1);
      setBit(&result, 21, 1);
      break;

    case 9://cmp
      type = SET_CPSR;
      setBit(&result, 24, 1);
      setBit(&result, 22, 1);
      break;

    default:
      fprintf(stderr, "Invalid data processing instruction.\n");
      return 0;
  }

  if (type == COMPUTES_RESULT || type == SINGLE_OPERAND) {
    uint32_t rd = registerValue(tokens->line[1]);
    rd <<= 12;
    result |= rd;
  }

  if (type == SET_CPSR)
    setBit(&result, 20, 1);


  int rnPos;
  uint32_t rn;
  if (type == COMPUTES_RESULT)
    rnPos = 2;
  else if (type == SET_CPSR || type == SINGLE_OPERAND)
    rnPos = 1;


  if (type == COMPUTES_RESULT || type == SET_CPSR) {
    rn = registerValue(tokens->line[rnPos]);
    rn <<= 16;
    result |= rn;
  }

  //mov (single operand) doesn't have an rnPos, but this position number is simply for indexing operand2

  if (tokens->line[rnPos + 1][0] == '#') {
    setBit(&result, 25, 1);
    storeConstant(&result, stringToNum(tokens->line[rnPos + 1] + 1));
  } else {
    //Check whether  shiftname and shift register are present
    if ((type == COMPUTES_RESULT && tokens->numTokens == 6) ||
        (type == SET_CPSR && tokens->numTokens == 5) ||
        (type == SINGLE_OPERAND && tokens->numTokens == 5)) {
      storeShiftRegister(&result, tokens->line[rnPos + 1], tokens->line[rnPos + 2],
                         tokens->line[rnPos + 3]);
    } else
      storeShiftRegister(&result, tokens->line[rnPos + 1], NULL, NULL);

  }


  return result;
}


uint32_t convertMultiply(struct tokenedInstruction *tokens) {
  uint32_t result = 0;
  setBit(&result, 31, 1);
  setBit(&result, 30, 1);
  setBit(&result, 29, 1);
  setBit(&result, 7, 1);
  setBit(&result, 4, 1);

  uint32_t rd = registerValue(tokens->line[1]);
  uint32_t rm = registerValue(tokens->line[2]);
  uint32_t rs = registerValue(tokens->line[3]);
  rd <<= 16;
  rs <<= 8;
  result |= rs | rd | rm;

  if (strcmp(tokens->line[0], "mla") == 0) {
    setBit(&result, 21, 1);
    uint32_t rn = registerValue(tokens->line[4]);
    rn <<= 12;
    result |= rn;
  }

  return result;
}


uint32_t convertBranch(struct tokenedInstruction *tokens, struct symbolTable *table, int pos) {
  uint32_t machineCode = 0;
  setBit(&machineCode, 27, 1);
  setBit(&machineCode, 25, 1);

  char *opcode = tokens->line[0];
  char *branchTo = tokens->line[1];

  //Elements in opcodes and bits correspond
  char *opcodes[] = {"beq", "bne", "bge", "blt", "bgt", "ble", "bal", "b"};
  char *bits[] = {"0000", "0001", "1010", "1011", "1100", "1101", "1110", "1110"};

  for (int i = 0; i < 8; i++) {
    if (strcmp(opcode, opcodes[i]) == 0) {
      for (int j = 0; j < 4; j++) {
        if (bits[i][j] == '1')
          setBit(&machineCode, (uint32_t) 31 - j, 1);
      }
    }
  }

  uint32_t offset;
  uint32_t address = (uint32_t) get(table, branchTo);

  if (address == -1) {
    address = stringToNum(branchTo);
  }

  offset = address - ((uint32_t) pos) - PC_AHEAD;
  offset *= 4;
  offset = shortenBits(offset, 26);
  //We now have a signed offset (I used uint32_t as bit shifting is defined by the C standard)
  offset >>= 2;
  machineCode |= offset;


  return machineCode;

}

uint32_t convertSingleDataTransfer(struct tokenedInstruction *tokens, struct symbolTable *table,
                                   int pos, size_t numInstructions, bool isLoading) {

  uint32_t machineCode = 0;
  setBit(&machineCode, 31, 1);
  setBit(&machineCode, 30, 1);
  setBit(&machineCode, 29, 1);
  setBit(&machineCode, 26, 1);
  //set to add by default
  setBit(&machineCode, 23, 1);
  struct tokenedInstruction address;
  uint32_t offset;

  address.numTokens = 0;
  address.line = NULL;
  //Set L bit
  if (isLoading) {
    setBit(&machineCode, 20, 1);
  }

  //Set Rd bits (note, register value is representable by 4 bits max)
  uint32_t rd = registerValue(tokens->line[1]);
  rd <<= 12;
  machineCode |= rd;

  //Check operand 2 is an expression
  if (tokens->line[2][0] == '=' && isLoading) {
    setBit(&machineCode, 24, 1);
    uint32_t constant = stringToNum(tokens->line[2] + 1);

    if (constant > 0xFF) {
      uint32_t pcAddress = pos + PC_AHEAD;
      constants = realloc(constants, sizeof(uint32_t) * (storedConstants + 1));
      constants[storedConstants++] = constant;

      //The expression in brackets is the memory location of the stored constant
      offset = (((uint32_t) numInstructions - table->size) + storedConstants - 1) - pcAddress;
      offset *= MEMORY_ADDRESSES_PER_WORD;

      uint32_t pc = (uint32_t) PC;
      //PC is rn
      pc <<= 16;
      machineCode |= pc;
      machineCode |= offset;
    }

      //Otherwise use mov
    else {
      strcpy(tokens->line[0], "mov");
      tokens->line[2][0] = '#';
      return convertDataProcess(tokens);
    }
  }

    //Form ldr/str rd, [...]
  else if (tokens->line[2][0] == '[' && tokens->numTokens == 3) {

    setBit(&machineCode, 24, 1);
    //Remove ']'
    tokens->line[2][strlen(tokens->line[2]) - 1] = '\0';
    tokenInstruction(tokens->line[2] + 1, &address);

    uint32_t rn = registerValue(address.line[0]);
    rn <<= 16;
    machineCode |= rn;

    //Of form [RN]
    if (address.numTokens == 1);


    else if (address.line[1][0] == '#') {
      int offset = 0;
      if (address.line[1][1] == '-') {
        setBit(&machineCode, 23, 0);
        offset = 1;
      }
      machineCode |= stringToNum(address.line[1] + 1 + offset);
    }

      //Shift register
    else {
      setBit(&machineCode, 25, 1);
      int registerPos = 0;

      if (address.line[1][0] == '-') {
        setBit(&machineCode, 23, 0);
        registerPos = 1;
      }

      //No shifting
      if (address.numTokens == 2)
        storeShiftRegister(&machineCode, address.line[1] + registerPos, NULL, NULL);

      else
        storeShiftRegister(&machineCode, address.line[1] + registerPos, address.line[2], address.line[3]);
    }
  }

  //post indexing
  else {
    int rnPos = 2;
    int regExprPos = 3;
    int shiftName = 4;
    int shiftExprReg = 5;

    tokens->line[rnPos][strlen(tokens->line[rnPos]) - 1] = '\0';
    uint32_t rn = registerValue(tokens->line[rnPos] + 1);
    rn <<= 16;
    machineCode |= rn;

    if (tokens->line[regExprPos][0] == '#') {
      //Intentional, I was aware this would hide the other variable called offset in this scope
      int offset = 0;
      if (tokens->line[regExprPos][1] == '-') {
        setBit(&machineCode, 23, 0);
        offset = 1;
      }
      machineCode |= stringToNum(tokens->line[regExprPos] + 1 + offset);
    } else {
      setBit(&machineCode, 25, 1);
      int registerPos = 0;
      if (tokens->line[regExprPos][0] == '-') {
        setBit(&machineCode, 23, 0);
        registerPos = 1;
      }

      //No actual shifting
      if (tokens->numTokens == 4)
        storeShiftRegister(&machineCode, tokens->line[regExprPos] + registerPos, NULL, NULL);

      else
        storeShiftRegister(&machineCode, tokens->line[regExprPos] + registerPos,
                           tokens->line[shiftName], tokens->line[shiftExprReg]);
    }
  }

  return machineCode;
}

int storeConstant(uint32_t *machineCode, uint32_t val) {

  //We need to store 8 bit value (val) into 12 bit location
  uint32_t rotateAmount = 0;
  uint32_t seenValue = val;
  while (val > 255 || rotateAmount % 2 != 0) {
    val = rotateLeft(val, 1);
    rotateAmount++;
    if (val == seenValue) {
      fprintf(stderr, "error: cannot represent the immediate accurately");
      return 1;
    }
  }

  rotateAmount /= 2;
  rotateAmount <<= 8;

  *machineCode |= rotateAmount;
  *machineCode |= val;

  return 0;

}

void setRegAndOffset(uint32_t *machineCode, char *reg, char *expr) {
  assert (expr[0] == '#');
  uint32_t bReg = registerValue(reg);
  bReg <<= 16;
  *machineCode |= bReg;
  storeConstant(machineCode, stringToNum(expr + 1));
}

void storeShiftRegister(uint32_t *machineCode, char *baseReg, char *shiftName, char *regOrExpr) {


  uint32_t bReg = registerValue(baseReg);
  *machineCode |= bReg;

  if (!shiftName && !regOrExpr)
    return;

  if (strcmp(shiftName, "lsr") == 0)
    setBit(machineCode, 5, 1);

  else if (strcmp(shiftName, "asr") == 0)
    setBit(machineCode, 6, 1);

  else if (strcmp(shiftName, "ror") == 0) {
    setBit(machineCode, 5, 1);
    setBit(machineCode, 6, 1);
  }


  if (regOrExpr[0] == 'r') {
    setBit(machineCode, 4, 1);
    uint32_t shiftReg = registerValue(regOrExpr);
    shiftReg <<= 8;
    *machineCode |= shiftReg;
  } else {
    uint32_t constant = stringToNum(regOrExpr + 1);
    constant <<= 7;
    *machineCode |= constant;
  }

}

uint32_t registerValue(char *r) {
  /**Move constants in Emulator/processor_data_handling.c in separate file.*/
  assert (r != NULL);

  if (r[0] == 'r')
    return stringToNum(r + 1);

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

uint32_t shortenBits(uint32_t num, int size) {
  //If num is meant to be a signed number <= size bits, we remove all higher bits
  uint32_t result = num;
  for (int i = 31; i >= size; i--)
    setBit(&result, i, 0);

  return result;
}

uint32_t stringToNum(char *str) {

  long num;

  //hex
  if (strchr(str, 'x') || strchr(str, 'X')) {
    num = strtol(str, NULL, 16);
  }

    //dec
  else {
    num = strtol(str, NULL, 10);
  }

  return (uint32_t) num;
}


void outputMachineCode(uint32_t *machineCode, int numLines) {

  printf("Outputting:\n");

  for (int i = 0; i < numLines; i++) {
    uint32_t *bits = getBits(reverseEndianness(machineCode[i]));

    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 8; k++)
        printf("%d", bits[j * 8 + k]);

      printf(" ");
    }
    printf("\n");
    free(bits);
  }
}