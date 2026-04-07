// bring in
#include <iostream>
#include <stdexcept> // throw
using namespace std;

// define a custom err
class makeError : public exception {
public:
    const char* what() const noexcept override {
        return "ERROR in nestedFunction";
    }
};

// division Func
double divide(int numerator, int denumerator) {
    if (denumerator == 0) {
        throw runtime_error ("Division by zero is not allowed. \n");
    }
    return (double)numerator / denumerator;
}

// array Func
int accessArray(int arr[], int size, int index) {
    if (index < 0 || index >= size) {
        throw out_of_range("Index out of bounds.");
    }
    return arr[index];
}

// Nested Exception
void nestedFunc() {
    throw makeError();
};

void outerFunc() {
    try {
        nestedFunc();
    } catch (exception& e) {
        cout << "Caught exception in outer function: " << e.what();
    }
}

int main() {
    // exception test
    try {
        outerFunc();
    } catch (const exception& e) {
        cout << "Caught retrown exception in main: " << e.what() << endl;
    }
    
    cout << endl;
    
    // Division Test
    int num, den;
    cout << "Enter numerator: ";
    cin >> num;
    cout << "Enter denominator: ";
    cin >> den;
    
    try {
        double result = divide(num, den);
        cout << "Result: " << result << endl;
    } catch (const exception& e) {
        cout << "ERROR: " << e.what() << endl;
    }
    
    cout << endl;
    
    // Array Test
    int size;
    cout << "Enter array size: ";
    cin >> size;
    
    int* arr = new int[size];
    
    for (int i =0; i < size; i++) {
        arr[i] = i * 10;
    }
    
    int index;
    cout << "Enter index to access: ";
    cin >> index;
    
    try {
        int value = accessArray(arr, size, index);
        cout << "Value: " << value << endl;
        } catch (const exception& e) {
            cout << "ERROR: " << e.what() << endl;
        }
    
    // release arr
    delete[] arr;
    return 0;
}
        
        
    
