/******************************************************************************
Copyright 2019 The Regents of the University of California.
All Rights Reserved.
Permission to copy, modify and distribute any part of this Software for
educational, research and non-profit purposes, without fee, and without
a written agreement is hereby granted, provided that the above copyright
notice, this paragraph and the following three paragraphs appear in all
copies.
Those desiring to incorporate this Software into commercial products or
use for commercial purposes should contact the:
Office of Innovation & Commercialization
University of California, San Diego
9500 Gilman Drive, Mail Code 0910
La Jolla, CA 92093-0910
Ph: (858) 534-5815
FAX: (858) 534-7345
E-MAIL: invent@ucsd.edu
IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE UNIVERSITY
OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
THE SOFTWARE PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
ENHANCEMENTS, OR MODIFICATIONS. THE UNIVERSITY OF CALIFORNIA MAKES NO
REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED OR
EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE OF THE
SOFTWARE WILL NOT INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
******************************************************************************/

#include "potential/2b/OW_OW.h"
#include "potential/fragmented/FitFuncs.h"

////////////////////////////////////////////////////////////////////////////////

namespace OW_OW_namespace {

//----------------------------------------------------------------------------//

OW_OW::OW_OW(const std::string mon0, const std::string mon1) {
    int fitfuncs_index = 0;
    std::vector<Polynomial*> polynomials;

    FitFuncs::add_polynomials(polynomials);

    for(int i = 0; i < polynomials.size(); i++) {
        if(i == fitfuncs_index) {
            this->poly = polynomials[i];
        } else {
            delete polynomials[i];
        }
    }

    std::string param_file = "OW_OW.params";

    // parse nl params and coefficients

    std::ifstream poly_file(param_file.c_str());
    std::string tmp;
    std::string line1, line2;
    getline(poly_file, line1);
    getline(poly_file, line2);

    std::stringstream stream(line1 + " " + line2);
    std::vector<double> nl_params;
    std::vector<double> coefficients;

    stream >> tmp;
    stream >> tmp;

    for(int i = 0; i < poly->get_num_nl_params(); i++) {
        stream >> tmp;
        nl_params.push_back(std::stod(tmp));
    }

    stream >> tmp;
    for(int i = 0; i < poly->get_num_coefficients(); i++) {
        stream >> tmp;
        coefficients.push_back(std::stod(tmp));
    }

    poly_file.close();
    poly->set_nl_params(nl_params);
    poly->set_coefficients(coefficients);

        

}

//----------------------------------------------------------------------------//

double OW_OW::f_switch(const double& r, double& g) const {

    // DO NOT USE
    return NULL;

}

//----------------------------------------------------------------------------//

double OW_OW::eval(const double* xyz0, const double* xyz1, const size_t ndim) const {
#ifdef DEBUG
    std::cerr << "\nEntering " << __func__ << " in " << __FILE__ << std::endl;
#endif

    std::vector<double> energies;


    for(size_t j = 0; j < ndim; j++) {
        std::vector<double> coords;


        for(size_t i = 0; i < 1*3; i++)
            coords.push_back(xyz0[j*3*1 + i]);

        for(size_t i = 0; i < 1*3; i++)
            coords.push_back(xyz1[j*3*1 + i]);


        std::vector<double> distances;

        for(int atom_index1 = 0; atom_index1 < coords.size() / 3; atom_index1++) {
            for(int atom_index2 = atom_index1 + 1; atom_index2 < coords.size() / 3; atom_index2++) {
                double x1, y1, z1, x2, y2, z2;
                x1 = coords[3*atom_index1];
                y1 = coords[3*atom_index1 + 1];
                z1 = coords[3*atom_index1 + 2];
                x2 = coords[3*atom_index2];
                y2 = coords[3*atom_index2 + 1];
                z2 = coords[3*atom_index2 + 2];
                distances.push_back(std::sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)) + ((z1 - z2) * (z1 - z2))));
            }
        }
  
	std::cout<<poly->eval_switch(distances)<<std::endl;
        energies.push_back(poly->eval(distances) * poly->eval_switch(distances));

    }

    double energy = 0.0;
    for (size_t i = 0; i < ndim; i++) {
        energy += energies[i];
    }

#ifdef DEBUG
    std::cerr << std::scientific << std::setprecision(10);
    std::cerr << "\nExiting " << __func__ << " in " << __FILE__ << std::endl;

    std::cerr <<  "Input coordinates (0) for " << ndim << " monomers:\n";
    for (size_t i = 0; i < ndim; i++) {
        for(size_t j = 0; j < 1*3; j++) {
            std::cerr << xyz0[1*3 * i + j] << " , ";
        }
        std::cerr << std::endl;
    }

    std::cerr <<  "Input coordinates (1) for " << ndim << " monomers:\n";
    for (size_t i = 0; i < ndim; i++) {
        for(size_t j = 0; j < 1*3; j++) {
            std::cerr << xyz1[1*3 * i + j] << " , ";
        }
        std::cerr << std::endl;
    }

    std::cerr << "Output energy: " << energy << std::endl;
#endif

    return energy;
}

double OW_OW::eval(const double* xyz0, const double* xyz1, double* grad0, double* grad1, const size_t ndim,
                               std::vector<double>* virial) const {
#ifdef DEBUG
    std::cerr << std::scientific << std::setprecision(10);
    std::cerr << "\nEntering " << __func__ << " in " << __FILE__ << std::endl;

    std::cerr <<  "Input coordinates (0) for " << ndim << " monomers:\n";
    for (size_t i = 0; i < ndim; i++) {
        for(size_t j = 0; j < 1*3; j++) {
            std::cerr << xyz0[1*3 * i + j] << " , ";
        }
        std::cerr << std::endl;
    }

    std::cerr <<  "Input coordinates (1) for " << ndim << " monomers:\n";
    for (size_t i = 0; i < ndim; i++) {
        for(size_t j = 0; j < 1*3; j++) {
            std::cerr << xyz1[1*3 * i + j] << " , ";
        }
        std::cerr << std::endl;
    }


    std::cerr <<  "Input gradients (0) for " << ndim << " monomers:\n";
    for (size_t i = 0; i < ndim; i++) {
        for(size_t j = 0; j < 1*3; j++) {
            std::cerr << grad0[1*3 * i + j] << " , ";
        }
        std::cerr << std::endl;
    }

    std::cerr <<  "Input gradients (1) for " << ndim << " monomers:\n";
    for (size_t i = 0; i < ndim; i++) {
        for(size_t j = 0; j < 1*3; j++) {
            std::cerr << grad1[1*3 * i + j] << " , ";
        }
        std::cerr << std::endl;
    }


    std::cerr << "Input virial:\n";
    for (size_t i = 0; i < 9; i++) {
        std::cerr << (*virial)[i] << " , ";
    }
    std::cerr << std::endl;
#endif

    for (size_t j = 0; j < ndim; j++) {

        std::vector<double> coords;



        for(size_t i = 0; i < 1*3; i++)
            coords.push_back(xyz0[j*3*1 + i]);

        for(size_t i = 0; i < 1*3; i++)
            coords.push_back(xyz1[j*3*1 + i]);


        std::vector<double> grads = poly->gradient(coords, *virial);


        for(size_t i = 0; i < 1*3; i++) {
            grad0[j*3*1 + i] = grads[0];
            grads.erase(grads.begin());
        }

        for(size_t i = 0; i < 1*3; i++) {
            grad1[j*3*1 + i] = grads[0];
            grads.erase(grads.begin());
        }


    }

    double energy = eval(xyz0, xyz1, ndim);

#ifdef DEBUG
    std::cerr << std::scientific << std::setprecision(10);
    std::cerr << "\nExiting " << __func__ << " in " << __FILE__ << std::endl;
    std::cerr << "Output energy: " << energy << std::endl;

    std::cerr <<  "Output gradients (0) for " << ndim << " monomers:\n";
    for (size_t i = 0; i < ndim; i++) {
        for(size_t j = 0; j < 1*3; j++) {
            std::cerr << grad0[1*3 * i + j] << " , ";
        }
        std::cerr << std::endl;
    }

    std::cerr <<  "Output gradients (1) for " << ndim << " monomers:\n";
    for (size_t i = 0; i < ndim; i++) {
        for(size_t j = 0; j < 1*3; j++) {
            std::cerr << grad1[1*3 * i + j] << " , ";
        }
        std::cerr << std::endl;
    }

    std::cerr << "Output virial:\n";
    for (size_t i = 0; i < 9; i++) {
        std::cerr << (*virial)[i] << " , ";
    }
    std::cerr << std::endl;
#endif

    return energy;
}

}  // namespace x2b_A1B2_A1B2_deg5

////////////////////////////////////////////////////////////////////////////////
