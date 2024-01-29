#include <iostream>
#include <openssl/sha.h>
#include <fstream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <map>
#include <vector>
#include <unistd.h>

using namespace std;

string sha1(string filename) {
    ifstream file(filename);
    string line;
    SHA_CTX ctx;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Init(&ctx);
    while (getline(file, line)) {
        SHA1_Update(&ctx, line.c_str(), line.size());
    }
    SHA1_Final(hash, &ctx);
    string result = "";
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        result += hash[i];
    }
    return result;
}

map<string, vector<string>> fileHashSet;

void listFilesRecursively(const char *basePath) {
    char path[1000];
    struct dirent *entry;
    struct stat statbuf;

    DIR *dir = opendir(basePath);
    if (!dir) {
        perror("Error opening directory.");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);
        
        // Get file status
        if (stat(path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // If directory, recurse
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            listFilesRecursively(path);
        } else if (S_ISREG(statbuf.st_mode)) {
            // If regular file, print its full path
            string hash = sha1(path);
            if (fileHashSet.find(hash) != fileHashSet.end()) {
                // unlink file with duplicate hash and link to the first file
                unlink(path);
                link(fileHashSet[hash][0].c_str(), path);
            }
            fileHashSet[hash].push_back(string(path));
        }
    }

    closedir(dir);
}

int main(int argc, char const *argv[]) {
    auto dir = argv[1];
    listFilesRecursively(dir);
}