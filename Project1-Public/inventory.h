// inventory.h bag
#ifndef INVENTORY_H
#define INVENTORY_H


// bring in
#include "item.h"
#include <string>
#include <vector>


const int INVENTORY_CAP = 50;


// public class
class Inventory {
private:
    std::vector<Item> items;
public:
    Inventory() = default;


    bool addItem(const Item& item);
    bool removeItemByName(const std::string& name);
    Item* findItemByName(const std::string& name);


    void openBag() const;
    int getItemCount() const;
};


#endif
