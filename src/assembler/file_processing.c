//
// Created by Dhru on 31/05/2018.
//


#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_LENGTH 7
#define MAX_LINE_LENGTH 511


 
/*
 * 1) Reader : File -> Line arrays : DONE
 * 2) Tokeniser : Line Arrays : Split operands
 *

/*
** Receives input from reader and splits each instruciton into 
** each part of the instruction by splitting at whitespace 
** Also removes ',' comma character between two registers
** 
*/
char ***tokeniser(char ** lines){
    //pre: lines is output from reader
    //post: no whitespace, no commas

  char ***tokens = malloc(MAX_FILE_LENGTH * sizeof(char**));
  for(int i = 0; i < MAX_FILE_LENGTH; i++){

  }

}


/*
**writes binary represenation to binary file
**will need to convert each 32 character string into a binary int
**and then write that to the file
**binary writer is little endian
*/

/*
void writer(char *binary, char *dest) {
    
  FILE *file;
  file = fopen(dest, "wb");
  int bin = convert_to_int(binary);
  if (file != NULL) {
    fwrite(bin, sizeof(int), 1, file);
  }

  fclose(file);

}
*/

