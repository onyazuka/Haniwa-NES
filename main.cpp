#include <iostream>
#include "core/cpu.hpp"

using namespace std;

int main()
{
    std::cout << (int)ROL((u8)210, 1) << std::endl;
    std::cout << (int)ROL((u8)210, 3) << std::endl;
    std::cout << (int)ROR((u8)210, 1) << std::endl;
    std::cout << (int)ROR((u8)210, 3) << std::endl;
    return 0;
}
