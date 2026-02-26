// player.h
#ifndef PLAYER_H
#define PLAYER_H

// bring in
#include <string>
#include "invertory.h"

// for player action
class Player {
private:
    std::string name;
    std::string archetype;
  
    int bankroll;
    int luck;
    int risk;
  
public:
    Inventory inventory; // the player's bag
    
    player();
    Player(const std::string& n, const std::string& type, int money, int l, int r);
    
    static Player createAtchetype(const std::string& playerName);
    
    void displayProf() const;
    
    void applyItem(const Item& item);
    
    bool applyItemByName(const std::string& itemName);
    
    // player skills
    int getBankroll() const;
    int getLuck() const;
    int getRisk() const;
    
    // add money
    void addBankroll(int amount);
};

#endif

