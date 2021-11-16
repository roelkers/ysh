#include <stdio.h>
#include <stdlib.h>

#define WORDBUFLENGTH 100

void splitIntoWords(char* input, char * words[50]) {
  int i = 0;
  char c;
  char * p;
  words[i] = malloc(WORDBUFLENGTH);
  p = words[i];
  while((c = *input) != '\0') {
    if(c == ' '){
      *p = '\0';
      //printf("found space %c\n", *p);
      i++;
      words[i] = malloc(WORDBUFLENGTH);
      p = words[i];
    } else {
      *p = c;
      //printf("found character %c\n", *p);
      p++;
    }
    input++;
  }
  words[i] = '\0';
}

int main() {
  
  char input[50];
  char * words[50];
  while(1) {
    printf("#");
    scanf("%[^\n]%*c", input);
    splitIntoWords(input, words);
  }
}
