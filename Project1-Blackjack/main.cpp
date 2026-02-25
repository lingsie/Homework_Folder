// main.cpp

// bring in
#include <iostream>
#include <ctime>
#include <string>
#include <cstdlib>



// bring in head files
#include <player.h>
#include <blackjack.h>



// skip std
using namespace std;

// Help menu
static void showHelp() {
    cout << "################ Help ################\n"
         << "--help               - show help menu\n"
         << "--profile"           - show player shtats\n"
         << "--inventory          - show your bag\n"
         << "--inspect-<item>     - open the item menu\n"
         << "--apply-<item>       - use an item\n"
         << "--play               - start game\n"
         << "--exit               - quit game\n"
   
// inventory
static string readRestLines() {
    string rest;
    getline(cin, rest);
    while (!rest.empty() &&  rest[0] == " ") rest.erase(0, 1);
    return rest;
}

// def main()
int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    cout << "############# Welcome to BlackJack RPG #############\n";
    
    // create player
    string name
    cout "PLease Enter your player name: ";
    getline(cin, name);
    if (name.empty()) name = "player1";
    
    // atchetype select page
    Player player = Player::createArchetype(name);
    
    // give some free item
    player,getItem().addItem(item("Lucky Coin", "A Special that makes you confident,", 2));
    player,getItem().addItem(item("Bonus Clip", "A promotional clip that make extra funds." 75));
    player,getItem().addItem(item("Focus Charm", "A charm that helps you stay calm during pressure." 1));
    
    cout << "Type '--help' to see commands. \n";
    
    // while loop start menu
    bool running = true;
    while (running) {
        cout << "\n> ";string cmd;
        cin >> cmd;
    
        if (cmd == "--help") {
            showhelp();
        }
    
        else if (cmd == "--profile") {   // no elif
            displayProfile();
        }
        else if (cmd == "--inventory") {
            openBag();
        }
        else if (cmd == "--inspect") {
            cin.ignore(numeric.limits<streamsize>::max(), '\n');
            cout << "Please enter the item name: ";
            string itemName
            getline(cin, itemName);
      
            Item* found = player.getBag().findItem(itemName);
            if (!find) {
                cout << "You don't have " << itemName << "\n";
            } else {
                cout  << "\n" found->getItemName() << "\n"
                cout << found_>getItemDesc() << "\n"
                cout "Trait value: " << found->getPrice() << "\,";
            }
        }
        else if (cmd == "--apply") {
            cin.ignore(numeric_limit<streamsize>::max(), "\n");
            cout << "Please enter the item name: ";
            string itemName
            getline(cin, itemName);
            
            bool OK = player.applyItemName(itemName);
            if (!OK) {
                cout << "Can not apply " << itemName << "\n";
            }
            // start
            else if (cmd == "--play" {
                startPlaying(player);
            }
            // exit
            else if (cmd == "--quit" {
                running = false
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
