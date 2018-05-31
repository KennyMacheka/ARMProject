#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/*
 *1. read line store in string
 *2. add labels to symbol table
 *3. read again
 *4. parse opcode and operands (tokeniser)
 *5. convert to binary (lookup table filled with binary represenations)
 *6. write binary to file (use binary writer)
 *7. next line (check file not ended, loop condition)
 *8. back to stage 1
 */

//splits file into separate lines
char **reader(char *src) {

  FILE *file;
  file = fopen(src, "r");

  char **lines;
  int x = 0;

  if (file) {
    while (!feof(file)) {
      lines[x] = (char *) fgetc(file);
    }
  }

  fclose(file);
  return lines;
}

//writes binary represenation to binary file
void writer(char *binary, char *dest) {
  FILE *file;
  file = fopen(dest, "wb");
  int bin = convert_to_int(binary);
  if (file != NULL) {
    fwrite(bin, sizeof(int), 1, file);
  }

  fclose(file);
}
int convert_to_int(char *binary) {
  int ints[sizeof(binary)/ 32];

  for (int i = 0; i < sizeof(binary) / 32; i++) {
    char word[32];
    for (int j = 0; j < 32; j++) {
      word[j] = binary[j + i * 32];
    }
    ints[i] = strtol(word, NULL, 2);
  }
  return ints;
}

/* Turn a string start with '#', might be decimal(#nnn) or hexadecimal(#0xnnn)
 * (NOTE: ASSUME non-negative)
   into a string represents the corresponding 12-bit binary number which first 4 bits are in
   rotate field and last 8 bits are numbers.
   This 12-bit number actually represents a 32-bit number which is got by represent the 8-bit number
   in 32-bit format and filling 0 in higher bits, then rotate right the 32-bit number by (4-bit number * 2)
*/
//Q: e.g. how to represent #0x1999? Just error? however is within memory address limit
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
  //TODO: turn num into binary specified above
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
  if(sigFigNo > 8) {
    fprintf(stderr, "error: cannot represent the immediate accurately");
  } else {
    if(sigFigNo + 31 - lastSig <= 8) {
      
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

/* take in a token, if is immediate, return 1; else return 0
 * */
int isImm(char token[]){
  if(token[0] == '#')
    return 1;
  return 0;
}


/* Input is a tokenised instruction with instruction number given(see doc/instructionNum)
 * Output is a string contains '0' and '1' represents required binary number
 * */
//TODO:remember to free result in the caller
//TODO:把相似部分拿出来，以优化switch,减少重复
char* data_process_ins_assembler(char tokenised_ins[][14], int tokenCount, int insNum) {
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
		  result[6] = (char) ((int) '0' + isImm(tokenised_ins[3])); //I
      result[7] = '0';//Opcodes
      result[8] = '0';
      result[9] = '0';
      result[10] = '0';
      result[11] = '0'; //S
      char biRnNo[5] = "";
      regNumToBinary(tokenised_ins[2], biRnNo);
      result[12] = biRnNo[0];//Rn
      result[13] = biRnNo[1];
      result[14] = biRnNo[2];
      result[15] = biRnNo[3];
      char biRdNo[5] = "";
      regNumToBinary(tokenised_ins[1], biRdNo);
      result[16] = biRdNo[0];//Rd
      result[17] = biRdNo[1];
      result[18] = biRdNo[2];
      result[19] = biRdNo[3];
      char biOp2No[13] = "";
      //if(result[6]){
      immToBinary(tokenised_ins[3], biOp2No);
      for(int i = 0; i < 12; i++) {
        result[20+i] = biOp2No[i];
      }
      //} //else shifted register(optional)
    case 4://eor
    case 1://sub
    case 2://rsb
    case 0://add
    case 5://orr
    case 6://mov
    case 7://tst
    case 8://teq
    case 9://cmp
		default: fprintf(stderr, "Invalid data processing instruction.\n"); //what should return?
  }
  return result;
}

int main(int argc, char **argv) {
  return EXIT_SUCCESS;
}
