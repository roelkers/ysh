#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <unistd.h>
#include <stdbool.h>

#define WORDBUFLENGTH 100
#define MAXWORDS 50
#define MAXPATHDIRS 50

void splitPathIntoFolders(char* input, char * words[MAXWORDS]) {
  for(int j = 0; j < MAXWORDS -1; j++) {
    words[j] = NULL;
  }
  int i = 0;
  char c;
  char * p;
  words[i] = malloc(WORDBUFLENGTH);
  p = words[i];
  while((c = *input) != '\0') {
    if(c == ':'){
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

void splitIntoWords(char* input, char * words[MAXWORDS]) {
  for(int j = 0; j < MAXWORDS -1; j++) {
    words[j] = NULL;
  }
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


void executeCommand (char * binary_path) {
  // Argument Array
  char *const args[] = {"", NULL};
 // Environment Variable Array
  char *const env[] = {"", "", NULL};
 
  int res = execve(binary_path, args, env);
  perror("execve");
  if(res == 0) {
    printf("executed %s successfully", binary_path);
  } else {
    printf("%s exited with error code %i", binary_path, res);
  }
}

void findExecutable (char * command, char * dir) {
    struct dirent *directory;
    DIR *directory_reader = opendir(dir); 
  
    if (directory_reader == NULL) {
        printf("Could not open current directory" ); 
        exit(0);
    }
    while ((directory= readdir(directory_reader)) != NULL) {
      if(strcmp(directory->d_name, command) == 0) {
        char * absolute_exec_path = malloc(sizeof(dir) + sizeof(directory->d_name));
        absolute_exec_path = strcat(strcat(dir, "/"), directory->d_name);
        executeCommand(absolute_exec_path);
        return;
      }
    }
    closedir(directory_reader);     
}

char * loadPathFromEnvironment() {
  return getenv("PATH");
}

void parsePathEntries (char * pathString, char ** pathDirs) {
  splitPathIntoFolders(pathString, pathDirs);
}

void handleUserInput (char * words[MAXWORDS]) {
  //findExecutable("fahrenheit");
  char * pathDirs [MAXPATHDIRS]; 
  char * path = loadPathFromEnvironment();
  parsePathEntries(path, pathDirs);
  /* for(int i = 0; i < 6; i++) { */
  /*   printf("path : %s\n", pathDirs[i]); */
  /* } */
  /* printf("%s\n", words[0]); */
  /* printf("%s\n", pathDirs[4]); */
  for(int i = 0; i < MAXPATHDIRS -1; i++) {
    if(pathDirs[i] != NULL) {
      findExecutable(words[0], pathDirs[i]); 
    }
  }
}

int main() {
  char input[MAXWORDS];
  char * words[MAXWORDS];
  while(1) {
    printf("#");
    scanf("%[^\n]%*c", input);
    splitIntoWords(input, words);
    handleUserInput(words);
  }
}
