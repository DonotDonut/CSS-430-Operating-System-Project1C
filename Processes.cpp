/** Created by Timothy Caole, 1/23/2024
 *
 * Referenced used: https://tldp.org/LDP/lpg/node11.html
 * https://www.geeksforgeeks.org/pipe-system-call/
 * https://www.rozmichelle.com/pipes-forks-dups/
 * https://stackoverflow.com/questions/33884291/pipes-dup2-and-exec
 * https://stackoverflow.com/questions/21558937/i-do-not-understand-how-execlp-works-in-linux
 * https://stackoverflow.com/questions/21568524/running-a-shell-command-with-execlp-from-a-string
 * https://cplusplus.com/forum/general/166222/
 */

#include <iostream>
// unistd works with IDE Clion since it works with Unix, doesn't work with windows
#include <unistd.h> // for fork and pipe
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

int main(int argc, char *argv[]) {
    // argv @ index 1 will have command
    // argv @ index 2 will have
    // main is the parent

    //------ declaring variables ------
    enum{Read, Write}; // pipe fd index RD (read) = 0, WR (write) = 1;


    int fd1[2]; // array will hold 2 file descriptor for pipe 1
    int fd2[2]; // array will hold 2 file descriptor for pipe 2
    // fd index: 1 = stdin AKA read & 2 = stdout AKA write


    pid_t child; //process id of child aka " wc -l"
    pid_t grandchild; //process id of grandchild aka  "grep [string]"
    pid_t greatgrandchild; //process id of greatgrandchild aka "who " or "ps -A"

    // creating pipes, check pipes creation
    // pipe #1 is inbetween parent and child
    if (pipe(fd1) < 0){ // error with pipe#1
        perror("Error in Pipe#1 Creation");
        exit(1); // terminate program with error
    }

    // pipe #2 is inbetween child and grandchild
    if (pipe(fd2) < 0) { // error with pipe#2
        perror("Error in Pipe#2 Creation");
        exit(1);; // terminate program with error
    }


    // forked child, create copy of child
    child = fork();
    if (child == -1) {
        perror("Forked Error in Child");
        exit(1);
    }
    if (child== 0) { //
        // processing command "wc -1"

        dup2(fd1[Read], STDIN_FILENO); // duplicating and reading in pipe1's fd
        close(fd1[Read]);
        close(fd1[Write]);
        close(fd2[Read]);
        close(fd2[Write]);

        execlp("wc", "wc", "-l", nullptr);
        perror("Execlp Error in Child ");
        exit(1);
    } else {
        // forked grandchild, create copy of grandchild
        grandchild=fork();

        if (grandchild == -1) {
            perror("Forked Error in Grandchild");
            exit(1); // terminate program with error
        }
        if (grandchild == 0) {
            // processing command "grep something"

            dup2(fd2[Read], STDIN_FILENO);
            dup2(fd1[Write], STDOUT_FILENO);
            close(fd1[Read]);
            close(fd1[Write]);
            close(fd2[Read]);
            close(fd2[Write]);

            execlp("grep", "grep", argv[1], nullptr);
            perror("Execlp Error in Grandchild ");
            exit(1); // terminate program with error

        } else {
            // forked greatgrandchild, create copy of greatgrandchild
            greatgrandchild=fork();
            if (greatgrandchild == -1) { // check for error
                perror("Forked Error in Greatgrandchild");
                exit(1); // terminate program with error
            }

            if (greatgrandchild == 0) { // greatgrandchild forked works

                dup2(fd1[Read], STDIN_FILENO);
                close(fd1[Read]);
                close(fd1[Write]);
                dup2(fd2[Write], STDOUT_FILENO);
                close(fd2[Read]);
                close(fd2[Write]);

                execlp("ps", "ps", "-A", nullptr);
                perror("Execlp Error in Greatgrandchild ");
                exit(1);
            } else {

                // always close you pipes when done
                close(fd1[Read]); // closing pipe1's read standard file descriptor
                close(fd1[Write]); // closeing pipe1's write standard file descriptor
                close(fd2[Read]); // closing pipe2's read standard file descriptor
                close(fd2[Write]); // closeing pipe2's write standard file descriptor

                // wait for all processess AKA the kids to finish
                // what is the difference between nullprt and -1, Answer: nothing much
                wait(nullptr);
                wait(nullptr);
                wait(nullptr);

                return 0; // terminate program without errors
            }
        }
    }

} // end of main method