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



int[0] convert_to_int(char *binary);

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


/* Input is a tokenised instruction with instruction number given(see doc/instructionNum)
 * Output is a string contains '0' and '1' represents required binary number
  */
char *data_process_ins_assembler(char **tokenised_ins, int insNum) {
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
