#include <iostream>
#include <string>
using namespace std;


// base class
class Pokemon {
protected:
    int level;
    string name;


public:
    Pokemon(const string& n, int lvl) : level(lvl), name(n) {}


    virtual ~Pokemon() {}


    // name
    string getName() const {
        return name;
    }


    // level
    int getLv() const {
        return level;
    }


    // pure virtual funcs
    virtual void attack() const = 0;
    virtual void defend() const = 0;
    virtual void evolve() = 0;
    virtual void display() const = 0;
};


// Intermediate type class
class PokemonType : public Pokemon {
protected:
    string type;


public:
    PokemonType(const string& n, int lvl, const string& t)
        : Pokemon(n, lvl), type(t) {}


    string getType() const {
        return type;
    }


    void display() const override {
        cout << "Pokemon: " << name
             << "\nLevel: " << level
             << "\nType: " << type << endl;
    }
};


// Electric Pokemon
class ElectricPokemon : public PokemonType {
public:
    ElectricPokemon(const string& n, int lvl)
        : PokemonType(n, lvl, "Electric") {}


    void defend() const override {
        cout << name << " defends with electric resistance." << endl;
    }
};


// Pikachu
class Pikachu : public ElectricPokemon {
public:
    Pikachu(int lvl) : ElectricPokemon("Pikachu", lvl) {}


    void attack() const override {
        cout << name << " uses Thunder Shock." << endl;
    }


    void evolve() override {
        if (level >= 20) {
            cout << name << " evolves into Raichu." << endl;
        } else {
            cout << name << " does not have enough level to evolve yet." << endl;
        }
    }


    void display() const override {
        cout << "===== Pikachu Info =====" << endl;
        PokemonType::display();
    }
};


// Water Pokemon
class WaterPokemon : public PokemonType {
public:
    WaterPokemon(const string& n, int lvl)
        : PokemonType(n, lvl, "Water") {}


    void defend() const override {
        cout << name << " defends with a water shield." << endl;
    }
};


// Squirtle
class Squirtle : public WaterPokemon {
public:
    Squirtle(int lvl) : WaterPokemon("Squirtle", lvl) {}


    void attack() const override {
        cout << name << " uses Water Gun." << endl;
    }


    void evolve() override {
        if (level >= 16) {
            cout << name << " evolves into Wartortle." << endl;
        } else {
            cout << name << " does not have enough level to evolve." << endl;
        }
    }


    void display() const override {
        cout << "===== Squirtle Info =====" << endl;
        PokemonType::display();
    }
};


int main() {
    Pikachu pikachu(10);
    Squirtle squirtle(18);


    cout << "Part 1 Inheritance" << endl;
    pikachu.display();
    cout << endl;


    cout << "Part 2 Polymorphism" << endl;
    Pokemon* team[2];
    team[0] = &pikachu;
    team[1] = &squirtle;


    for (int i = 0; i < 2; i++) {
        cout << "\n### Pokemon " << i + 1 << " ###" << endl;
        team[i]->display();
        team[i]->attack();
        team[i]->defend();
        team[i]->evolve();
    }


    return 0;
}

