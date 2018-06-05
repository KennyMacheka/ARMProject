//
// Created by kenny on 03/06/18.
//

#ifndef ARM11_35_FILE_HANDLING_H
#define ARM11_35_FILE_HANDLING_H
#include <stddef.h>
#include <stdio.h>
extern const int MAX_CHARS;
struct assemblyCode{
  char **code;
  size_t numLines;
};

struct assemblyCode *readFile(FILE*);
void outputCode (struct assemblyCode*);
void freeCode(struct assemblyCode **);
#endif //ARM11_35_FILE_HANDLING_H
