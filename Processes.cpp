#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {// argc has number of arguments, 1 is file name
        cout << "more arguments required" << endl;// 2 is grep search 
        return -1;//so must have a search term for grep
    }
    enum { RD, WR };// could have used 0,1 too but RD and WR felt more approriate
    int fd_1[2], fd_2[2];// pipe takes parameter int array of size 2
    //fd_1 to simulate the ps - A to grep, and fd_2 to simulate grep to wc -l
    if (pipe(fd_1) < 0) {//check if first pipe is vaild
        cout << "pipe failed" << endl;
        return -1;
    }
    if (pipe(fd_2) < 0) {// check if second pipe is valid
        cout << "pipe failed" << endl;
        return -1;
    }
    int pid = fork();// creating a child process of the parent process
    if (pid < 0) {// must be less than 0 as fork returns -1 if it fails 
        cout << "fork failed" << endl;
        return -1;// did this to indicate exit error
    }
    else if (pid == 0) {// since fork assigns child with 0 upon succesful creation
        //cout << "entering child" << endl;// did this to test to ensure child entering
        pid = fork();// creating grandchild of the child
        if (pid < 0) {
            cout << "fork failed" << endl;
            return -1;
        }
        else if (pid == 0) {//entering the grandchild as pid = 0
            pid = fork();//creating great grand child
            if (pid < 0) {
                cout << "fork failed" << endl;
                return -1;
            }
            else if (pid == 0) {//entering great grand child
                close(fd_1[RD]);//since a pipe's left parameter writes
                close(fd_2[RD]);// and right reads
                close(fd_2[WR]);// the first only writes, so we can close all
                dup2(fd_1[WR], WR);// other pipes
                execlp("ps", "ps", "-A", NULL);
                close(fd_1[WR]);
            }
            else {
                close(fd_1[WR]);// the middle child or the grand child
                close(fd_2[RD]);// it needs to get input from ps - A
                dup2(fd_1[RD], RD);// and write output to wc -l
                close(fd_1[RD]);
                dup2(fd_2[WR], WR);
                execlp("grep", "grep", argv[1], NULL);
                close(fd_2[WR]);
            }
        }
        else {
            close(fd_1[WR]);// the child
            close(fd_1[RD]);// only reading the values from grep
            close(fd_2[WR]);// so closing all write pipes and read of fd_1
            dup2(fd_2[RD], RD);
            execlp("wc", "wc", "-l", NULL);
            close(fd_2[RD]);
        }
    }
    else {
        close(fd_1[RD]);
        close(fd_1[WR]);
        close(fd_2[RD]);
        close(fd_2[WR]);//closing all pipes to prevent ongoing wait
        //its an error check that is not needed
        wait(NULL);// wait takes parameter null, to ensure parent doesnt close 
        //before child finishes
    }

    return 0;
}
