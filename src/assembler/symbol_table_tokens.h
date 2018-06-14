//
// Created by kenny on 04/06/18.
//

#ifndef ARM11_35_SYMBOL_TABLE_H
#define ARM11_35_SYMBOL_TABLE_H

#include <stddef.h>
#include "file_handling.h"


struct symbolTable {
    int *addresses;
    char **label;
    int size;
};

struct tokenedInstruction {
    char **line;
    size_t numTokens;
};

struct tokenedCode {
    struct tokenedInstruction *code;
    size_t numLines;
};


struct symbolTable *setupTable();

void insert(struct symbolTable *, int, char *);

int get(struct symbolTable *, char *);

void freeSymbolTable(struct symbolTable **);

void tokenInstruction(char *, struct tokenedInstruction *);

struct tokenedCode *setupTokens(struct assemblyCode *);

void freeTokenedCode(struct tokenedCode **);

void freeTokenedInstruction(struct tokenedInstruction *);

void printTokens(struct tokenedCode *);


#endif //ARM11_35_SYMBOL_TABLE_H
