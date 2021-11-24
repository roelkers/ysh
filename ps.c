#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

#define PROCDIR "/proc"
#define WORDBUFLENGTH 100
#define MAXWORDS 50

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

int isDirectory(const char * path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0) {
    return 0;
  }
  return S_ISDIR(statbuf.st_mode);
}

bool IsDigitsOnly(char * c) {
    while(*c != '\0')
    {
        if (*c < '0' || *c > '9')
            return false;
        c++;
    }
    return true;
}

char * concatFolder(char * path, char * folder) {
  char * absolute_path = malloc(sizeof(path) + sizeof(folder)+1);
  strcat(absolute_path, path);
  strcat(absolute_path, "/"), 
  strcat(absolute_path, folder);
  return absolute_path;
}

char * getTime(char * absolute_path) {
  struct stat statbuf;
  if (stat(absolute_path, &statbuf) != 0) {
    return 0;
  }
  __time_t uptime = statbuf.st_mtime - time(NULL);
  struct tm time = *localtime(&uptime);
  char * timeStr; 
  asprintf(&timeStr, "%d:%d:%d", time.tm_hour, time.tm_min, time.tm_sec);
  return timeStr;
}

char * getCmd(char * absolute_path) {
  char * cmdPath = concatFolder(absolute_path, "status");
  char * cmdColumns[3];
  char * cmd = NULL;
  size_t n;
  //Check if has permission to oper folder
  FILE *fp = fopen(cmdPath, "r");
  if(fp == NULL) {
    perror("fopen");
    return "";
  }
  getline(&cmd, &n, fp);
  splitStr(cmd, cmdColumns, 3, '\t');
  fclose(fp);
  return cmdColumns[1];
}

void handleProcessDir(char * absolutePath, char * processId) {
  char * cmd = getCmd(absolutePath);
  char * time = getTime(absolutePath);
  printf("%s\t%s\t%s", processId, time, cmd);
}

void readProcessDir(struct dirent * directory) {
  char * dir = PROCDIR;
  if(IsDigitsOnly(directory->d_name) ) {
    char * absolute_path = concatFolder(PROCDIR, directory->d_name);
    if(isDirectory(absolute_path)) {
      handleProcessDir(absolute_path, directory->d_name);
    }
  }
}

int main() {
    char * dir = PROCDIR;
    struct dirent *directory;
  
    DIR *directory_reader = opendir(dir); 
    if (directory_reader == NULL) {
        printf("Could not open current directory" ); 
        exit(0);
    }
    printf("PID\tTIME\tCMD\n");
    while ((directory= readdir(directory_reader)) != NULL) {
      readProcessDir(directory);
    }
    closedir(directory_reader);     
}
