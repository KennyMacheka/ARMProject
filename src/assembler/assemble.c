#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "file_handling.h"
#include "convert_instructions.h"

int main(int argc, char **argv) {

  if (argc < 3) {
    printf("Invalid file names.\n");
    return EXIT_FAILURE;
  }

  FILE *fin = fopen(argv[1], "r");
  FILE *fout = fopen(argv[2], "wb");

  if (fin == NULL) {
    printf("Invalid file input.\n");
    return EXIT_FAILURE;
  }


  struct assemblyCode *input = readFile(fin);
  fclose(fin);

  //Convert to machine code and write to fout
  convert(input, fout);

  freeCode(&input);
  fclose(fout);

  return EXIT_SUCCESS;
}
