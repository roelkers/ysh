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

bool doesFileExist(const char * path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0) {
    return false;
  }
  return true;
}

struct executableWithArgs {
  char * executable;
  char * args[MAXWORDS]; 
  char * binary_path;
  struct executableWithArgs * next;
};

int outFds[2];
int fileFd;
int saved_stdout;

void executeCommand (struct executableWithArgs);
void freeExecutable(struct executableWithArgs *);

void splitStr(char* input, char * words[MAXWORDS], int wordsSize, char separator) {
  for(int j = 0; j < wordsSize -1; j++) {
    *words[j] = '\0';
  }
  int i = 0;
  char c;
  char * p;
  p = words[i];
  while((c = *input) != '\0') {
    if(c == separator){
      *p = '\0';
      i++;
      p = words[i];
    } else {
      *p = c;
      p++;
    }
    input++;
  }
  *p = '\0';
}

void executeCommandChain (struct executableWithArgs * command) {
  struct executableWithArgs * currentCommand;
  currentCommand = command;

  while(currentCommand != NULL) {
    /* printf("executing command %s\n", command->executable); */
    /* printf("binary path %s\n", command->binary_path); */
    if(command->binary_path != NULL) {
      executeCommand(*currentCommand);
    }
    currentCommand = currentCommand->next;
  }
}

void executeCommand (struct executableWithArgs command) {
  int pid = 0;
 // Environment Variable Array
  char *const env[] = {"", "", NULL};
  int returnStatus = 0;
  /* printf("command.binary_path: %s\n", command.binary_path); */
  /* printf("executableWithArgs.args[0]:%s\n",command.args[0]); */
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
    execve(command.binary_path, command.args, env);
    perror("execve");
  } 
  waitpid(pid, &returnStatus, 0);
  if(returnStatus != 0) {
    printf("%s exited with error code %i\n", command.binary_path, returnStatus);
  }
}

void findExecutable (struct executableWithArgs* command, char * dir) {
    struct dirent *directory;
    DIR *directory_reader = opendir(dir); 
  
    if (directory_reader == NULL) {
        perror("opendir");
        exit(0);
    }
    while ((directory= readdir(directory_reader)) != NULL) {
      if(strcmp(directory->d_name, command->executable) == 0) {
        /* printf("dir: %s\n", dir); */
        char * absolute_exec_path = malloc(WORDBUFLENGTH);
        /* printf("absolute_exec_path: %s\n", absolute_exec_path); */
        strcpy(absolute_exec_path, dir);
        strcat(absolute_exec_path, "/");
        strcat(absolute_exec_path, directory->d_name);
        /* printf("absolute_exec_path: %s\n", absolute_exec_path); */
        command->binary_path = absolute_exec_path;
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

void parsePathEntries (char * pathString, char * pathDirs[MAXPATHDIRS]) {
  splitStr(pathString, pathDirs, MAXPATHDIRS, ':');
}

void findExecutablesInPath(struct executableWithArgs *command, char * pathDirs [MAXPATHDIRS]) {
  struct executableWithArgs * currentCommand;
  currentCommand = command;

  while(currentCommand != NULL) {
    /* printf("looking for cmd: %s\n",currentCommand->executable); */
    for(int i = 0; i < MAXPATHDIRS; i++) {
      if(*pathDirs[i] != '\0' && currentCommand->executable != NULL) {
        findExecutable(currentCommand, pathDirs[i]); 
      }
    }
    currentCommand = currentCommand->next;
  }
  /* for(int i = 0; i < MAXPATHDIRS; i++) { */
  /*   free(pathDirs[i]); */
  /* } */
}

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

struct executableWithArgs * allocExecutable(void) {
  return (struct executableWithArgs*) malloc(sizeof(struct executableWithArgs));
}

void getExecutables(char * words[MAXWORDS], struct executableWithArgs *command) {
  struct executableWithArgs * currentCommand = command;
  int k  = 1; // arg index
  bool inputRedirectionToggled = false;
  bool outputRedirectionToggled = false;
  currentCommand->executable = strdup(words[0]);
  currentCommand->args[0] = strdup(words[0]);
  currentCommand->next = NULL; 
  for(int i = 1; i < MAXWORDS-1; i++) {
    if(*words[i] != '\0') {
      switch(*words[i]) {
        case('<'):
          if(inputRedirectionToggled) {
            printf("Error: Double < redirection\n");
            exit(1);
          }
          inputRedirectionToggled = true;
          if(words[i+1] != NULL) {
            command->args[k] = words[i+1];
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
          k++;
          break;
        case('|'):
          currentCommand->args[k+1] = NULL; //finish args array
          i++; // skip word
          printf("executable in word %s\n", words[i]);
          currentCommand->next = allocExecutable(); 
          currentCommand = currentCommand->next;
          currentCommand->args[0] = strdup(words[i]);
          currentCommand->executable = strdup(words[i]); //write next executable 
          currentCommand->next = NULL; 
          k = 1; // reset arg index
          break;
        default: 
          currentCommand->args[k] = strdup(words[i]);
          k++;
          break;
      }
    }
    else {
      currentCommand->args[k] = NULL;
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

void freeExecutable(struct executableWithArgs * command) {
   if(command->next != NULL) {
      freeExecutable(command);
   }
   free(command->binary_path); 
   free(command->executable);
   int k = 0;
   while(command->args[k] != NULL) {
     free(command->args[k]);
     k++;
   }
}

void handleUserInput (char * words[MAXWORDS], char * pathDirs [MAXPATHDIRS]) {
  struct executableWithArgs command;
  getExecutables(words, &command);
  
  findExecutablesInPath(&command, pathDirs);
  executeCommandChain(&command);
  //freeExecutable(&command);
  cleanup();
}

int main() {
  char input[MAXWORDS];
  char * words[MAXWORDS];
  for(int i = 0; i < MAXWORDS; i++) {
    words[i] = malloc(WORDBUFLENGTH);
  }
  char * pathDirs [MAXPATHDIRS]; 
  for(int i = 0; i < MAXPATHDIRS; i++) {
    pathDirs[i] = malloc(WORDBUFLENGTH);
  }
  for(int i = 0; i < MAXPATHDIRS; i++) {
    *pathDirs[i] = '\0';
  }
  char * path = loadPathFromEnvironment();
  parsePathEntries(path, pathDirs);
  saved_stdout = dup(STDOUT_FILENO);
  while(1) {
    printf("#");
    scanf("%[^\n]%*c", input);
    splitStr(input, words, MAXWORDS, ' ');
    handleUserInput(words, pathDirs);
  }
}
