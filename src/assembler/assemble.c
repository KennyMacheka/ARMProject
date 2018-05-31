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

int main(int argc, char **argv) {
  return EXIT_SUCCESS;
}
