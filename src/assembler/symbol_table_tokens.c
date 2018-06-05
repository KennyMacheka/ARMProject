//
// Created by Dhru on 25/05/2018.
//
//So strtok_r can be called (not part of c standard)
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include "symbol_table_tokens.h"

struct symbolTable *setupTable (){
  struct symbolTable *t = malloc(sizeof(struct symbolTable));
  t->addresses = NULL;
  t->label = NULL;
  t->size = 0;

  return t;
}

void insert(struct symbolTable *t, int address, char *label){
  t->addresses = realloc(t->addresses, sizeof(int)*(t->size+1));
  t->label = realloc(t->label, sizeof(char*)*(t->size+1));

  t->addresses[t->size] = address;
  t->label[t->size++] = label;
}

int get(struct symbolTable *t, char *label){
  for(int i = 0; i < t->size; i++){
    if(strcmp(t->label[i], label) == 0){
      return t->addresses[i];
    }
  }
  return -1;
}


void tokenInstruction (char *instruction, struct tokenedInstruction *tokens){
  //pre: tokens->numTokens = 0
  char* copy = malloc(strlen(instruction)+1);
  char* rest = copy;
  strcpy(copy,instruction);

  const char* delims = " ,";

  char *token = strtok_r(rest, delims, &rest);
  int shouldBreak = 0;
  while (token != NULL){
    tokens->line = realloc(tokens->line,sizeof(char *)*(tokens->numTokens+1));
    tokens->line[tokens->numTokens] = (char *) malloc(strlen(token));
    strcpy(tokens->line[tokens->numTokens++],token);

    if (shouldBreak)
      break;

    //If instruction has '[' we don't want to break up tokens anymore
    //A valid assembler program will have '[' at the end of an instruction so this will be valid
    token = strtok_r(rest, delims, &rest);
    if (strlen(rest)!= 0){
      if (rest[0] == '[') {
        token = rest;
        shouldBreak = 1;
      }
    }
  }

  free(copy);
}


struct tokenedCode *setupTokens(struct assemblyCode *input){
  struct tokenedCode* tokens = malloc(sizeof(struct tokenedCode));
  tokens->numLines = input->numLines;
  tokens->code = malloc(sizeof(struct tokenedInstruction)*tokens->numLines);

  for (int i = 0; i<input->numLines; i++){
    tokens->code[i].line = NULL;
    tokens->code[i].numTokens = 0;
    tokenInstruction(input->code[i], &tokens->code[i]);
  }

  return tokens;
}

void freeTokenedCode (struct tokenedCode **code){


  for (int i = 0; i<(*code)->numLines; i++){
    freeTokenedInstruction(&(*code)->code[i]);
  }
  free((*code)->code);
  free(*code);
  *code = NULL;

}
void freeTokenedInstruction(struct tokenedInstruction *tokens ){

  //Caller is responsible for freeing tokens if it was allocated via memory
  //This function doesn't do this as it was designed for working with tokenedCode
  //tokenedCode only has allocated memory for an array of tokenedInstruction s, not each individiaul one

  for (int i = 0; i<tokens->numTokens; i++){
    free(tokens->line[i]);
  }

  free(tokens->line);
}

void printTokens (struct tokenedCode* tokens){

  for (int i = 0; i<tokens->numLines;i++){
    for (int j = 0; j<tokens->code[i].numTokens; j++){
      printf("Token: %s ", tokens->code[i].line[j]);
    }
    printf("\n");
  }

}
