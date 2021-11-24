#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


#define WORDBUFLENGTH 100
#define MAXWORDS 50
#define MAXPATHDIRS 50
#define CUSTOMPATH "./bin"

int outFds[2];
int fileFd;
int saved_stdout;

bool doesFileExist(const char * path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0) {
    return false;
  }
  return true;
}

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

  /* printf("executableWithArgs.args[1]:%s\n",command.args[0]); */
  /* printf("executableWithArgs.args[1]:%s\n",command.args[1]); */
  /* printf("executableWithArgs.args[2]:%s\n",command.args[2]); */
  /* printf("executableWithArgs.args[3]:%s\n",command.args[3]); */
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

/* pipe(outFds); */
/* close(STDOUT_FILENO); */
/* dup2(outFds[0], STDOUT_FILENO); */

void redirectOutputToFile(char * filePath){
  if(doesFileExist(filePath)) {
    printf("%s: file exists.\n", filePath);
    exit(1);
  }
  fileFd = open(filePath, O_CREAT | O_WRONLY); 
  if(fileFd < 0) {
    perror("fopen");
    exit(1);
  }
  int dup2status = dup2(fileFd, STDOUT_FILENO);
  if(dup2status < 0) {
    perror("dup2");
    exit(1);
  }
}

void getExecutables(char * words[MAXWORDS], executableWithArgs executables[MAXWORDS]) {
  bool inputRedirectionToggled = false;
  bool outputRedirectionToggled = false;
  executables[0].executable = words[0];
  for(int i = 0; i < MAXWORDS-1; i++) {
    if(words[i] != NULL) {
      switch(*words[i]) {
        case('<'):
          if(inputRedirectionToggled) {
            printf("Error: Double < redirection\n");
            exit(1);
          }
          inputRedirectionToggled = true;
          if(words[i+1] != NULL) {
            executables[0].args[i] = words[i+1];
          }
          return;
        case('>'):
          if(outputRedirectionToggled) {
            printf("Error: Double > redirection\n");
            exit(1);
          }
          outputRedirectionToggled = true;
          if(words[i+1] == NULL) {
            printf("No file for redirecting output given.");
            exit(1);
          }
          redirectOutputToFile(words[i+1]);
          break;
        default: 
          executables[0].args[i] = words[i];
          break;
      }
    }
    else {
      executables[0].args[i] = NULL;
    } 
  }
} 

void cleanup () {
  if(fileFd) {
    //set permissions
    int fchmodstatus = fchmod(fileFd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if(fchmodstatus != 0) {
      perror("fchmod");
    }
    //close file
    close(fileFd);
    //reattach stdout
    int dup2status = dup2(saved_stdout, STDOUT_FILENO);
    if(dup2status < 0) {
      perror("error restoring stdout.");
      exit(1);
    }
  }
}

void handleUserInput (char * words[MAXWORDS]) {
  struct executableWithArgs executables[MAXWORDS];
  getExecutables(words, executables);
  findExecutableInPath(executables);
  cleanup();
}

int main() {
  char input[MAXWORDS];
  char * words[MAXWORDS];
  saved_stdout = dup(STDOUT_FILENO);
  while(1) {
    printf("#");
    scanf("%[^\n]%*c", input);
    splitStr(input, words, MAXWORDS, ' ');
    handleUserInput(words);
  }
}
