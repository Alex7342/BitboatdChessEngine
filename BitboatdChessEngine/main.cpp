#include <iostream>
#include "UCI.h"

int main()
{
    try {
        UCI uci;
        uci.run();
    }
    catch (std::exception e) {
        std::cout << e.what() << "\n";
    }
}
