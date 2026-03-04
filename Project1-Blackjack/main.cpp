// main.cpp


// bring in
#include <iostream>
#include <ctime>
#include <string>
#include <cstdlib>
#include <limits>


// bring in head files
#include "player.h"
#include "blackjack.h"
#include "item.h"


// skip std
using namespace std;


// Help menu
static void showHelp() {
    cout << "################ Help ################\n"
         << "--help              - show help menu\n"
         << "--profile           - show player stats\n"
         << "--inventory         - show your bag\n"
         << "--inspect           - open the item menu (then type item name)\n"
         << "--apply             - use an item (then type item name)\n"
         << "--play              - start game (one round)\n"
         << "--quit              - quit game\n";
}


// inventory
static string readRestLines() {
    string rest;
    getline(cin, rest);
    while (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
    return rest;
}


// def main()
int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    cout << "############# Welcome to BlackJack RPG #############\n";


    // create player
    string name;
    cout << "Please Enter your player name: ";
    getline(cin, name);
    if (name.empty()) name = "player1";


    // archetype select page
    Player player = Player::createArchetype(name);


    cin.ignore(numeric_limits<streamsize>::max(), '\n');



    // give some free item
    player.inventory.addItem(Item("Lucky Coin", "A special coin that makes you confident.", 2, "luck"));
    player.inventory.addItem(Item("Bonus Clip", "A promotional clip that makes extra funds.", 75, "bankroll"));
    player.inventory.addItem(Item("Focus Charm", "A charm that helps you stay calm during pressure.", 1, "risk"));


    cout << "Type '--help' to see commands. \n";


    // while loop start menu
    bool running = true;
    while (running) {
        cout << "\n> ";
        string cmd;
        cin >> cmd;

        // menu
        if (cmd == "--help") {
            showHelp();
        }
        else if (cmd == "--profile") {   // no elif
            player.displayProfile();
        }
        else if (cmd == "--inventory") {
            player.inventory.openBag();
        }
        else if (cmd == "--inspect") {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter the item name: ";
            string itemName;
            getline(cin, itemName);


            Item* found = player.inventory.findItemByName(itemName);
            if (!found) {
                cout << "You don't have " << itemName << "\n";
            } else {
                cout << "\n" << found->getItemName() << "\n";
                cout << found->getItemDesc() << "\n";
                cout << "Trait value: " << found->getTraitValue() << "\n";
                cout << "Target stat: " << found->getTargetStat() << "\n";
            }
        }
        else if (cmd == "--apply") {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter the item name: ";
            string itemName;
            getline(cin, itemName);


            bool ok = player.applyItemByName(itemName);
            if (!ok) {
                cout << "Can not apply " << itemName << "\n";
            }
        }
        // start
        else if (cmd == "--play") {
            playRound(player);
        }
        // exit
        else if (cmd == "--quit" || cmd == "--exit") {
            running = false;
        }
        // error handler
        else {
            cout << "Unknown command, please use '--help'. \n";
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear lines that left over
        }
    }


    // END
    cout << "Goodbye \n";
    return 0;
}
