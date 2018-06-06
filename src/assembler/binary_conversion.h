//
// Created by pc on 06/06/2018.
//

#ifndef ARM11_35_BINARY_CONVERSION_H
#define ARM11_35_BINARY_CONVERSION_H
/*
#define MAX_DP_TOKEN_LENGTH 14
#define MAX_MUL_TOKEN_LENGTH 4
#define MAX_SDT_TOKEN_LENGTH 20
 */

char* data_process_ins_assembler(char **tokenised_ins, int tokenCount, int insNum);
char* multiply_ins_assembler(char** tokenised_ins, int tokenCount, int insNum);
//char* single_data_transfer_ins_assembler(char **tokenised_ins, int tokenCount, int insNum);

#endif //ARM11_35_BINARY_CONVERSION_H
