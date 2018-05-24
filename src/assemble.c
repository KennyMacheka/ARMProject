#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

  if (file != NULL) {
    fwrite(binary, sizeof(binary), 1, file);
  }

  fclose(file);
}

/* Input is a string start with '#', might be decimal(#nnn) or hexadecimal(#0xnnn)
   Output is a string represents the corresponding 12-bit binary number which first 4 bits are in 
   rotate field and last 8 bits are numbers.
   This 12-bit number actually represents a 32-bit number which is got by represent the 8-bit number
   in 32-bit format and filling 0 in higher bits, then rotate right the 32-bit number by (4-bit number * 2)
*/
//Q: e.g. how to represent #0x1999? Just error? however is within memory address limit
char* immToBinary(char* imm) {
  char result[13] = "";
  //TODO
  return result;
}
/* When Operand2 is a shift register, deal with it another way (optional task) */

/* Input is a string start with 'r' followed by a decimal number
   Output is a string represents the number in 4-bit binary
*/
char* regNumToBinary(char* regNum) {
  char result[5] = "";
  //TODO
  return result;
}

/* Input is a tokenised instruction with instruction number given(see doc/instructionNum)
 * Output is a string contains '0' and '1' represents required binary number
 * */
char* data_process_ins_assembler(char** tokenised_ins, int insNum) {
  char result[33] = "111000";
  switch (insNum) {
    case 3://and
			//assert argument no. is 4
		  //check if tokenised_ins[3] is imm

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
