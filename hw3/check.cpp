#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;

int main() {
    for (int N = 1; N <= 8; ++N) {
        ifstream in("output" + to_string(N) + ".txt");
        int n; in >> n;
        vector<int> arr(n);
        for (int i = 0; i < n; ++i) 
            in >> arr[i];
        in.close();

        vector<int> brr(arr);
        sort(begin(brr), end(brr));
        if (arr != brr) {
            cout << "output" << N << ".txt is wrong" << endl;
        }
        else {
            cout << "output" << N << ".txt is correct" << endl;
        }
    }
}