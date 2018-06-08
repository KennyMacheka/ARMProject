#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "binary_conversion.h"



void numToBinChar4(uint16_t num, char result[]) {
  uint16_t mask = 1 << 3;
  for(int i = 0; i < 4; i++) {
    if((num & mask) == 0) {
      result[i] = '0';
    } else {
      result[i] = '1';
    }
    num = num << 1;
  }
}

/* Turn a string start with '#', might be decimal(#nnn) or hexadecimal(#0xnnn)
 * (NOTE: ASSUME non-negative)
   into a string represents the corresponding 12-bit binary number which first 4 bits are in
   rotate field and last 8 bits are numbers.
   This 12-bit number actually represents a 32-bit number which is got by represent the 8-bit number
   in 32-bit format and filling 0 in higher bits, then rotate right the 32-bit number by (4-bit number * 2)
*/
void immToBinary(char imm[], char result[]) {
  assert(imm[0] == '#');
  uint32_t num = 0;
  int bits = 0;
  if(imm[1] == '0' && imm[2] == 'x') {//hexadecimal
    for(int i = 3; imm[i]; i++) {
      bits++;
    }
    for(int i = 3; imm[i]; i++) {
      if(imm[i] >= '0' && imm[i] <= '9'){
        num += (imm[i]- '0') * (int)pow(16, bits-i+2);
      } else if(imm[i] >= 'a' && imm[i] <= 'f') {
        num += (imm[i] - 'a' + 10) * (int)pow(16, bits-i+2);
      } else if(imm[i] >= 'A' && imm[i] <= 'F') {
        num += (imm[i] - 'A' + 10) * (int)pow(16, bits-i+2);
      }
    }
  } else {//decimal
    for(int i = 1; imm[i]; i++) {
      bits++;
    }
    for(int i = 1; imm[i]; i++) {
      assert(imm[i] >= '0' && imm[i] <= '9');
      num += (imm[i]- '0') * (int)pow(10, bits-i);
    }
  }
  //Turn num into specified binary
  char biNum[33] = "";
  uint32_t mask = 1 << 31;
  for(int i = 0; i < 32; i++) {
    if((num & mask) == 0) {
      biNum[i] = '0';
    } else {
      biNum[i] = '1';
    }
    num <<= 1;
  }
  int lastSig = 0;
  int firstSig = 0;
  for(int i = 31; i >= 0; i--) {
    if(biNum[i] == '1') {
      lastSig = i;
      break;
    }
  }
  for(int i = 0; i < 32; i++) {
    if(biNum[i] == '1') {
      firstSig = i;
      break;
    }
  }
  int sigFigNo = lastSig - firstSig + 1;
  if(sigFigNo > 8 ||
     (sigFigNo == 8 && lastSig%2 == 0)) {
    fprintf(stderr, "error: cannot represent the immediate accurately");
  } else {
    for(int i = 0; i < sigFigNo; i++) {
      result[11-i] = biNum[lastSig + i];
    }
    if(sigFigNo != 8){
      for(int i = 0; i < 8- sigFigNo; i++) {
        result[11-sigFigNo-i] = '0';
      }
    }
    if(sigFigNo + 31 - lastSig <= 8) {
      result[0] = '0';//No rotation
      result[1] = '0';
      result[2] = '0';
      result[3] = '0';
    } else {
      int rotate0 = 31 - lastSig;
      if(rotate0 % 2 == 0) {
        uint16_t rotate = rotate0 / 2;
        numToBinChar4(rotate, result);
      } else {
        uint16_t rotate = (rotate0+1)/2;
        numToBinChar4(rotate, result);
        for(int i = 4; i < 11; i++) {
          result[i] = result[i+1];
        }
        result[11] = '0';
      }
    }
  }

}

/* When Operand2 is a shift register, deal with it another way (optional task) */

/* Turn a string start with 'r' followed by a decimal number
   into a string represents the number in 4-bit binary format
*/
void regNumToBinary(char regNum[],char result[]) {
  assert(regNum[0] == 'r');
  uint16_t num = 0;
  if(regNum[2]){ //when for Rn, n>=10
    num = 10;
    num += ((int)regNum[2] - (int)'0');
  } else {
    num += ((int)regNum[1] - (int)'0');
  }
  numToBinChar4(num, result);
}

/* take in a token, if is immediate, return 1; else return 0
 * */
int isImm(char token[]){
  if(token[0] == '#')
    return 1;
  return 0;
}

void dataProcess4tokenInit(char **tokenised_ins, char result[]) {
  result[6] = (char) ((int) '0' + isImm(tokenised_ins[3])); //I
  result[11] = '0'; //S
  regNumToBinary(tokenised_ins[2], &result[12]);//Rn
  regNumToBinary(tokenised_ins[1], &result[16]);//Rd
  //if(result[6]){
  immToBinary(tokenised_ins[3], &result[20]);
  //} //else shifted register(optional)
}

void dataProcessCPSRInit(char **tokenised_ins, char result[]) {
  result[6] = (char) ((int) '0' + isImm(tokenised_ins[2])); //I
  result[11] = '1'; //S
  regNumToBinary(tokenised_ins[1], &result[12]);//Rn
  result[16] = '0';//Rd
  result[17] = '0';
  result[18] = '0';
  result[19] = '0';
  //if(result[6]){
  immToBinary(tokenised_ins[3], &result[20]);
  //} //else shifted register(optional)
}

/* Input is a tokenised instruction with instruction number given(see doc/instructionNum)
 * Output is a string contains '0' and '1' represents required binary number
 * */
char* data_process_ins_assembler(char **tokenised_ins, int tokenCount, int insNum) {
  char *result = calloc(33, sizeof(char));
  result[0] = '1';//cond
  result[1] = '1';
  result[2] = '1';
  result[3] = '0';
  result[4] = '0';//00
  result[5] = '0';
  switch (insNum) {
    case 3://and
      assert(tokenCount == 4);
      dataProcess4tokenInit(tokenised_ins, result);
      result[7] = '0';//Opcodes
      result[8] = '0';
      result[9] = '0';
      result[10] = '0';
      break;
    case 4://eor
      assert(tokenCount == 4);
      dataProcess4tokenInit(tokenised_ins, result);
      result[7] = '0';//Opcodes
      result[8] = '0';
      result[9] = '0';
      result[10] = '1';
      break;
    case 1://sub
      assert(tokenCount == 4);
      dataProcess4tokenInit(tokenised_ins, result);
      result[7] = '0';//Opcodes
      result[8] = '0';
      result[9] = '1';
      result[10] = '0';
      break;
    case 2://rsb
      assert(tokenCount == 4);
      dataProcess4tokenInit(tokenised_ins, result);
      result[7] = '0';//Opcodes
      result[8] = '0';
      result[9] = '1';
      result[10] = '1';
      break;
    case 0://add
      assert(tokenCount == 4);
      dataProcess4tokenInit(tokenised_ins, result);
      result[7] = '0';//Opcodes
      result[8] = '1';
      result[9] = '0';
      result[10] = '0';
      break;
    case 5://orr
      assert(tokenCount == 4);
      dataProcess4tokenInit(tokenised_ins, result);
      result[7] = '1';//Opcodes
      result[8] = '1';
      result[9] = '0';
      result[10] = '0';
      break;
    case 6://mov
      assert(tokenCount == 3);
      result[6] = (char) ((int) '0' + isImm(tokenised_ins[2])); //I
      result[7] = '1';//Opcodes
      result[8] = '1';
      result[9] = '0';
      result[10] = '1';
      result[11] = '0'; //S
      result[12] = '0';//Rn which is undefined
      result[13] = '0';
      result[14] = '0';
      result[15] = '0';
      regNumToBinary(tokenised_ins[1], &result[16]);//Rd
      //if(result[6]){
      immToBinary(tokenised_ins[2], &result[20]);
      //} //else shifted register(optional)
      break;
    case 7://tst
      assert(tokenCount == 3);
      dataProcessCPSRInit(tokenised_ins, result);
      result[7] = '1';//Opcodes
      result[8] = '0';
      result[9] = '0';
      result[10] = '0';
      break;
    case 8://teq
      assert(tokenCount == 3);
      dataProcessCPSRInit(tokenised_ins, result);
      result[7] = '1';//Opcodes
      result[8] = '0';
      result[9] = '0';
      result[10] = '1';
      break;
    case 9://cmp
      assert(tokenCount == 3);
      dataProcessCPSRInit(tokenised_ins, result);
      result[7] = '1';//Opcodes
      result[8] = '0';
      result[9] = '1';
      result[10] = '0';
      break;
    default: fprintf(stderr, "Invalid data processing instruction.\n"); //what should return?
  }
  return result;
}

void mulInsInit(char **tokenised_ins, char result[]) {
  regNumToBinary(tokenised_ins[1], &result[12]);//Rd
  regNumToBinary(tokenised_ins[2], &result[28]);//Rm
  regNumToBinary(tokenised_ins[3], &result[20]);//Rs
}

/* Input is a tokenised instruction with instruction number given(see doc/instructionNum)
* Output is a string contains '0' and '1' represents required binary number
* */
char* multiply_ins_assembler(char **tokenised_ins, int tokenCount, int insNum) {
  char *result = calloc(33, sizeof(char));
  result[0] = '1';//cond
  result[1] = '1';
  result[2] = '1';
  result[3] = '0';
  for(int i = 0; i < 6; i++) {//000000
    result[4+i] = '0';
  }
  result[11] = '0'; //S
  result[24] = '1';//1001
  result[25] = '0';
  result[26] = '0';
  result[27] = '1';
  if(insNum == 10) { //mul
    assert(tokenCount == 4);
    mulInsInit(tokenised_ins, result);
    result[10] = '0'; //A
    result[16] = '0'; //Rn which is undefined
    result[17] = '0';
    result[18] = '0';
    result[19] = '0';
  } else if(insNum == 11) {//mla
    assert(tokenCount == 5);
    mulInsInit(tokenised_ins, result);
    result[10] = '1'; //A
    regNumToBinary(tokenised_ins[4], &result[16]);//Rn
  } else {
    fprintf(stderr, "Invalid multiply instruction.\n");//return?
  }
  return result;
}

