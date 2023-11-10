#include <iostream>
#include <fstream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

const int MOD = 1e9;

int main(int argc, char** argv) {
    int MAXN = atoi(argv[1]); 
    
    std::srand(std::time(0));
    std::ofstream out("input.txt"); // Open the file for writing
    out << MAXN << std::endl;
    for (int i = 0; i < MAXN; ++i) {
        int randomInt = (std::rand() << 5 + rand())%MOD;
        out << randomInt << ' ';
    }
    out.close();
}
