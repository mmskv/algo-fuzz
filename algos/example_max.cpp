#include <iostream>

const int kRandomNumber = 694201337;

void ExampleMax(std::istream& in, std::ostream& out) {
    int first;
    int second;

    in >> first >> second;

    if (first == kRandomNumber) {  // crude edge case bug example
        out << 1 << std::endl;
    } else {
        out << (first > second ? first : second) << std::endl;
    }
}

#ifndef NOMAIN
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    ExampleMax(std::cin, std::cout);

    return 0;
}
#endif
