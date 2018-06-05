#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "file_handling.h"
#include "symbol_table_tokens.h"
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


int main(int argc, char **argv) {

  if (argc < 3){
    printf("Invalid file names.\n");
    return EXIT_FAILURE;
  }

  FILE* file = fopen(argv[1],"r");
  if (file == NULL){
    printf("Invalid file input.\n");
    return EXIT_FAILURE;
  }

  struct assemblyCode *input = readFile(file);
  fclose(file);

  struct tokenedCode *tokens = setupTokens(input);


  freeCode(&input);



  return EXIT_SUCCESS;
}
