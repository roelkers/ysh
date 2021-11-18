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
  char *const args[] = {binary_path, "-c", "echo \"Visit $HOSTNAME:$PORT from your browser.\"", NULL};
 // Environment Variable Array
  char *const env[] = {"", "", NULL};
 
  execve(binary_path, args, env);
}

void findExecutable (char * command) {
    char dir[27] = "/home/rufus/Dev/yawnshell/";
    struct dirent *directory;
    DIR *directory_reader = opendir("."); 
  
    if (directory_reader == NULL) {
        printf("Could not open current directory" ); 
        exit(0);
    }
    while ((directory= readdir(directory_reader)) != NULL) {
      if(strcmp(directory->d_name, command) == 0) {
        char * absolute_exec_path = malloc(sizeof(dir) + sizeof(directory->d_name));
        absolute_exec_path = strcat(dir, directory->d_name);
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
  /* findExecutable(words[0]); */
  //findExecutable("fahrenheit");
  char * pathDirs [MAXPATHDIRS]; 
  char * path = loadPathFromEnvironment();
  parsePathEntries(path, pathDirs);
  for(int i = 0; i < 4; i++) {
    printf("path : %s\n", pathDirs[i]);
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
