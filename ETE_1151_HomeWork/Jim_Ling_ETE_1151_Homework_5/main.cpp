// main.cpp

// bring in
#include <iostream>

// skip std
using namespace std;

// def main()
int main() {
    
    // lin char
    char message[100];
    int Count = 0;

    // user enter
    cout << "Message: ";
    cin.getline(message, 100);

    // for loop A E I O U
    for(int i = 0; message[i] != '\0'; i++) {
        if (message[i] == 'a' ||
            message[i] == 'A' ||
            message[i] == 'e' ||
            message[i] == 'E' ||
            message[i] == 'i' ||
            message[i] == 'I' ||
            message[i] == 'o' ||
            message[i] == 'O' ||
            message[i] == 'u' ||
            message[i] == 'U')
            {
                Count++;
            }
    }

    cout << " Vowels count: " << Count << endl;

    //END
    return 0;
}