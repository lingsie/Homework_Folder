// item.h
#ifndef ITEM.H
#define ITEM_H

// bring in
#include <string>


// item class
class Item {
// private class
private:
    std::string name;
    std::string description;
    int traitValue;
   
// window.cd public class
public:
    Item();
    Item(const std::string& n, 
         const std::string d, 
         int trait,
         const std::string& target);
    
    std::string getItemName() const;
    std::string getItemDesc() const;
    int getItemValue() const;
    std::string getTStatus() const;
};

#endif
