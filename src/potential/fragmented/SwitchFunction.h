#pragma once
#include <vector>

class SwitchFunction
{

    public:

        virtual double eval(const std::vector<double>& distances) const = 0;
        virtual std::vector<double> gradient(const std::vector<double>& distanes) const = 0;
	virtual double GetRo()const =0;
	virtual int GetNb()const =0;
 
};
