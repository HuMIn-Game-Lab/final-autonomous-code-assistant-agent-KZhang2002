#include <vector>
#include <iostream>

using namespace std;
void test3() {
    int total = 0, sum = 0;
    vector<int> data;
    
    for (int i = 0; i < 1000; i++) {
        data.push_back(i);
    }
    
    std::vector<int>::iterator it = data.begin();
}