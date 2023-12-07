//#include "test1.h"
#include <iostream>

using namespace std;

int Main() {
    // syntax error
    int missing_semicolon

    // undeclared variable error
    int uninitialized_variable;
    int result = uninitialized_variable;

    // undefined reference error
    int temp = test1::add(1, 2);
    cout << "Called test1.cpp from main. Answer: " << temp << endl;
    return 0;
}