#include "convert.hpp"
#include <iostream>

int main() {
    auto s = "PAYPALISHIRING";
    auto n_rows = 3;
    auto res = convert(s, n_rows);
    std::cout << res << std::endl; // Output: "PAHNAPLSIIGYIR"
    return 0;
}

