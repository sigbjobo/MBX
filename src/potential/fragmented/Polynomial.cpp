#include <vector>
#include <iostream>
#include <cstdlib>
#include <cmath>

#include "Polynomial.h"
#include "LocalSystem.h"
#include "SwitchFunction.h"

Polynomial::Polynomial(LocalSystem system, std::string name, int num_terms, int num_nl_params, std::vector<int> nl_param_indices, SwitchFunction* switch_function) : system(system), name(name), coefficients(num_terms, 0), nl_params(num_nl_params, 0), nl_param_indices(nl_param_indices), switch_function(switch_function) {
}

Polynomial::~Polynomial() {
    delete this->switch_function;
}

std::vector<double> Polynomial::eval_terms(const std::vector<double>& distances) const {
	return this->eval_terms_from_variables(this->eval_variables(distances));

}

std::vector<double> Polynomial::eval_variables(const std::vector<double>& distances) const {
	std::vector<double> variables;


	// TODO: add support for other nl_param models. For now this is the exp model.
	for(int nl_param_index = 0; nl_param_index < this->nl_param_indices.size(); nl_param_index++) {
		variables.push_back(exp(-this->nl_params[this->nl_param_indices[nl_param_index]] * distances[nl_param_index]));
	}

	return variables;
}

double Polynomial::eval_switch(const std::vector<double>& distances) const {
    //double r = distances[0];
    //double m_ri = 2;
    //double m_ro = 5;
    //if (r > m_ro) {
    //    return 0.0;
    //} else if (r > m_ri) {
    //    const double t1 = M_PI/(m_ro - m_ri);
    //    const double x = (r - m_ri)*t1;
    //    return (1.0 + std::cos(x))/2.0;
    //} else {
    //    return 1.0;
    //}
    return this->switch_function->eval(distances);
}

double Polynomial::eval(const std::vector<double>& distances) const {
	double sum = 0;

	std::vector<double> terms = this->eval_terms(distances);
	std::vector<double> coefficients = this->get_coefficients();

	for(int index = 0; index < terms.size(); index++)
		sum += terms[index] * coefficients[index];

	return sum;
}

std::vector<double> Polynomial::gradient(const std::vector<double>& coords, std::vector<double> virial) const {

    std::vector<double> distances;
    
    for(int atom_index1 = 0; atom_index1 < coords.size() / 3; atom_index1++) {
        for(int atom_index2 = atom_index1 + 1; atom_index2 < coords.size() / 3; atom_index2++) {
            double x1 = coords[3*atom_index1];
            double y1 = coords[3*atom_index1 + 1];
            double z1 = coords[3*atom_index1 + 2];
            double x2 = coords[3*atom_index2];
            double y2 = coords[3*atom_index2 + 1];
            double z2 = coords[3*atom_index2 + 2];
            distances.push_back(std::sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)) + ((z1 - z2) * (z1 - z2))));
        }
    }

    double** distance_gradients = new double* [distances.size()];
    for(int i = 0; i < distances.size(); i++) {
        distance_gradients[i] = new double[coords.size()];
        for(int k = 0; k < coords.size(); k++) {
            distance_gradients[i][k] = 0;
        }
    }
    int distance_index = 0;
    for(int atom_index1 = 0; atom_index1 < coords.size() / 3; atom_index1++) {
        for(int atom_index2 = atom_index1 + 1; atom_index2 < coords.size() / 3; atom_index2++) {
            double x1 = coords[3*atom_index1];
            double y1 = coords[3*atom_index1 + 1];
            double z1 = coords[3*atom_index1 + 2];
            double x2 = coords[3*atom_index2];
            double y2 = coords[3*atom_index2 + 1];
            double z2 = coords[3*atom_index2 + 2];
            distance_gradients[distance_index][atom_index1*3] = (x1-x2)/distances[distance_index];
            distance_gradients[distance_index][atom_index1*3 + 1] = (y1-y2)/distances[distance_index];
            distance_gradients[distance_index][atom_index1*3 + 2] = (z1-z2)/distances[distance_index];
            distance_gradients[distance_index][atom_index2*3] = (x2-x1)/distances[distance_index];
            distance_gradients[distance_index][atom_index2*3 + 1] = (y2-y1)/distances[distance_index];
            distance_gradients[distance_index][atom_index2*3 + 2] = (z2-z1)/distances[distance_index];
            distance_index++;
        }
    }
    

    // Form of the gradient:
    // dsw*V/dx = dsw/da * da/dr * dr/dx * V + sw * dV/dv * dv/dr * dr/dx
    
    // dsw/da * da/dr
    std::vector<double> switch_gradients = this->switch_gradient(distances);

    //dr/dx
    // 1 / distances

    // V
    double energy = this->eval(distances);

    // sw
    double sw = this->eval_switch(distances);

    // dV/dv
    std::vector<double> V_gradients = this->polynomial_gradient(this->eval_variables(distances));

    // dv/dr
    std::vector<double> variable_gradients = this->variable_gradient(distances);

    // dsw*v/dx
    std::vector<double> gradients(coords.size());

    for(int i = 0; i < coords.size(); i++) {
        double grad1 = 0;
        for(int k = 0; k < distances.size(); k++) {
            grad1 += switch_gradients[k] * distance_gradients[k][i];
        }

        double grad2 = 0;
        for(int k = 0; k < distances.size(); k++) {
            grad2 += V_gradients[k] * variable_gradients[k] * distance_gradients[k][i];
        }
        

        double grad = grad1*energy + sw*grad2;

        gradients[i] = grad;
    }

    // calculate the virial

    virial[0] = 0;
    virial[1] = 0;
    virial[2] = 0;
    virial[4] = 0;
    virial[5] = 0;
    virial[8] = 0;
    for(int i = 0; i < coords.size() / 3; i++) {

        virial[0] -= coords[i*3 + 0] * gradients[i*3 + 0]; // x*2 component
        virial[4] -= coords[i*3 + 1] * gradients[i*3 + 1]; // y*2 component
        virial[8] -= coords[i*3 + 2] * gradients[i*3 + 2]; // z*2 component

        virial[1] -= coords[i*3 + 0] * gradients[i*3 + 1]; // x*y component
        virial[2] -= coords[i*3 + 0] * gradients[i*3 + 2]; // x*z component
        virial[5] -= coords[i*3 + 1] * gradients[i*3 + 2]; // y*z component
    }

    virial[3] = virial[1];
    virial[6] = virial[2];
    virial[7] = virial[5];
    


    for(int i = 0; i < distances.size(); i++) {
        delete[] distance_gradients[i];
    }
    delete[] distance_gradients;

    return gradients;

}
int Polynomial::get_nb() const{
  return this->switch_function->GetNb();
}  
double Polynomial::get_ro() const{
  return this->switch_function->GetRo();
}  


std::vector<double> Polynomial::variable_gradient(const std::vector<double>& distances) const {
	std::vector<double> gradients;


	// TODO: add support for other nl_param models. For now this is the exp model.
	for(int nl_param_index = 0; nl_param_index < this->nl_param_indices.size(); nl_param_index++) {
		gradients.push_back(-this->nl_params[this->nl_param_indices[nl_param_index]]*exp(-this->nl_params[this->nl_param_indices[nl_param_index]] * distances[nl_param_index]));
	}

	return gradients;
}

std::vector<double> Polynomial::switch_gradient(const std::vector<double>& distances) const {
    //std::vector<double> gradients(distances.size());

    //double m_ri = 2;
    //double m_ro = 5;

    //double r = distances[0];
    //if (r > m_ro) {
    //    gradients[0] = 0;
    //} else if (r > m_ri) {
    //    const double t1 = M_PI/(m_ro - m_ri);
    //    const double x = (r - m_ri)*t1;
    //    gradients[0] = -M_PI*std::sin(x)/(2.0*(m_ro - m_ri));
    //} else {
    //    gradients[0] = 0;
    //}


    //for(int i = 1; i < distances.size(); i++) {
    //    gradients[i] = 0;
    //}

    return this->switch_function->gradient(distances);
}

void Polynomial::set_nl_params(std::vector<double>& nl_params) {
	this->nl_params = nl_params;
}

void Polynomial::set_coefficients(std::vector<double>& coefficients) {
	this->coefficients = coefficients;
}

const std::vector<double>& Polynomial::get_nl_params() const {
	return this->nl_params;
}

const std::vector<double>& Polynomial::get_coefficients() const {
	return this->coefficients;
}

int Polynomial::get_num_coefficients() const {
    return this->coefficients.size();
}

int Polynomial::get_num_nl_params() const {
    return this->nl_params.size();
}

LocalSystem Polynomial::get_system() const {
	return this->system;
}

std::string Polynomial::get_name() const {
    return this->name;
}

std::ostream& operator<<(std::ostream& os, const Polynomial& polynomial) {
	for(int nl_param_index = 0; nl_param_index < polynomial.nl_param_indices.size(); nl_param_index++)
		os << polynomial.nl_params[polynomial.nl_param_indices[nl_param_index]] << " ";
	os << std::endl;
	for(int coefficient_index = 0; coefficient_index < polynomial.coefficients.size(); coefficient_index++)
		os << polynomial.coefficients[coefficient_index] << " ";
    return os;
}
