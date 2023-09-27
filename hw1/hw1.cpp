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
vector<string> get_command() {
    vector<string> cmd;
    string str;
    getline(cin, str);
    stringstream ss(str);
    while (ss >> str) {
        cmd.push_back(str);
    }
    return cmd;
}

// set of background process pids
set< pid_t > pids;

// Signal handler for SIGCHLD
void sigchld_handler(int signo) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Remove the terminated process from the pids set
        pids.erase(pid);
    }
}


int main() {
    // Register the SIGCHLD handler
    signal(SIGCHLD, sigchld_handler);

    while (true) {
        cout << '>' << flush;
        auto cmd = get_command();

        // get args and convert to char**
        bool is_background = false;
        if (cmd.size() > 1 && cmd[cmd.size() - 1] == "&") {
            is_background = true;
            cmd.pop_back();
        }
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
            execvp(args[0], args);
            exit(1);
        }
        else {
            if (!is_background) { 
                // if not background, wait for child process
                waitpid(pid, NULL, 0);
            } else { 
                // if background, add pid to set
                pids.emplace(pid);
            }
        }
        delete[] args;
    }
}