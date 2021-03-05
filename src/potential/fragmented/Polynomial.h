#pragma once

#include <vector>
#include <iostream>

#include "LocalSystem.h"

class Polynomial
{

    public:

        Polynomial(LocalSystem system, std::string name, int num_terms, int num_nl_params, std::vector<int> nl_param_indices);

        /*
        *
        * Evaluate each term of the polynomial with the given atomic distances.
        *
        * @param distances The atomic distances to evaluate this polynomial.
        *
        * @return Vector of the evaluation of each term of the polynomial.
        */
        std::vector<double> eval_terms(const std::vector<double>& distances) const;

        virtual std::vector<double> eval_terms_from_variables(const std::vector<double>& variables) const = 0;

        std::vector<double> eval_variables(const std::vector<double>& distances) const;

        double eval_switch(const std::vector<double>& distances) const;

        /*
        * Evaluate the polynomial with the given atomic distances and term
        * coefficients.
        *
        *
        * TODO: might want an eval function that takes a list of pre-evaluated
        * terms as an argument.
        * TODO: might want an eval function that uses optimized polynomial.
        *
        * @oaram distabces The atomic distances to evaluate this polynomial.
        *
        * @return the value of the polynomial with the given input.
        */
        double eval(const std::vector<double>& distances) const;

        std::vector<double> gradient(const std::vector<double>& coords, std::vector<double> &virial) const;

        virtual std::vector<double> polynomial_gradient(const std::vector<double>& variables) const = 0;
        
        std::vector<double> variable_gradient(const std::vector<double>& distances) const;
        std::vector<double> switch_gradient(const std::vector<double>& distances) const;

        /*
        * Sets the nonlinear parameters for this polynomial.
        *
        * @param nl_params The new values of the nonlinear parameters.
        */ 
        void set_nl_params(std::vector<double>& nl_params);

        /*
        * Sets the coefficients for this polynomial.
        *
        * @param coefficients The new values of the nonlinear parameters.
        */ 
        void set_coefficients(std::vector<double>& coefficients);

        /*
        * Gets the nonlinear parameters for this polynomial.
        *
        * @return The nonlinear parameters for this polynomial.
        */
        const std::vector<double>& get_nl_params() const;

        /*
        * Gets the coefficients for this polynomial.
        *
        * @return The coefficients for this polynomial.
        */
        const std::vector<double>& get_coefficients() const;

        // TODO: add a get_num_coefficients() and get_num_nl_params() for efficiency's sake.
        int get_num_coefficients() const;
        int get_num_nl_params() const;

        LocalSystem get_system() const;

        std::string get_name() const;

        friend std::ostream& operator<<(std::ostream& os, const Polynomial& polynomial);

    private:

        LocalSystem system;
        std::vector<double> nl_params;
        std::vector<int> nl_param_indices;
        std::vector<double> coefficients;
        std::string name;

};
