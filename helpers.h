/*

Arafat Rahaman
shell Project
helpers.h

*/

#define LINE_MAX 4096 

#ifndef MAKEFILE_SHELL_H
#define MAKEFILE_SHELL_H

int printWD();
char ** parse(char*line, char*delim);
int find_special (char*args[], char * special);
char * check_for_path (char *name);
char * helpFile_check_for_path (char *name);
void printHelp();
char * lastArg(char **array);
int lastIndex(char **array);
void waitBi();

#endif