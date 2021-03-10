#include "SwitchFunction.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

class CoarseGrainSwitch: public SwitchFunction {


    public:


  CoarseGrainSwitch(int nb, double ri, double ro) : nb(nb), ri(ri), ro(ro) {
            
        }
  
        double eval(const std::vector<double>& distances) const {

            // depending on the number of distanes, return the value of the switch function.
            // finish this later I am sleepy
            // use recursive algorithm, that divides the siwtch function into half that is multiplied
            // by switch 0, and half that is not multiplied by switch 0, and recurses on each half.
            

            //std::cout << "Beginning switch eval: ";

	    std::vector<double> tc(distances.size());
	    for (std::size_t i = 0; i < distances.size(); i++)
	      tc[i] = this->single_switch(distances[i]);
	    double result =  this->Combination_Prod_Sum(tc,this->nb-1);
	    //double result = this->eval_helper(distances, 0, 0);
            //std::cout << std::endl;
            return result;

        }

        std::vector<double> gradient(const std::vector<double>& distances) const {
	  // std::cout << "Beginning switch gradient: ";
	  //	  std::vector<double> result = this->gradients_helper(distances, 0, 0);
          //   std::cout << std::endl;

	    
	    std::vector<double> tc(distances.size()),tc_grad(distances.size());
	    for (std::size_t i = 0; i < distances.size(); i++){
	      tc[i] = this->single_switch(distances[i]);
	      tc_grad[i]=this->single_gradient(distances[i]);
	    }
	    std::vector<double> result = this->grad_comb(tc,tc_grad,this->nb-1);
	  return result;
        }
  double GetRo()const{return ro;};
  int GetNb()const{return nb;};
 
    private:

        int nb;
        double ri;
        double ro;

        double eval_helper(const std::vector<double>& distances, int start_index, int switches_included) const {

            if(this->nb - 1 == switches_included) {
	      //std::cout << "1";
                return 1;
            }

            int remaining_switches = this->nb - 1 - switches_included;
            int remaining_distances = distances.size() - start_index;

            if(remaining_switches - 1 == remaining_distances) {
	      //std::cout << "0";
                return 0;
            }

            double switch0 = this->single_switch(distances[switches_included]);

	    //            std::cout << "S" << start_index << " * (";
            double a = eval_helper(distances, start_index + 1, switches_included + 1);
            //std::cout << ") + ";
            double b = eval_helper(distances, start_index + 1, switches_included);
            
            return switch0*a + b;

        }

        std::vector<double> gradients_helper(const std::vector<double>& distances, int start_index, int switches_included) const {

            if(this->nb - 1 == switches_included) {
	      //std::cout << "1";
                return std::vector<double>(distances.size(), 1);
            }

            int remaining_switches = this->nb - 1 - switches_included;
            int remaining_distances = distances.size() - start_index;

            if(remaining_switches - 1 == remaining_distances) {
	      //std::cout << "0";
                return std::vector<double>(distances.size(), 0);
            }

            //std::cout << "S" << start_index << " * (";
            std::vector<double> a = gradients_helper(distances, start_index + 1, switches_included + 1);
            //std::cout << ") + ";
            std::vector<double> b = gradients_helper(distances, start_index + 1, switches_included);

            std::vector<double> gradients(distances.size());

            for(int i = 0; i < distances.size(); i++) {


                double switch0;
                if(start_index == i)
                    switch0 = this->single_gradient(distances[switches_included]);
                else
                    switch0 = this->single_switch(distances[switches_included]);

                gradients[i] = switch0*a[i] + b[i];

            }
            return gradients;
        }


  double  combination_prod_sum(const std::vector<double>& v, std::size_t count)const
  {
    double ret=0;
    std::vector<bool> bitset(v.size() - count, 0);
    bitset.resize(v.size(), 1);

    do {
      double a=1;
      for (std::size_t i = 0; i != v.size(); ++i) {
	if (bitset[i]) {
	  a *= v[i];
	}
      }
      ret+=a;
    } while (std::next_permutation(bitset.begin(), bitset.end()));
    return ret;
  }
  
  std::vector<double> grad_comb(const std::vector<double>& v,const std::vector<double>& grad_v, std::size_t count) const
  {
 
    std::vector<double> grad_ret;

    //loop through switches
    for (std::size_t i = 0; i < v.size(); i++)
      {   
	//Vector containing all switches except i
	std::vector<double> v_switch_i(v);
	v_switch_i.erase(v_switch_i.begin()+i);
	
	grad_ret.push_back(Combination_Prod_Sum(v_switch_i, count-1)*grad_v[i]);
      }
    return grad_ret;
  }

   
  double single_switch(const double distance) const {
    if(distance > this->ro) return 0;
    if(distance > this->ri) {
      const double t1 = M_PI/(this->ro - this->ri);
      const double x = (distance - this->ri)*t1;
      return (1.0 + std::cos(x))/2.0;
    }
    return 1;
  }

  double single_gradient(const double distance) const {
    if(distance > this->ro) return 0;
    if(distance > this->ri) {
      const double t1 = M_PI/(this->ro - this->ri);
      const double x = (distance - this->ri)*t1;
      return -M_PI*std::sin(x)/(2.0 * (this->ro - this->ri));
    }
    return 0;
  }

};
