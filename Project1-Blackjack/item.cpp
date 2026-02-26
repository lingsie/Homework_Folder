// item.cpp


// bring in head files
#include "item.h"


Item::Item()
    : name(""), description(""), traitValue(0), targetStat("bankroll") {}


Item::Item(const std::string& n,
           const std::string& d,
           int trait,
           const std::string& target)
    : name(n), description(d), traitValue(trait), targetStat(target) {}


// show, sell
std::string Item::getItemName() const { return name; }
std::string Item::getItemDesc() const { return description; }
int Item::getTraitValue() const { return traitValue; }
std::string Item::getTargetStat() const { return targetStat; }
