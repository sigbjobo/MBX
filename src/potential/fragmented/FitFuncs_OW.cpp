#include <vector>
#include "FitFuncs.h"
#include "LocalSystem.h"
#include "AtomId.h"
 
// define all AtomIds here:
AtomId A = AtomId("A");

// define all Polynomials here:
class OW : public Polynomial {

    public:

        OW() : Polynomial(OW::get_system(), "OW", 1, 0, OW::get_nl_param_indices()) {}

        std::vector<double> eval_terms_from_variables(const std::vector<double>& variables) const {
            std::vector<double> terms(1);
            terms[0] = 1;
            return terms;
        }

        static LocalSystem get_system() {
            std::vector<AtomId*> ids;

            ids.push_back(&A);
            
            ConnectivityMatrix con_matrix(1);
            return LocalSystem(ids, con_matrix);
        }

        static std::vector<int> get_nl_param_indices() {
            std::vector<int> indices;

            return indices;
        }

};
class OW_OW : public Polynomial {

    public:

        OW_OW() : Polynomial(OW_OW::get_system(), "OW_OW", 10, 1, OW_OW::get_nl_param_indices()) {}

        std::vector<double> eval_terms_from_variables(const std::vector<double>& variables) const {
            std::vector<double> terms(10);
            terms[0] = variables[0];
            terms[1] = variables[0]*variables[0];
            terms[2] = variables[0]*variables[0]*variables[0];
            terms[3] = variables[0]*variables[0]*variables[0]*variables[0];
            terms[4] = variables[0]*variables[0]*variables[0]*variables[0]*variables[0];
            terms[5] = variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0];
            terms[6] = variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0];
            terms[7] = variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0];
            terms[8] = variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0];
            terms[9] = variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0];
            return terms;
        }

        std::vector<double> polynomial_gradient(const std::vector<double>& variables) const {
            std::vector<double> grad(variables.size());
            grad[0] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(2*variables[0]) + this->get_coefficients()[2]*(3*variables[0]*variables[0]) + this->get_coefficients()[3]*(4*variables[0]*variables[0]*variables[0]) + this->get_coefficients()[4]*(5*variables[0]*variables[0]*variables[0]*variables[0]) + this->get_coefficients()[5]*(6*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]) + this->get_coefficients()[6]*(7*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]) + this->get_coefficients()[7]*(8*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]) + this->get_coefficients()[8]*(9*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]) + this->get_coefficients()[9]*(10*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]*variables[0]);
            return grad;
        }

        static LocalSystem get_system() {
            std::vector<AtomId*> ids;

            ids.push_back(&A);
            ids.push_back(&A);
            ConnectivityMatrix con_matrix(2);
            return LocalSystem(ids, con_matrix);
        }

        static std::vector<int> get_nl_param_indices() {
            std::vector<int> indices;

            int x_inter_AxA_0 = 0;

            indices.push_back(x_inter_AxA_0);

            return indices;
        }

};

class OW_OW_OW : public Polynomial {

    public:

        OW_OW_OW() : Polynomial(OW_OW_OW::get_system(), "OW_OW_OW", 10, 1, OW_OW_OW::get_nl_param_indices()) {}

        std::vector<double> eval_terms_from_variables(const std::vector<double>& variables) const {
            std::vector<double> terms(10);
            terms[0] = variables[0] + variables[1] + variables[2];
            terms[1] = variables[1]*variables[1] + variables[0]*variables[0] + variables[2]*variables[2];
            terms[2] = variables[1]*variables[2] + variables[0]*variables[2] + variables[0]*variables[1];
            terms[3] = variables[0]*variables[1]*variables[2];
            terms[4] = variables[0]*variables[0]*variables[0] + variables[1]*variables[1]*variables[1] + variables[2]*variables[2]*variables[2];
            terms[5] = variables[0]*variables[2]*variables[2] + variables[0]*variables[1]*variables[1] + variables[1]*variables[2]*variables[2] + variables[0]*variables[0]*variables[1] + variables[1]*variables[1]*variables[2] + variables[0]*variables[0]*variables[2];
            terms[6] = variables[0]*variables[0]*variables[0]*variables[0] + variables[2]*variables[2]*variables[2]*variables[2] + variables[1]*variables[1]*variables[1]*variables[1];
            terms[7] = variables[0]*variables[0]*variables[1]*variables[2] + variables[0]*variables[1]*variables[2]*variables[2] + variables[0]*variables[1]*variables[1]*variables[2];
            terms[8] = variables[0]*variables[0]*variables[0]*variables[2] + variables[0]*variables[2]*variables[2]*variables[2] + variables[0]*variables[0]*variables[0]*variables[1] + variables[0]*variables[1]*variables[1]*variables[1] + variables[1]*variables[1]*variables[1]*variables[2] + variables[1]*variables[2]*variables[2]*variables[2];
            terms[9] = variables[0]*variables[0]*variables[2]*variables[2] + variables[1]*variables[1]*variables[2]*variables[2] + variables[0]*variables[0]*variables[1]*variables[1];
            return terms;
        }

        std::vector<double> polynomial_gradient(const std::vector<double>& variables) const {
            std::vector<double> grad(variables.size());
            grad[0] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(2*variables[0]) + this->get_coefficients()[2]*(1*variables[2] + 1*variables[1]) + this->get_coefficients()[3]*(1*variables[1]*variables[2]) + this->get_coefficients()[4]*(3*variables[0]*variables[0]) + this->get_coefficients()[5]*(1*variables[2]*variables[2] + 1*variables[1]*variables[1] + 2*variables[0]*variables[1] + 2*variables[0]*variables[2]) + this->get_coefficients()[6]*(4*variables[0]*variables[0]*variables[0]) + this->get_coefficients()[7]*(2*variables[0]*variables[1]*variables[2] + 1*variables[1]*variables[2]*variables[2] + 1*variables[1]*variables[1]*variables[2]) + this->get_coefficients()[8]*(3*variables[0]*variables[0]*variables[2] + 1*variables[2]*variables[2]*variables[2] + 3*variables[0]*variables[0]*variables[1] + 1*variables[1]*variables[1]*variables[1]) + this->get_coefficients()[9]*(2*variables[0]*variables[2]*variables[2] + 2*variables[0]*variables[1]*variables[1]);
            grad[1] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(2*variables[1]) + this->get_coefficients()[2]*(1*variables[2] + 1*variables[0]) + this->get_coefficients()[3]*(1*variables[0]*variables[2]) + this->get_coefficients()[4]*(3*variables[1]*variables[1]) + this->get_coefficients()[5]*(2*variables[0]*variables[1] + 1*variables[2]*variables[2] + 1*variables[0]*variables[0] + 2*variables[1]*variables[2]) + this->get_coefficients()[6]*(4*variables[1]*variables[1]*variables[1]) + this->get_coefficients()[7]*(1*variables[0]*variables[0]*variables[2] + 1*variables[0]*variables[2]*variables[2] + 2*variables[0]*variables[1]*variables[2]) + this->get_coefficients()[8]*(1*variables[0]*variables[0]*variables[0] + 3*variables[0]*variables[1]*variables[1] + 3*variables[1]*variables[1]*variables[2] + 1*variables[2]*variables[2]*variables[2]) + this->get_coefficients()[9]*(2*variables[1]*variables[2]*variables[2] + 2*variables[0]*variables[0]*variables[1]);
            grad[2] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(2*variables[2]) + this->get_coefficients()[2]*(1*variables[1] + 1*variables[0]) + this->get_coefficients()[3]*(1*variables[0]*variables[1]) + this->get_coefficients()[4]*(3*variables[2]*variables[2]) + this->get_coefficients()[5]*(2*variables[0]*variables[2] + 2*variables[1]*variables[2] + 1*variables[1]*variables[1] + 1*variables[0]*variables[0]) + this->get_coefficients()[6]*(4*variables[2]*variables[2]*variables[2]) + this->get_coefficients()[7]*(1*variables[0]*variables[0]*variables[1] + 2*variables[0]*variables[1]*variables[2] + 1*variables[0]*variables[1]*variables[1]) + this->get_coefficients()[8]*(1*variables[0]*variables[0]*variables[0] + 3*variables[0]*variables[2]*variables[2] + 1*variables[1]*variables[1]*variables[1] + 3*variables[1]*variables[2]*variables[2]) + this->get_coefficients()[9]*(2*variables[0]*variables[0]*variables[2] + 2*variables[1]*variables[1]*variables[2]);
            return grad;
        }

        static LocalSystem get_system() {
            std::vector<AtomId*> ids;

            ids.push_back(&A);
            ids.push_back(&A);
            ids.push_back(&A);
            ConnectivityMatrix con_matrix(3);
            return LocalSystem(ids, con_matrix);
        }

        static std::vector<int> get_nl_param_indices() {
            std::vector<int> indices;

            int x_inter_AxA_0 = 0;

            indices.push_back(x_inter_AxA_0);
            indices.push_back(x_inter_AxA_0);
            indices.push_back(x_inter_AxA_0);

            return indices;
        }

};

class OW_OW_OW_OW : public Polynomial {

    public:

        OW_OW_OW_OW() : Polynomial(OW_OW_OW_OW::get_system(), "OW_OW_OW_OW", 10, 1, OW_OW_OW_OW::get_nl_param_indices()) {}

        std::vector<double> eval_terms_from_variables(const std::vector<double>& variables) const {
            std::vector<double> terms(10);
            terms[0] = variables[0] + variables[4] + variables[1] + variables[2] + variables[3] + variables[5];
            terms[1] = variables[0]*variables[5] + variables[1]*variables[4] + variables[2]*variables[3];
            terms[2] = variables[4]*variables[5] + variables[2]*variables[4] + variables[0]*variables[2] + variables[2]*variables[5] + variables[1]*variables[3] + variables[0]*variables[4] + variables[1]*variables[5] + variables[1]*variables[2] + variables[3]*variables[5] + variables[3]*variables[4] + variables[0]*variables[1] + variables[0]*variables[3];
            terms[3] = variables[4]*variables[4] + variables[3]*variables[3] + variables[1]*variables[1] + variables[5]*variables[5] + variables[2]*variables[2] + variables[0]*variables[0];
            terms[4] = variables[0]*variables[0]*variables[5] + variables[0]*variables[5]*variables[5] + variables[2]*variables[2]*variables[3] + variables[1]*variables[4]*variables[4] + variables[2]*variables[3]*variables[3] + variables[1]*variables[1]*variables[4];
            terms[5] = variables[0]*variables[0]*variables[0] + variables[4]*variables[4]*variables[4] + variables[2]*variables[2]*variables[2] + variables[1]*variables[1]*variables[1] + variables[3]*variables[3]*variables[3] + variables[5]*variables[5]*variables[5];
            terms[6] = variables[0]*variables[3]*variables[4] + variables[1]*variables[3]*variables[5] + variables[2]*variables[4]*variables[5] + variables[0]*variables[1]*variables[2];
            terms[7] = variables[0]*variables[0]*variables[3] + variables[0]*variables[0]*variables[2] + variables[1]*variables[3]*variables[3] + variables[0]*variables[1]*variables[1] + variables[0]*variables[0]*variables[1] + variables[2]*variables[2]*variables[5] + variables[1]*variables[2]*variables[2] + variables[2]*variables[4]*variables[4] + variables[0]*variables[3]*variables[3] + variables[0]*variables[2]*variables[2] + variables[3]*variables[3]*variables[4] + variables[4]*variables[4]*variables[5] + variables[1]*variables[5]*variables[5] + variables[3]*variables[4]*variables[4] + variables[3]*variables[3]*variables[5] + variables[0]*variables[0]*variables[4] + variables[3]*variables[5]*variables[5] + variables[4]*variables[5]*variables[5] + variables[1]*variables[1]*variables[5] + variables[2]*variables[2]*variables[4] + variables[2]*variables[5]*variables[5] + variables[1]*variables[1]*variables[2] + variables[0]*variables[4]*variables[4] + variables[1]*variables[1]*variables[3];
            terms[8] = variables[0]*variables[1]*variables[3] + variables[0]*variables[2]*variables[4] + variables[1]*variables[2]*variables[5] + variables[3]*variables[4]*variables[5];
            terms[9] = variables[1]*variables[2]*variables[4] + variables[1]*variables[4]*variables[5] + variables[2]*variables[3]*variables[4] + variables[2]*variables[3]*variables[5] + variables[0]*variables[2]*variables[3] + variables[0]*variables[2]*variables[5] + variables[1]*variables[2]*variables[3] + variables[0]*variables[4]*variables[5] + variables[1]*variables[3]*variables[4] + variables[0]*variables[3]*variables[5] + variables[0]*variables[1]*variables[4] + variables[0]*variables[1]*variables[5];
            return terms;
        }

        std::vector<double> polynomial_gradient(const std::vector<double>& variables) const {
            std::vector<double> grad(variables.size());
            grad[0] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(1*variables[5]) + this->get_coefficients()[2]*(1*variables[2] + 1*variables[4] + 1*variables[1] + 1*variables[3]) + this->get_coefficients()[3]*(2*variables[0]) + this->get_coefficients()[4]*(2*variables[0]*variables[5] + 1*variables[5]*variables[5]) + this->get_coefficients()[5]*(3*variables[0]*variables[0]) + this->get_coefficients()[6]*(1*variables[3]*variables[4] + 1*variables[1]*variables[2]) + this->get_coefficients()[7]*(2*variables[0]*variables[3] + 2*variables[0]*variables[2] + 1*variables[1]*variables[1] + 2*variables[0]*variables[1] + 1*variables[3]*variables[3] + 1*variables[2]*variables[2] + 2*variables[0]*variables[4] + 1*variables[4]*variables[4]) + this->get_coefficients()[8]*(1*variables[1]*variables[3] + 1*variables[2]*variables[4]) + this->get_coefficients()[9]*(1*variables[2]*variables[3] + 1*variables[2]*variables[5] + 1*variables[4]*variables[5] + 1*variables[3]*variables[5] + 1*variables[1]*variables[4] + 1*variables[1]*variables[5]);
            grad[1] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(1*variables[4]) + this->get_coefficients()[2]*(1*variables[3] + 1*variables[5] + 1*variables[2] + 1*variables[0]) + this->get_coefficients()[3]*(2*variables[1]) + this->get_coefficients()[4]*(1*variables[4]*variables[4] + 2*variables[1]*variables[4]) + this->get_coefficients()[5]*(3*variables[1]*variables[1]) + this->get_coefficients()[6]*(1*variables[3]*variables[5] + 1*variables[0]*variables[2]) + this->get_coefficients()[7]*(1*variables[3]*variables[3] + 2*variables[0]*variables[1] + 1*variables[0]*variables[0] + 1*variables[2]*variables[2] + 1*variables[5]*variables[5] + 2*variables[1]*variables[5] + 2*variables[1]*variables[2] + 2*variables[1]*variables[3]) + this->get_coefficients()[8]*(1*variables[0]*variables[3] + 1*variables[2]*variables[5]) + this->get_coefficients()[9]*(1*variables[2]*variables[4] + 1*variables[4]*variables[5] + 1*variables[2]*variables[3] + 1*variables[3]*variables[4] + 1*variables[0]*variables[4] + 1*variables[0]*variables[5]);
            grad[2] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(1*variables[3]) + this->get_coefficients()[2]*(1*variables[4] + 1*variables[0] + 1*variables[5] + 1*variables[1]) + this->get_coefficients()[3]*(2*variables[2]) + this->get_coefficients()[4]*(2*variables[2]*variables[3] + 1*variables[3]*variables[3]) + this->get_coefficients()[5]*(3*variables[2]*variables[2]) + this->get_coefficients()[6]*(1*variables[4]*variables[5] + 1*variables[0]*variables[1]) + this->get_coefficients()[7]*(1*variables[0]*variables[0] + 2*variables[2]*variables[5] + 2*variables[1]*variables[2] + 1*variables[4]*variables[4] + 2*variables[0]*variables[2] + 2*variables[2]*variables[4] + 1*variables[5]*variables[5] + 1*variables[1]*variables[1]) + this->get_coefficients()[8]*(1*variables[0]*variables[4] + 1*variables[1]*variables[5]) + this->get_coefficients()[9]*(1*variables[1]*variables[4] + 1*variables[3]*variables[4] + 1*variables[3]*variables[5] + 1*variables[0]*variables[3] + 1*variables[0]*variables[5] + 1*variables[1]*variables[3]);
            grad[3] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(1*variables[2]) + this->get_coefficients()[2]*(1*variables[1] + 1*variables[5] + 1*variables[4] + 1*variables[0]) + this->get_coefficients()[3]*(2*variables[3]) + this->get_coefficients()[4]*(1*variables[2]*variables[2] + 2*variables[2]*variables[3]) + this->get_coefficients()[5]*(3*variables[3]*variables[3]) + this->get_coefficients()[6]*(1*variables[0]*variables[4] + 1*variables[1]*variables[5]) + this->get_coefficients()[7]*(1*variables[0]*variables[0] + 2*variables[1]*variables[3] + 2*variables[0]*variables[3] + 2*variables[3]*variables[4] + 1*variables[4]*variables[4] + 2*variables[3]*variables[5] + 1*variables[5]*variables[5] + 1*variables[1]*variables[1]) + this->get_coefficients()[8]*(1*variables[0]*variables[1] + 1*variables[4]*variables[5]) + this->get_coefficients()[9]*(1*variables[2]*variables[4] + 1*variables[2]*variables[5] + 1*variables[0]*variables[2] + 1*variables[1]*variables[2] + 1*variables[1]*variables[4] + 1*variables[0]*variables[5]);
            grad[4] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(1*variables[1]) + this->get_coefficients()[2]*(1*variables[5] + 1*variables[2] + 1*variables[0] + 1*variables[3]) + this->get_coefficients()[3]*(2*variables[4]) + this->get_coefficients()[4]*(2*variables[1]*variables[4] + 1*variables[1]*variables[1]) + this->get_coefficients()[5]*(3*variables[4]*variables[4]) + this->get_coefficients()[6]*(1*variables[0]*variables[3] + 1*variables[2]*variables[5]) + this->get_coefficients()[7]*(2*variables[2]*variables[4] + 1*variables[3]*variables[3] + 2*variables[4]*variables[5] + 2*variables[3]*variables[4] + 1*variables[0]*variables[0] + 1*variables[5]*variables[5] + 1*variables[2]*variables[2] + 2*variables[0]*variables[4]) + this->get_coefficients()[8]*(1*variables[0]*variables[2] + 1*variables[3]*variables[5]) + this->get_coefficients()[9]*(1*variables[1]*variables[2] + 1*variables[1]*variables[5] + 1*variables[2]*variables[3] + 1*variables[0]*variables[5] + 1*variables[1]*variables[3] + 1*variables[0]*variables[1]);
            grad[5] = 0 + this->get_coefficients()[0]*(1) + this->get_coefficients()[1]*(1*variables[0]) + this->get_coefficients()[2]*(1*variables[4] + 1*variables[2] + 1*variables[1] + 1*variables[3]) + this->get_coefficients()[3]*(2*variables[5]) + this->get_coefficients()[4]*(1*variables[0]*variables[0] + 2*variables[0]*variables[5]) + this->get_coefficients()[5]*(3*variables[5]*variables[5]) + this->get_coefficients()[6]*(1*variables[1]*variables[3] + 1*variables[2]*variables[4]) + this->get_coefficients()[7]*(1*variables[2]*variables[2] + 1*variables[4]*variables[4] + 2*variables[1]*variables[5] + 1*variables[3]*variables[3] + 2*variables[3]*variables[5] + 2*variables[4]*variables[5] + 1*variables[1]*variables[1] + 2*variables[2]*variables[5]) + this->get_coefficients()[8]*(1*variables[1]*variables[2] + 1*variables[3]*variables[4]) + this->get_coefficients()[9]*(1*variables[1]*variables[4] + 1*variables[2]*variables[3] + 1*variables[0]*variables[2] + 1*variables[0]*variables[4] + 1*variables[0]*variables[3] + 1*variables[0]*variables[1]);
            return grad;
        }

        static LocalSystem get_system() {
            std::vector<AtomId*> ids;

            ids.push_back(&A);
            ids.push_back(&A);
            ids.push_back(&A);
            ids.push_back(&A);
            ConnectivityMatrix con_matrix(4);
            return LocalSystem(ids, con_matrix);
        }

        static std::vector<int> get_nl_param_indices() {
            std::vector<int> indices;

            int x_inter_AxA_0 = 0;

            indices.push_back(x_inter_AxA_0);
            indices.push_back(x_inter_AxA_0);
            indices.push_back(x_inter_AxA_0);
            indices.push_back(x_inter_AxA_0);
            indices.push_back(x_inter_AxA_0);
            indices.push_back(x_inter_AxA_0);

            return indices;
        }

};

void FitFuncs::add_polynomials(std::vector<Polynomial*>& polynomials) {
    polynomials.push_back(new OW_OW());
    polynomials.push_back(new OW_OW_OW());
    polynomials.push_back(new OW_OW_OW_OW());
}
