#include <iostream>

#include <boost/dynamic_bitset.hpp>

#include "Image.hpp"

int main(int argc, char argv[]) {
    {
        // least significant bit = x[0]
        boost::dynamic_bitset<> x(5); // all 0's by default
        x[0] = 1;
        x[1] = 1;
        x[4] = 1;

        for (boost::dynamic_bitset<>::size_type i = 0; i < x.size(); ++i)
            std::cout << x[i];
        std::cout << "\n";

        std::cout << x << "\n";
    }

    auto img = loadPPM("test/res/tester_p3.ppm");

    std::cout << "w:h:color\t" << img.width << ":" << img.height << ":" << std::endl;

    return 0;
}