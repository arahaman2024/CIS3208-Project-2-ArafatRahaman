/*

Arafat Rahaman
shell Project
helpers.c 

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "helpers.h"

// Arguments: N/A
// Return: 0 upon success, if it fails it just exits the program.
// Functionality: uses getcwd to get the current work directory, and prints it
int printWD() {
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\n\t$ Current working directory: %s\n\n", cwd);
    }else{
        fprintf(stderr, "Could not access CWD. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

// Arguments: input to the shell, space character (used to seperate individual strings)
// Return: point to a point to an array of strings
// Functionality: mallocs memory to store the input recieved to the shell and pareses it. The returned array can then be used to to execute commands in the shell
char ** parse(char*line, char*delim){
  char**array=malloc(sizeof(char*));
  *array=NULL;
  int n = 0;
  char*buf = strtok(line,delim);
  if (buf == NULL){
    free(array);
    array=NULL;
    return array;
  }

  while(buf != NULL) {
    char**temp = realloc(array,(n+2)*sizeof(char*));
    if(temp==NULL){
      free(array);
      array=NULL;
      return array;
    }
    array = temp;
    temp[n++]=buf;
    temp[n]=NULL;
    buf = strtok(NULL,delim);
  }
  return array;
}

// this keeps going until a NULL
// so if you set the & to be null from the start it will read the line as wc < in > out NULL NULL
// Arguments: takes in the an array of strings, and a special character
// Return: returns the index of the first instance of the passed special char, -1 if no special char was found
// Functionality: Goes throught the passed in string and looks for special char
int find_special (char*args[], char * special){
	int i = 0;
	while(args[i]!=NULL){
		if(strcmp(args[i],special)==0){
			return i;
		}
		i++;
	}
	return -1;
}

// write a c function that will give you the full path of a given command such as wc by checking if an executable file like that exists returns NULL if it doesnt

// Arguments: pointer to a string (something like ls, wc, executableProgram)
// Return: NULL if an executable wasnt found, if found the full path name to the executable
// Functionality: uses the PATH enviornmental varaible, and looks for the passed in strings full path, and if its an executable. Checks each PATH varaibel files.
char * check_for_path (char *name) {
    char *path = getenv("PATH");
    char *dupPath = strdup(path);
    char *directories = strtok(dupPath, ":");

    while (directories != NULL) {
        char returnPath[4096] = "";
        strcpy(returnPath, directories);
        strcat(returnPath, "/");
        strcat(returnPath, name);
        if (access(returnPath, X_OK) == 0) {
            free(dupPath);
            // printf("path = %s\n", returnPath);
            char *result = strdup(returnPath);
            return result;
        }
        directories = strtok(NULL, ":");
    }
    // printf("file not found.");
    free(dupPath);
    return NULL;
}


// Arguments: pointer to a string (something like ls, wc, executableProgram)
// Return: return NULL if the given file wasn't a text file. Returns the exact path variable to the input text file
// Functionality: ONLY used for the printHelp() function, and is used to find the path the help.txt file. Wont work at all if the current work directory isnt the same the program's work directory. Like if you changed using cd in the shell.
char * helpFile_check_for_path (char *name) {
    char *path = getenv("PATH");
    char *dupPath = strdup(path);
    char *directories = strtok(dupPath, ":");

    while (directories != NULL) {
        char returnPath[4096] = "";
        strcpy(returnPath, directories);
        strcat(returnPath, "/");
        strcat(returnPath, name);
        if (access(returnPath, F_OK) == 0) {
            free(dupPath);
            char *result = strdup(returnPath);
            return result;
        }
        directories = strtok(NULL, ":");
    }
    free(dupPath);
    return NULL;
}

// Arguments: N/A
// Return: N/A
// Functionality: void function with no args that finds the help.txt file using the enviornment PATH varaiable. Finds the file and prints the contents which contain a manual of some sort to the shell.
void printHelp() {
    char helpPath[4096] = "";
    char *path = helpFile_check_for_path("help.txt");
    if (path != NULL) {
        strcpy(helpPath, path);
        // printf("Path found. Path is [%s].\n", helpPath);
        FILE *helpFile = fopen(helpPath, "r");
        if (helpFile == NULL) {
            printf("Error opening file.\n");
        } else {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), helpFile) != NULL) {
                printf("%s", buffer);
            }
            fclose(helpFile);
            puts("");
        }
        
        free(path);
    } else {
        printf("Help text file could not be found.\n");
    }
}


// Arguments: pointer to an array of strings
// Return: returns pointer to the last thing before the NULL in the array
// Functionality: returns pointer to the last thing before the NULL in the array
char * lastArg(char **array) {
    char **ptr = array;
    while (*ptr != NULL) {
        ptr++;
    }
    ptr--;
    return *ptr;
}

// Arguments: pointer to an array of strings
// Return: returns the index of the last thing in the array before the NULL
// Functionality: returns the index of the last thing in the array before the NULL
int lastIndex(char **array) {
    char **ptr = array;
    int count = 0;
    while (*ptr != NULL) {
        ptr++;
        count = count + 1;
    }
    count--;
    return count;
}


// Arguments: N/A
// Return: N/A
// Functionality: used in the wait built-in where it waits for all the children process to finish.
void waitBi() {
    while (waitpid(-1,NULL, 0) > 0) {}
}