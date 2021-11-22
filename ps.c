#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

#define PROCDIR "/proc"

int isDirectory(const char * path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0) {
    return 0;
  }
  return S_ISDIR(statbuf.st_mode);
}

int isLink(const char * path) {
  struct stat statbuf;
  if (lstat(path, &statbuf) != 0) {
    return 0;
  }
  return S_ISLNK(statbuf.st_mode);
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

void checkTtyInProcessFd(DIR * directory_reader, char* absolute_path) {
  struct dirent *directory;
  char * filePath;
  while ((directory= readdir(directory_reader)) != NULL) {
    filePath = concatFolder(absolute_path, directory->d_name); 
    printf("is file %s\n", filePath);
    if(isLink(filePath)) {
      printf("is Link %s\n", filePath);
    }
  }
}

void handleProcessDir(char * absolute_path) {
  char * fdPath = concatFolder(absolute_path, "fd");
  //Check if has permission to oper folder
  if(access(fdPath, R_OK) != 0) {
    return;
  }
  /* printf("%s\n", fdPaths); */
  DIR *directory_reader = opendir(fdPath); 
  if (directory_reader == NULL) {
    printf("Could not open current directory" );
    exit(0);
  }
  checkTtyInProcessFd(directory_reader, fdPath);
}

void readProcessDir(struct dirent * directory) {
  char * dir = PROCDIR;
  if(IsDigitsOnly(directory->d_name) ) {
    char * absolute_path = concatFolder(PROCDIR, directory->d_name);
    if(isDirectory(absolute_path)) {
      handleProcessDir(absolute_path);
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
    while ((directory= readdir(directory_reader)) != NULL) {
      readProcessDir(directory);
    }
    closedir(directory_reader);     
}
