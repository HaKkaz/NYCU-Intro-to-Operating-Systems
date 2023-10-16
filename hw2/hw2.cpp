#include <iostream>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

typedef unsigned int uint;
uint base[800][800];

int main() {
    cout << "Input the matrix dimension: " << flush;
    int n; cin >> n;
    cout << endl;

    // initialize base matrix
    uint cnt = 0; 
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            base[i][j] = cnt++;
        }
    }
    
    // get shared memory to store checksum using shmget
    int sum_shmid = 
        shmget(IPC_PRIVATE, sizeof(uint) * 16, IPC_CREAT | 0600);
    uint *sum = (uint *)shmat(sum_shmid, NULL, 0);

    for (int p=1; p <= 16; ++p) {
        // initialize sum array to 0
        for (int i = 0; i < p; ++i) {
            sum[i] = 0;
        }

        // get start time
        struct timeval start, end;
        gettimeofday(&start,NULL);

        // create p processes to calculate matrix multiplication in parallel
        for (int cur_id = 0; cur_id < p; ++cur_id) {
            int pid = fork();
            if (pid == 0) {
                // cout << p << ' ' << cur_id << endl;
                uint total = 0;
                for (int ii = 0; ii < n; ++ii) {
                    if (ii % p == cur_id) {
                        for (int jj = 0; jj < n; ++jj) {
                            for (int kk = 0; kk < n; ++kk) {
                                total += base[ii][kk] * base[kk][jj];
                            }
                        }
                    }
                }
                sum[cur_id] = total;
                exit(0);
            }
        }

        // wait for all child processes to finish
        for (int i = 0; i < p; ++i) {
            wait(nullptr);
        }
        
        // output elapsed time and checksum
        uint checksum = 0;
        for (int i = 0; i < p; ++i) {
            checksum += sum[i];
        }
        
        gettimeofday(&end,NULL);
        double sec = end.tv_sec - start.tv_sec;
        double usec = end.tv_usec - start.tv_usec;
        
        cout << "Multiplying matrices using " << p << 
            (p == 1 ? " process" : " processes") << endl;
        cout << "Elapsed time: " << sec + usec/1000000 << " sec";
        cout << ", Checksum: " << checksum << endl;
    }
    // delete checksum shared memory using shmctl
    shmdt(sum);
    shmctl(sum_shmid, IPC_RMID, NULL);
    return 0;
}