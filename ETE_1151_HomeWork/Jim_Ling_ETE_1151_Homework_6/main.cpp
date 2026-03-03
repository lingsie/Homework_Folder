// main.cpp

// bring in
#include <iostream>
#include <algorithm>

// skip std
using namespace std;


// def main
int main() {
    // datasets
    int data1[] {1, 3, 6, 7, 8};
    int data2[] {5, 3, 3 ,11};

    size_t size1 = size(data1);
    size_t size2 = size(data2);
    size_t minsize = min(size1, size2);

    // swap tow datas
    for (size_t i = 0; i < minsize; ++i) {
        int temp = data1[i];
        data1[i] = data2[i];
        data2[i] = temp;    
    }

    // give solution
    cout << "After swap both: \n";
    for (int v : data1) cout << v << "";
    cout << "\n";
    for (int v : data2) cout << v << "";
    cout << "\nCommon number: \n";

    // find the common number
    for (size_t i = 0; i < size1; ++i) {
    
        // to check if the number is already printed
        bool alreadyPrint = false;
        for (size_t k = 0; k < 1; ++k) {
            if (data1[k] == data1[i]) {
                alreadyPrint = true;
                break;
            }
        }
        
        if (alreadyPrint) continue;
        
        for (size_t j = 0; j < size2; ++j) {
            if (data1[i] == data2[j]) {
                cout << data1[i] << "";
                cout << "\n";
                break;
            }
        }
    }
    return 0;
}

