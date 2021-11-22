#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

#define WORDBUFLENGTH 100
#define MAXWORDS 50
#define MAXPATHDIRS 50
#define CUSTOMPATH "./bin"

typedef struct executableWithArgs {
  char * executable;
  char * args[MAXWORDS]; 
} executableWithArgs;

void splitStr(char* input, char * words[MAXWORDS], int wordsSize, char separator) {
  for(int j = 0; j < wordsSize -1; j++) {
    words[j] = NULL;
  }
  int i = 0;
  char c;
  char * p;
  words[i] = malloc(WORDBUFLENGTH);
  p = words[i];
  while((c = *input) != '\0') {
    if(c == separator){
      *p = '\0';
      i++;
      words[i] = malloc(WORDBUFLENGTH);
      p = words[i];
    } else {
      *p = c;
      p++;
    }
    input++;
  }
  p = '\0';
}


void executeCommand (char * binary_path, executableWithArgs command) {
  int pid = 0;
  // Argument Array
  /* char *const args[] = {"", NULL}; */
 // Environment Variable Array
  char *const env[] = {"", "", NULL};
  int returnStatus = 0;

  pid = fork();
  if(pid < 0) {
    perror("Unable to create child process");
    exit(1);
  }
  //only executes in child process
  if(pid == 0) { 
    execve(binary_path, command.args, env);
    perror("execve");
  } 
  waitpid(pid, &returnStatus, 0);
  if(returnStatus != 0) {
    printf("%s exited with error code %i\n", binary_path, returnStatus);
  }
}

bool findExecutable (executableWithArgs command, char * dir) {
    struct dirent *directory;
    DIR *directory_reader = opendir(dir); 
  
    if (directory_reader == NULL) {
        printf("Could not open current directory" ); 
        exit(0);
    }
    while ((directory= readdir(directory_reader)) != NULL) {
      if(strcmp(directory->d_name, command.executable) == 0) {
        char * absolute_exec_path = malloc(sizeof(dir) + sizeof(directory->d_name));
        strcat(absolute_exec_path, dir);
        strcat(absolute_exec_path, "/");
        strcat(absolute_exec_path, directory->d_name);
        executeCommand(absolute_exec_path, command);
        return true;
      }
    }
    closedir(directory_reader);     
}

char * loadPathFromEnvironment() {
  char* pathStr = getenv("PATH");
  char * newPath = malloc(strlen(pathStr) + strlen(CUSTOMPATH)+1); 
  strcat(newPath, CUSTOMPATH); 
  strcat(newPath, ":"); 
  strcat(newPath, pathStr); 
  return newPath;
}

void parsePathEntries (char * pathString, char ** pathDirs) {
  splitStr(pathString, pathDirs, MAXPATHDIRS, ':');
}

void findExecutableInPath(executableWithArgs executables[MAXWORDS]) {
  char * pathDirs [MAXPATHDIRS]; 
  char * path = loadPathFromEnvironment();
  parsePathEntries(path, pathDirs);
  for(int i = 0; i < MAXPATHDIRS -1; i++) {
    if(pathDirs[i] != NULL) {
      bool found = findExecutable(executables[0], pathDirs[i]); 
      if(found) {
        return;
      }
    }
  }
}

void getExecutables(char * words[MAXWORDS], executableWithArgs executables[MAXWORDS]) {
  executables[0].executable = words[0];
  for(int i = 0; i < MAXWORDS-1; i++) {
    if(words[i] != NULL) {
      executables[0].args[i] = words[i];
    }
  }
} 

void handleUserInput (char * words[MAXWORDS]) {
  struct executableWithArgs executables[MAXWORDS];
  getExecutables(words, executables);
  findExecutableInPath(executables);
}

int main() {
  char input[MAXWORDS];
  char * words[MAXWORDS];
  while(1) {
    printf("#");
    scanf("%[^\n]%*c", input);
    splitStr(input, words, MAXWORDS, ' ');
    handleUserInput(words);
  }
}
