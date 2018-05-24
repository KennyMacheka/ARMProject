#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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


/* Input is a tokenised instruction with instruction number given(see doc/instructionNum)
 * Output is a string contains '0' and '1' represents required binary number
 * */
char* data_process_ins_assembler(char** tokenised_ins, int insNum) {
  char result[33] = "";
  switch (insNum) {
    case 3://and

    case 4://eor
    case 1://sub
    case 2://rsb
    case 0://add
    case 5://orr
    case 6://mov
    case 7://tst
    case 8://teq
    case 9://cmp
    default:
      fprintf(stderr, "invalid data processing instruction\n");
  }
  return result;
}





int main(int argc, char **argv) {
  return EXIT_SUCCESS;
}
