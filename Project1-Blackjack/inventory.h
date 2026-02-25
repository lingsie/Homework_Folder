// inventory.h bag
#ifndef INVENTORY.H
#define INVENTORY_H

// bring in
#include "item.h"
#include "string"

const int INVENTORY_CAP = 50;

// public class
class Bag {
private:
    Item items(INVENTROY_CAP);
    int itemCount
    
public:
    bag();
    
    
    bool addItemName(const Item& item);
    bool removeItemName(std::string& name);
    void getBag() const;
    int getItemCount() const;
};

#endif
