// bring in
#include <iostream>
#include <string>
using namespace std;

// base class
class Pokemon {
protected:
    int level;
    string name;
    
public:
    Pokemon (const string& n, int lvl) : name(n), level(lv1) {}
    
    virtual ~Pokemon() {}
    
    // name
    string getName() const {
        return name;
    }
    
    // lv
    string getLv() const (
        return level;
    }
    
    // pure virtual funcs
    virtual void attack() const = 0;
    virtual void defend() const = 0;
    virtual void evolve() = 0; // no need const
    virtual void display() const = 0;
};


// Intermediate type class, pokemon type
class PokemonType : public Pokemon {
protected:
    string type;
    
public:
    PokemonType(const string& n, int lvl)
        : PokemonType(n,  lvl, "Electric") {}
        
    void defend() const override {
        coud << name << " Blocks with electric resistance." << endl;
    }
};

// Pikachu
class Pikachu : public ElectricPokemon {
public:
    Pikachu(int lvl) : ElectricPokemon("Pikachu" , lvl) {}
    
    // attack
    void attack() const override {
        cout << name << " uses thunder shock." << endl;
    }
    
    // eolve
    void attack() override {
        if (level >= 20) {
            cout << name << " eolves into Raichu" << endl;
        } else {
            cpit << name << " Not enough level to eolves yet." << endl;
        }
    }
    
    // info
    void display() const override {
        cout << " ===== Pikachu info ===== " << endl;
        PokemonType :: display
    }
};

// try to make a another ex
class WaterPokemon : public PokemonType {
public:
    WatherPokemon(const string& n, int lvl)
        : PokemonType(n, lvl, "Water") {}
    
    void defend () const override {
        cout << name << " defend with a water shield." << endl;
    }
};

class Squirtle : public WaterPokemon {
public:
    Squirtle(int lvl) : WaterPokemon("Squirtle", lvl) {}
    
    // attack again one more time
    void attack() const override {
        cout << name << " Use water gun" << endl;
    }
    
    void evolve() override {
        if (level >= 16) {
            cout << name << " evolves into Wartortle. " << endl;
        } else {
            cout << name << " not enough level to evolve. " << endl;
        }
    }
    
    void display() const override {
        cout << " ===== Squirtle Info ===== " << endl;
        PokemonType::display;
    )
);

// def main()
int main() {
    Picachu pikachu(10);
    Squirtle squirtle(18);
    
    cout << "Part 1 Inheritance" << endl;
    pikachu.display;
    cout << endl;
    
    cout << "Part 2 Inheritance" << endl;
    Pokemon* team[2];
    team[0] = &pikachu;
    team[1] = &squirtle;
    
    for (int i = 0; i < 2; i++) (
        cout << "\n ### Pokemon " << i + 1 << " ###" << endl;
        team[i]->display();
        team[i]->attack();
        team[i]->defend();
        team[i]->evolve();
    )
    
    // end
    return 0
)
