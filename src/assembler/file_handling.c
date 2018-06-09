//
// Created by kenny on 03/06/18.
//
//This is needed for getline to be valid
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_handling.h"

//Extra byte is the null character
const int MAX_CHARS = 512;

//NOTE:
//If needed, can add error if line exceeds 511 by checking variable size (i.e. size>MAX_CHARS)
//We can check for this in main by returning NULL
struct assemblyCode *readFile(FILE* file) {
  if (file == NULL)
    return NULL;


  struct assemblyCode *input = (struct assemblyCode *) malloc(sizeof(struct assemblyCode));
  input->code = NULL;
  input->numLines = 0;

  char *line = (char*) (malloc(MAX_CHARS));
  size_t  size = MAX_CHARS;
  ssize_t  charsRead;
  while ((charsRead = getline(&line,&size, file)) != -1){
    //Find "\n" if one is present
   if (line[charsRead-1] == '\n') {
     line[charsRead - 1] = '\0';
     //Empty line
     if (charsRead == 1)
       continue;
   }

   input->code = realloc(input->code,sizeof(char * )*(input->numLines+1));
   input->code[input->numLines] = (char *) malloc(MAX_CHARS);
   strcpy(input->code[input->numLines++], line);
  }
  return input;
}

void outputCode (struct assemblyCode* input){
  for (int i = 0; i<input->numLines; i++)
    printf("%s\n", input->code[i]);
}

void freeCode(struct assemblyCode **code){

  for (int i = 0; i<(*code)->numLines; i++)
    free((*code)->code[i]);

  free((*code)->code);
  free(*code);
  *code = NULL;
}
