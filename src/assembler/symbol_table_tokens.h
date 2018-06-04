//
// Created by kenny on 04/06/18.
//

#ifndef ARM11_35_SYMBOL_TABLE_H
#define ARM11_35_SYMBOL_TABLE_H

struct symbolTable{
  int* addresses;
  char **label;
  int size;
};

struct tokenedCode{
  char **tokens;
  int numTokens;
};


void setupTable (struct symbolTable*);
void insert(struct symbolTable *, int , char *);

#endif //ARM11_35_SYMBOL_TABLE_H
