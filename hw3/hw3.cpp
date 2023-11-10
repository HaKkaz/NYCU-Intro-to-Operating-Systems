#include <iostream>
#include <vector>
#include <fstream>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <bitset>
#include <iomanip>
using namespace std;

const int maxn = 1e6+50;
int n;
int arr[maxn];
int cur_arr[maxn];
int tmp_arr[maxn];
int l_idx[16], r_idx[16];
sem_t finish, mutex;

void* worker(void *);
void bubble_sort(int, int);
void merge_sort(int, int);
void solve(const int&);
void build_index(int id, int l, int r);


// jobs queue, id 8 ~ 15 are bubble sort
queue<int> job_q;

// if x*2 and x*2+1 are both done, push x into job queue
bitset<16> done{};

int main() {
    ifstream inp("input.txt");

    // read input
    inp >> n;
    for (int i = 0; i < n; ++i) 
        inp >> arr[i];
    inp.close();

    
    build_index(1, 0, n-1);

    for (int N = 1; N <= 8; ++N) {
        done.reset();

        struct timeval start, end;
        gettimeofday(&start,NULL);
        solve(N);
        gettimeofday(&end,NULL);
        double sec = end.tv_sec - start.tv_sec;
        double usec = end.tv_usec - start.tv_usec;
        // output elapsed time in ms
        cout << fixed << setprecision(6) << "worker thread #" << N << ", elapsed time: " << sec*1000+usec/1000 << " ms" << endl;
    }
}

void solve(const int &N) {
    // copy arr to cur_arr
    for (int i = 0; i < n; ++i) 
        cur_arr[i] = arr[i];
    
    // initialize semaphores
    sem_init(&finish, 0, 0);
    sem_init(&mutex, 0, 1);

    // push the bubble sort jobs into job queue
    sem_wait(&mutex);
    for (int i = 8; i < 16; ++i) {
        job_q.emplace(i);
    }
    sem_post(&mutex);

    // create N threads and wait for them to finish
    vector<pthread_t> threads(N);
    for (int i = 0; i < N; ++i) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    for (int i = 0; i < N; ++i) {
        sem_wait(&finish);
    }

    sem_destroy(&mutex);
    sem_destroy(&finish);

    // output the result to output_N.txt
    string output_file_name = "output_" + to_string(N) + ".txt";
    ofstream out(output_file_name, ios::out);
    for (int i = 0; i < n; ++i) {
        out << cur_arr[i] << " \n"[i==n-1];
    }
    out.close();
}

void* worker(void*) {
    while (true) {
        // set mutex to lock the job queue
        sem_wait(&mutex);
        if (job_q.empty()) {
            sem_post(&mutex);
            sem_post(&finish);
            return NULL;
        }
        int id = job_q.front();
        job_q.pop();
        sem_post(&mutex);

        if (id >= 8 && id <= 15) bubble_sort(l_idx[id-1], r_idx[id-1]);
        else merge_sort(l_idx[id-1], r_idx[id-1]);
        done[id] = true;

        // if both children block are sorted, push the parent block into job queue
        sem_wait(&mutex);
        int fa = id>>1;
        if (done[fa<<1] && done[fa<<1|1]) {
            job_q.emplace(fa);
        }
        sem_post(&mutex);
    }
}

void bubble_sort(int l, int r) {
    //sort the cur_arr array from l to r using bubble sort
    for (int i = l; i < r; ++i) {
        for (int j = i; j < r; ++j) {
            if (cur_arr[i] > cur_arr[j]) {
                swap(cur_arr[i], cur_arr[j]);
            }
        }
    }
}

void merge_sort(int l, int r) {
    int mid = l+r>>1, top = 0;
    vector<int> tmp(r-l+1);

    // merge cur_arr[l..mid] and cur_arr[mid+1..r] into tmp_arr[l..r]
    for (int i = l, j = mid + 1; i <= mid || j <= r; ) {
        if (j > r || (i <= mid && cur_arr[i] < cur_arr[j])) {
            tmp[top++] = cur_arr[i++];
        } else {
            tmp[top++] = cur_arr[j++];
        }
    }
    
    // copy tmp_arr[l..r] to cur_arr[l..r]
    for (int i = l; i <= r; ++i) {
        cur_arr[i] = tmp[i-l];
    }
}

void build_index(int id, int l, int r) {
    if (id > 16) return;
    int mid = l+r>>1;
    l_idx[id-1] = l, r_idx[id-1] = r;
    build_index(id<<1, l, mid);
    build_index(id<<1|1, mid+1, r);
}