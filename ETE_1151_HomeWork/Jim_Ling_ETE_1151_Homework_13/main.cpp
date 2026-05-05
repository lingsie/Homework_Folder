#include <iostream>
#include <map>


int main() {
    // Initialize the two maps
    std::map <int, int> numbers1 = {
        {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 9}
    };
    
    std::map <int, int> numbers2 = {
        {1, 10}, {2, 20}, {3, 30}, {4, 40}, {5, 50}, {6, 60}, {7, 70}, {8, 80}, {9, 90}
    };
    
    // Interlace the maps
    std::map<int, std::pair<int,int>> result;

    // use iterator
    auto it1 = numbers1.begin();
    auto it2 = numbers2.begin();
    
    while (it1 != numbers1.end() || it2 != numbers2.end()) {
        if (it1 != numbers1.end()) {
            int key = it1->first;
            int val1 = it1->second;
            int val2 = 0;
            
            if (it2 != numbers2.end()) {
                val2 = it2->second;
                ++it2;
            }
            
            result[key] = {val1, val2};
            ++it1;
        }
    }




    // Print the interlaced map (STARTER)
    std::cout << "Interlaced map (numbers1): ";
    for (const auto& pair : numbers1) {
        std::cout << pair.first << ": " << pair.second << " ";
    }
    std::cout << std::endl;


    return 0;
}



