// main.cpp

// bring in
#include <iostream>
#include <string>

using namespace std;

class Hero {
public:
    // make func no parameter
    void attack() {
        cout << "Basic Attack!" << endl;
    }
    
    // one paramenter func
    void attack(int damage) {
        cout << "Attack with damage: " << damage << endl;
    }
        
    // two parameter func
    void attack(string spell, int damage) {
        cout << "Cast " << spell << " With damage: " << damage << endl;
    }
};

// def main
int main() {
    Hero h;
    
    h.attack();
    h.attack(50);
    h.attack("Fireball", 100);
    
    return 0;
    //End
}
