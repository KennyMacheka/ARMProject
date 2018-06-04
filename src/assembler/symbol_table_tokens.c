//
// Created by Dhru on 25/05/2018.
//

#include <string.h>
#include <stdlib.h>
#include "symbol_table_tokens.h"

void setupTable (struct symbolTable* t){
  t->addresses = NULL;
  t->label = NULL;
  t->size = 0;
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






