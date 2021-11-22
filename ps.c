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

bool IsDigitsOnly(char * c) {
    while(*c != '\0')
    {
        if (*c < '0' || *c > '9')
            return false;
        c++;
    }
    return true;
}

int readProcessDir(struct dirent * directory) {
  char * dir = PROCDIR;
  if(IsDigitsOnly(directory->d_name)) {
    printf("found: %s\n", directory->d_name);
    char * absolute_path = malloc(sizeof(dir) + sizeof(directory->d_name));
    strcat(absolute_path, PROCDIR);
    strcat(absolute_path, "/"), 
    strcat(absolute_path, directory->d_name);
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
