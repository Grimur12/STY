#include "execute.h"
#include <stdio.h>
// For fork()
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
// for waitpid()
#include <sys/wait.h>

#define ERROR_CODE 127

int execute(char *file_path, char *argv[])
{
    if (file_path == NULL) {
        return ERROR_CODE;
    }

    if (argv == NULL) {
        argv = (char **)malloc(2 * sizeof(char *));
        argv[0] = file_path;
        argv[1] = NULL;
    }

    pid_t pid = fork();
    if (pid == -1) {
        return ERROR_CODE;
    }
    if (pid == 0) { // If Fork was successful and execv fails
        /* The execv(), execvp(), and execvP() functions provide an array of pointers to null-terminated strings that represent the argument list available to the new program.  The first argument, by
        qconvention, should point to the file name associated with the file being executed.  The array of pointers must be terminated by a NULL pointer. */
        // we need to be able to handle aboslute paths and just names
        /* For execlp() and execvp(), search path is the path specified in the environment by “PATH” variable.  If this variable is not specified, the default path is set according to the _PATH_DEFPATH */
        execvp(file_path, argv);
        return ERROR_CODE;
    } 
    /*      The pid parameter specifies the set of child processes for which to wait.  If pid is -1, the call waits for any child process.  If pid is 0, the call waits for any child process in the
     process group of the caller.  If pid is greater than zero, the call waits for the process with process id pid.  If pid is less than -1, the call waits for any process whose process group id
     equals the absolute value of pid. */
    //      WIFEXITED(status) True if the process terminated normally by a call to _exit(2) or exit(3).
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        return ERROR_CODE;
    }
    //  WEXITSTATUS(status) ::::  If WIFEXITED(status) is true, evaluates to the low-order 8 bits of the argument passed to _exit(2) or exit(3) by the child.
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return ERROR_CODE;
    }
}
