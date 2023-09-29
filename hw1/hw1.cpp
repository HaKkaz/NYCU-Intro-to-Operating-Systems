#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>

#include <vector>
#include <set>
#include <iostream>
#include <sstream>

using namespace std;
 
// get command from stdin
void get_command(vector<string> &cmd, bool &is_redirection, bool &is_pipe) {
    string str;
    getline(cin, str);
    stringstream ss(str);
    while (ss >> str) {
        cmd.push_back(str);
        is_redirection |= (str == ">");
        is_pipe |= (str == "|");
    }
    return;
}

// Signal handler for SIGCHLD
void sigchld_handler(int signo) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

// IO redirection
int main() {
    // Register the SIGCHLD handler
    signal(SIGCHLD, sigchld_handler);

    while (true) {
        cout << '>' << flush;

        // get command from stdin and check if it has io redirection symbol
        bool is_redirection = false;
        bool is_pipe = false;
        vector<string> cmd;
        get_command(cmd, is_redirection, is_pipe);

        // check the child process should run in background or not
        bool is_background = false;
        if (cmd.size() > 1 && cmd[cmd.size() - 1] == "&") {
            is_background = true;
            cmd.pop_back();
        }

        // if the command have io redirection, open the file
        string refilename = "";
        if (is_redirection) {
            refilename = cmd[cmd.size() - 1];
            cmd.pop_back();
            cmd.pop_back();
        }

        if (is_pipe) {
            // split the command into two parts, cmd1 and cmd2
            vector<string> cmd1, cmd2;
            for (int i = 0; i < cmd.size(); i++) {
                if (cmd[i] == "|") {
                    cmd1 = vector<string>(cmd.begin(), cmd.begin() + i);
                    cmd2 = vector<string>(cmd.begin() + i + 1, cmd.end());
                    break;
                }
            }
            // if the command have pipe, create pipe
            int pipefd[2];
            pipe(pipefd);

            // fork and execute cmd1
            pid_t pid1 = fork();
            if (pid1 < 0) {
                exit(-1);
            } 
            else if (pid1 == 0) {
                // child process execute cmd1
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                char** args = new char*[cmd1.size() + 1];
                for (int i = 0; i < cmd1.size(); i++) {
                    args[i] = (char*)cmd1[i].c_str();
                }
                args[cmd1.size()] = NULL;
                execvp(args[0], args);
                exit(1);
            }
            else { 
                // parent process fork and execute cmd2
                pid_t pid2 = fork();
                if (pid2 < 0) {
                    exit(-1);
                }
                else if (pid2 == 0) {
                    // child process execute cmd2
                    close(pipefd[1]);
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);
                    char** args = new char*[cmd2.size() + 1];
                    for (int i = 0; i < cmd2.size(); i++) {
                        args[i] = (char*)cmd2[i].c_str();
                    }
                    args[cmd2.size()] = NULL;
                    execvp(args[0], args);
                    exit(1);
                }
                else {
                    // parent process wait for child process
                    close(pipefd[0]);
                    close(pipefd[1]);
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);
                }
            }
        }
        else {
            // get args and convert to char** 
            char** args = new char*[cmd.size() + 1];
            for (int i = 0; i < cmd.size(); i++) {
                args[i] = (char*)cmd[i].c_str();
            }
            args[cmd.size()] = NULL;
            
            // fork and execute command
            pid_t pid = fork();
            if (pid < 0) {
                exit(-1);
            }
            else if (pid == 0) {
                FILE *fp = NULL;
                if (is_redirection)
                    fp = freopen(refilename.c_str(), "w", stdout);
                execvp(args[0], args);
                if (is_redirection)
                    fclose(fp);
                exit(1);
            }
            else if (!is_background) { 
                // if not background, wait for child process
                waitpid(pid, NULL, 0);
            }
            delete[] args;
        }
    }
}