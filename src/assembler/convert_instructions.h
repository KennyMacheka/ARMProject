//
// Created by kenny on 05/06/18.
//

#ifndef ARM11_35_CONVERT_INSTRUCTIONS_H
#define ARM11_35_CONVERT_INSTRUCTIONS_H
#include "file_handling.h"

size_t convert(struct assemblyCode *, FILE *);
size_t convertDataProcess(struct assemblyCode *input, FILE *fout);
size_t convertMultiply(struct assemblyCode *input, FILE *fout);
size_t convertSDT(struct assemblyCode *input, FILE *fout);

#endif //ARM11_35_CONVERT_INSTRUCTIONS_H
