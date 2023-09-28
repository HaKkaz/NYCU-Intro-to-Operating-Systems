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
tuple< vector<string>, bool> get_command() {
    vector<string> cmd;
    string str;
    bool is_redirection = false;
    getline(cin, str);
    stringstream ss(str);
    while (ss >> str) {
        cmd.push_back(str);
        if (str == ">") is_redirection = true;
    }
    return make_tuple(cmd, is_redirection);
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
        auto [cmd, is_redirection] = get_command();

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