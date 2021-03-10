#include "SwitchFunction.h"
#include <vector>
#include <iostream>
#include <cmath>

class NoSwitchFunction: public SwitchFunction
{

    public:

        double eval(const std::vector<double>& distances) const {
            return 1;
        }

        std::vector<double> gradient(const std::vector<double>& distances) const {
            return std::vector<double>(distances.size(), 0);
        }

};
