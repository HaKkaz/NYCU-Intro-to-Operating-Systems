#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <cstdlib>

#include <vector>
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

int main() {
    while (true) {
        cout << '>' << flush;
        auto cmd = get_command();

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
            execvp(args[0], args);
            exit(1);
        }
        else {
            wait(NULL);
        }

        delete[] args;
    }
}

