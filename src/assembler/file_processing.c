//
// Created by Dhru on 31/05/2018.
//


#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_LENGTH 7
#define MAX_LINE_LENGTH 10



/*
 * 1) Reader : File -> Line arrays
 * 2) Tokeniser : Line Arrays : Split operands
 */

/* reads the input file and separates each instruction
** into an array
*/
void reader(char *src) {

  FILE *file;
  file = fopen(src, "r");
  printf("test");

  char **lines;
  lines = malloc(MAX_FILE_LENGTH * sizeof(char));
  for(int i = 0; i < MAX_FILE_LENGTH; i++){
      lines[i] = malloc(MAX_LINE_LENGTH * sizeof(char));
  }

  int x = 0;
  if(file != NULL){
      fscanf(file, "%[^n]", lines[x]);
      //printf("%s", lines[x]);
      x++;
  }

  fclose(file);
  //tokeniser(lines);
  
}
/*
char ***tokeniser(char ** lines){

//char ***tokens = malloc()

}
*/


/*
//writes binary represenation to binary file
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
int main(int argc, char **argv)
{
    /* code */
    reader(argv[1]);
    return 0;
}
