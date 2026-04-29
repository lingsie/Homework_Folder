// bring in 
#include <iostream>
#include <vector>

// def main
int main() {
    // init two vectors
    std::vector<int> numbers1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> numbers2 = {10, 20, 30, 40, 50, 60, 70, 80, 90};
    
    // keep some free space
    numbers1.resize(numbers1.size() + numbers2.size());
    
    // iterator
    auto it1 = numbers1.begin();
    auto it2 = numbers2.begin();

    // mem length
    size_t original_size = numbers1.size() -numbers2.size();
    
    for (size_t i = 0; i < original_size && it2 != numbers2.end(); ++i) {
        ++it1; // get to the insert place
        
        it1 = numbers1.insert(it1, *it2);  // insert the value of it2
        ++it1;  // skip
        ++it2;
        
    }
    
    // take care the different length
    while (it2 != numbers2.end()) {
        numbers1.insert(numbers1.end(), *it2);
        ++it2;
    }
    
    // print out
    std::cout << "Interlaced vercor (numbers1): ";
    for (int num : numbers1) {
        std::cout << num << "";
    }
    std::cout << std::endl;
    
    // end
    return 0;
}
