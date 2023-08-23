/*

Arafat Rahaman
shell Project
shell script

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


int main(int argc, char *argv[]) {

    char *input = NULL;
    size_t inputSize = 0;

    while (1) {

        printf("~AShell: ");
        getline(&input, &inputSize, stdin);
        char * inputDup = strdup(input);
        char ** array = parse(inputDup," \n");

        int bg = 0;

        if (!strcmp(lastArg(array), "&")) {
            int lastI = lastIndex(array);
            array[lastI] = NULL;
            bg = 1;
        }

        
        // BUILT-INS
        if (  (strcmp(array[0], "exit") == 0) && (array[1] == NULL) ) {
            break;
        }else if (  (strcmp(array[0], "help") == 0) && (array[1] == NULL)  ) {
            printHelp();
        }else if (  (strcmp(array[0], "pwd") == 0) && (array[1] == NULL)  ) {
            printWD();
        }else if (  (strcmp(array[0], "cd") == 0) && (array[1] != NULL) && (array[2] == NULL) ) {    // need to see what cd without any args does
            if (chdir(array[1]) != 0) {
                fprintf(stderr, "Could not change the directory to <%s>. Exiting...\n", array[1]);
                continue;
            }else{
                printWD();
            }
        }else if (  (strcmp(array[0], "cd") == 0) && (array[1] == NULL)  ) {
            printf("Missing destination argument after cd command.\n");
            continue;
        }else if (  (strcmp(array[0], "wait") == 0)  ) { 
            waitBi();
        


        
        // CASE 1: NO SPECIAL CHARS
        }else if ( (find_special(array, "<") == -1) && (find_special (array, ">") == -1)  && (find_special (array, "|") == -1) ){
            
            if (*array[0] != '/' || *array[0] != '.') {
                char *executable = check_for_path(array[0]);
                if (executable != NULL) {
                    array[0] = executable;
                }else{
                    printf("Couldn't find executable command.\n");
                    continue;
                }
            }

            int forkReturn;
            int status;

            if ((forkReturn = fork()) == -1) {
                fprintf(stderr, "forkfailure: %s\n", strerror(errno));
                continue;
            }else if (forkReturn == 0) {

                if ((execv(array[0], array)) < 0) {
                    fprintf(stderr, "exec error: %s\n", strerror(errno));
                    continue;
                }
            }else{
                if (bg == 0) {
                    if ((waitpid(forkReturn, &status, 0)) == -1){
                        printf("Waitpid() failed.\n");
                        continue;
                    };
                }else{
                    if ((waitpid(forkReturn, &status, WNOHANG)) == -1){
                        printf("Waitpid() failed.\n");
                        continue;
                    }
                }
            };
            

        // IF NO | WAS GIVEN BUT FILE REDIRECTION
        }else if (  (find_special (array, "|") == -1)  &&   (find_special (array, ">") != -1 || find_special (array, "<") != -1)  ) {  

            // CASE: OUTPUT FILE ----------------------------------------------------------------------------------------------------------------------------------------------------------------

            if ( (find_special (array, "<") == -1) && (find_special (array, ">") != -1)) {       // does not have <, aka just has outputs to a file, printarg > textfile.txt

                if (*array[0] != '/' || *array[0] != '.') {
                    char *executable = check_for_path(array[0]);
                    if (executable != NULL) {
                        array[0] = executable;
                    }else{
                        printf("Couldn't find executable command.\n");
                        continue;
                    }
                }

                int specialIndex = find_special(array, ">");
                int count = 0;
                while(array[count] != NULL) {
                    count++;
                }
                
                int indexOfOutFile = count - 1;

                array[specialIndex] = NULL;

                if ( (indexOfOutFile - specialIndex) != 1) {
                    printf("Outfile has to be the last to be last and right before output file.\n");
                    continue;
                }

                int forkReturn;
                int status;

                if ((forkReturn = fork()) == -1) {
                    fprintf(stderr, "forkfailure: %s\n", strerror(errno));
                    continue;
                }else if (forkReturn == 0) {

                    int Ofd = open(array[indexOfOutFile], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

                    if (Ofd == -1) {
                        printf("Couldn't open/create the output file.\n");
                        continue;
                    }
                    
                    if ((dup2(Ofd, STDOUT_FILENO)) == -1) {
                        printf("Couldn't dup2().\n");
                        continue;
                    }

                    if ((execv(array[0], array)) < 0) {
                        fprintf(stderr, "exec error: %s\n", strerror(errno));
                        continue;
                    }
                }else{
                    if (bg == 0) {
                        if ((waitpid(forkReturn, &status, 0)) == -1){
                            printf("Waitpid() failed.\n");
                            continue;
                        };
                    }else{
                        if ((waitpid(forkReturn, &status, WNOHANG)) == -1){
                            printf("Waitpid() failed.\n");
                            continue;
                        }
                    }
                    // }else{
                    //     if ((waitpid(forkReturn, &status, WNOWAIT)) == -1){
                    //         printf("Waitpid() failed.\n");
                    //         continue;
                    //     };
                    // }
                };



            // CASE INPUT FILE ----------------------------------------------------------------------------------------------------------------------------------------------------------------
            }else if (  (find_special (array, ">") == -1) && (find_special (array, "<") != -1)  ) {
                
                // dealing with just the first arg
                if (*array[0] != '/' || *array[0] != '.') {
                    char *executable = check_for_path(array[0]);
                    if (executable != NULL) {
                        array[0] = executable;
                    }else{
                        printf("Couldn't find executable command.\n");
                        continue;
                    }
                }
                
                // find where the < is
                int specialIndex = find_special(array, "<");
                int count = 0;
                
                // find the amount of args
                while(array[count] != NULL) {
                    count++;
                }
                
                int indexOfInFile = count - 1;
                // printf("array[indexOfInFile] = %s\n", array[indexOfInFile]);

                array[specialIndex] = NULL;     // wc < inputFile becomes wc

                // simply exits if it isnt "<", "inputFile" right next to each other
                if ( (indexOfInFile - specialIndex) != 1) {
                    printf("Outfile has to be the last to be last and right before output file.\n");
                    continue;
                }

                int forkReturn;
                int status;

                if ((forkReturn = fork()) == -1) {
                    fprintf(stderr, "forkfailure: %s\n", strerror(errno));
                    continue;
                }else if (forkReturn == 0) {
                    
                    int fdOpen = open(array[indexOfInFile], O_RDONLY);

                    if (fdOpen == -1) {
                        printf("Couldn't open the input file.\n");
                        continue;
                    }

                    if ((dup2(fdOpen, STDIN_FILENO)) == -1) {
                        printf("Couldn't dup2().\n");
                        continue;
                    }
                    
                    if ((execv(array[0], array)) < 0) {
                        fprintf(stderr, "exec error: %s\n", strerror(errno));
                        continue;
                    }

                }else{
                    if (bg == 0) {
                        if ((waitpid(forkReturn, &status, 0)) == -1) {
                            printf("Waitpid() failed.\n");
                            continue;
                        };
                    }else{
                        if ((waitpid(forkReturn, &status, WNOHANG)) == -1){
                            printf("Waitpid() failed.\n");
                            continue;
                        }
                    }
                };

            

            // CASE: BOTH INPUT AND OUTPUT ----------------------------------------------------------------------------------------------------------------------------------------------------------------
            } else if ( (find_special (array, ">") != -1) && (find_special (array, "<") != -1)  ) {

                // we know at this point that it has both "<" and ">"
                // need to make sure we have this exact format
                // executable args < I > O    ||    executable args > O < I
                // we can check formating as well
                // total
                
                int count = 0;
                while(array[count] != NULL) {
                    count++;
                }

                int inChar = (find_special (array, "<"));
                int outChar = (find_special (array, ">"));

                // files names always come after the < or >
                char *inFileName = array[inChar+1];
                char *outFileName = array[outChar+1];


                int firstRedirect = 0;

                if (inChar > outChar) {
                    firstRedirect = outChar;
                }else{
                    firstRedirect = inChar;
                }

                // we can now do some error checking
                // I am assuming that from the first redirection to the last arguemnt there will be a total of 4 args.
                //      executable arg1 arg2 ... < I > O, the the last 4 args will always be two redirections, followed by their files
                //      if (indexOfLastArg - firstRedirect != 3) {not valid} OR
                //      if (absoluteValue(inChar - outChar) != 2) {not valid}

                int indexOfLastArg = count - 1;


                if ((indexOfLastArg - firstRedirect != 3) || (abs(inChar-outChar) != 2)) {
                    printf("Invalid formatting of file redirection.\n");
                    continue;
                }

                // if (CANNOT OPEN INFILE, INVALID)

                FILE *argFile;
                argFile = fopen(inFileName, "r");
                if (argFile == NULL) {
                    printf("Couldn't open the input file.\n");
                    continue;
                }
                fclose(argFile);

                // printf("COULD open inFile.\n");

                // not we need to set the file find of special to NULL
                // well we know at this point that > or < will always be at index 1 or 3, and 1 will always be the lesser of the 2
                // need it so [executable, >, out, <, in, NULL] -> [executable, NULL, out, <, in, NULL]
                // we also have the input fileName saved and output fileName saved

                array[firstRedirect] = NULL;
                // lets now deal with weather the executable is fullPathName or just a command like wc

                if (*array[0] != '/' || *array[0] != '.') {
                    char *executable = check_for_path(array[0]);
                    if (executable != NULL) {
                        array[0] = executable;
                    }else{
                        printf("Couldn't find executable command.\n");
                        continue;
                    }
                }

                int forkReturn;
                int status;

                if ((forkReturn = fork()) == -1) {
                    fprintf(stderr, "forkfailure: %s\n", strerror(errno));
                    continue;
                }else if (forkReturn == 0) {
                    int iP = open(inFileName, O_RDONLY);
                    if (iP == -1) {
                        printf("Couldn't open the inputFile.\n");
                        continue;
                    }
                    int oP = open(outFileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                    if (oP == -1) {
                        printf("Couldn't open the outputFile.\n");
                        continue;
                    }
                    if ((dup2(oP, STDOUT_FILENO)) == -1) {
                        printf("Couldn't dup2().\n");
                        continue;
                    }
                    if ((dup2(iP, STDIN_FILENO)) == -1) {
                        printf("Couldn't dup2().\n");
                        continue;
                    }

                    if ((execv(array[0], array)) < 0) {
                        fprintf(stderr, "exec error: %s\n", strerror(errno));
                        continue;
                    }
                }else{
                    if (bg == 0) {
                        if ((waitpid(forkReturn, &status, 0)) == -1){
                            printf("Waitpid() failed.\n");
                            continue;
                        };
                    }else{
                        if ((waitpid(forkReturn, &status, WNOHANG)) == -1){
                            printf("Waitpid() failed.\n");
                            continue;
                        }
                    }
                }
                
                puts("");

            }

        // CASE: PIPES ----------------------------------------------------------------------------------------------------------------------------------------------------------------

        }else if (find_special (array, "|") != -1) {
            int fd[512][2];
            // int specialChar = find_special(input);

            int numProcs = 1;
            for ( int i = 0; array[i] != NULL; i++) {
                if (!strcmp(array[i], "|")) {
                    numProcs++;
                }
            }
            // printf("numProcs = %d\n", numProcs);

            for (int i = 0; i < numProcs-1; i++) {
                if (pipe(fd[i]) < 0) {
                    printf("Pipeing failed.");
                    continue;
                }
            }

            int forkReturn;
            int status;

            int pipeIndex = find_special(array, "|");
            int total = pipeIndex; 
            int comName = 0;

            array[pipeIndex] = NULL;
            int forkReturns[1024];

            // store all pids to be used in waitpid() call
            // int forkReturns[1024];
            for (int i = 0; i < numProcs; i++) {
                

                if ((forkReturn = fork()) < 0) {
                    printf("Fork didn't work.\n");
                    exit(1);

                }else if( forkReturn == 0) {
                    
                    // printf("1. I am child number[%d].\n", i);

                    // if its wc, it will get turned into /bin/wc
                    if (*array[comName] != '/' || *array[comName] != '.') {
                        char *executable = check_for_path(array[comName]);
                        if (executable != NULL) {
                            array[comName] = executable;
                        }else{
                            printf("Couldn't find executable exiting program...\n");
                            exit(1);
                        }
                    }

                    // first command doesn't read from pipe
                    if (i != 0) {
                        if (dup2(fd[i-1][0], STDIN_FILENO) < 0) {
                            printf("dup2 didn't work for read end of a pipe.\n");
                            exit(1);
                        }
                    }

                    // last command doesnt read from pipe
                    if (i != numProcs - 1) {
                        if (dup2(fd[i][1], STDOUT_FILENO) < 0) {
                            printf("dup2 didn't work for write end of a pipe.\n");
                            exit(1);
                        }
                    }

                    for (int i = 0; i < numProcs - 1; i++) {
                        close(fd[i][0]);
                        close(fd[i][1]);
                    }


                    if (execv(array[comName], &array[comName]) < 0) {
                        perror("execv");
                        exit(1);
                    }

                }else{
                    forkReturns[i] = forkReturn;
                    comName = pipeIndex + 1;
                    pipeIndex = total + find_special(&array[comName], "|") + 1;
                    total = pipeIndex;
                    if (pipeIndex != -1) {
                        array[pipeIndex] = NULL;
                    }
                }
            }

            for (int i = 0; i < numProcs - 1; i++) {
                close(fd[i][0]);
                close(fd[i][1]);
            }

            int i;
            // all procs besides last
            for (i = 0; i < numProcs-1; i++) {
                if (waitpid(forkReturns[i], &status, 0) < 0) {
                    perror("waitpid");
                    exit(1);
                }
            }
            
            if (bg == 0) {
                if (waitpid(forkReturns[i], &status, 0) == -1) {
                    perror("waitpid");
                    exit(1);
                }
            }else{
                if ((waitpid(forkReturn, &status, WNOHANG)) == -1){
                    printf("Waitpid() failed.\n");
                    continue;
                }
            }
        }
        else{
            printf("You have entered an unknown command. Please try again.\n");
        }

        int status;
        waitpid(-1, &status, WNOHANG);
    }

    free(input);
    return 0;
}