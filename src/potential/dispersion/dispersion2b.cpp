#include "dispersion2b.h"

namespace disp {

double GetDispersion(std::string m1, std::string m2, size_t nm, bool do_grads,
                     std::vector<double> xyz1, std::vector<double> xyz2,
                     std::vector<double> &grd1, std::vector<double> &grd2) {
  // Order the two monomer names and corresponding xyz
  bool swaped = false;
  if (m2 < m1) {
    std::string tmp = std::move(m1);
    m1 = std::move(m2);
    m2 = std::move(tmp);
    std::vector<double> tmp2 = std::move(xyz1);
    xyz1 = std::move(xyz2);
    xyz2 = std::move(tmp2);
    tmp2 = std::move(grd1);
    grd1 = std::move(grd2);
    grd2 = std::move(tmp2);
    swaped = true;
  }

  size_t nat1 = 0;
  size_t nat2 = 0;

  std::vector<size_t> types1;
  std::vector<size_t> types2;

  size_t nt2 = 0;

  double disp = 0.0;
    
  std::vector<double> C6, d6;

  if (m1 == "h2o" and m2 == "h2o") {
    // Define number of atoms in each mon
    nat1 = 3;
    nat2 = 3;
    
    // Define the type of atom in each mon
    types1.push_back(0);
    types1.push_back(1);
    types1.push_back(1);

    types2.push_back(0);
    types2.push_back(1);
    types2.push_back(1);

    // Set the number of different types
    nt2 = 2;

    // Fill in (in order) the C6 and d6 coefficients
    C6.push_back(2.373212214147944e+02); // kcal/mol * A^(-6) O -- O
    C6.push_back(8.349556669872743e+01); // kcal/mol * A^(-6) O -- H
    C6.push_back(8.349556669872743e+01); // kcal/mol * A^(-6) H -- O
    C6.push_back(2.009358600184719e+01); // kcal/mol * A^(-6) H -- H

    d6.push_back(9.295485815062264e+00); // A^(-1)
    d6.push_back(9.775202425217957e+00); // A^(-1)
    d6.push_back(9.775202425217957e+00); // A^(-1)
    d6.push_back(9.406475169954112e+00); // A^(-1)

  } else if (m1 == "f" and m2 == "h2o") {
    // Define number of atoms in each mon
    nat1 = 1;
    nat2 = 3;

    // Define the type of atom in each mon
    types1.push_back(0);

    types2.push_back(0);
    types2.push_back(1);
    types2.push_back(1);

    // Set the number of different types
    nt2 = 2;

    // Fill in (in order) the C6 and d6 coefficients
    C6.push_back(3.488640000000000e+02); // kcal/mol * A^(-6) F -- O
    C6.push_back(1.286780000000000e+02); // kcal/mol * A^(-6) F -- H

    d6.push_back(3.586190000000000e+00); // A^(-1)
    d6.push_back(2.697680000000000e+00); // A^(-1)
  } else {
    return 0.0;
  }

  // Going over pairs:
  for (size_t i = 0; i < nat1; i++) {
    size_t i3 = 3*i;
    size_t ti = types1[i];
    for (size_t j = 0; j < nat2; j++) {
      size_t j3 = 3*j;
      size_t tj = types2[j];
      double c1[3*nm], c2[3*nm];
      
      // For each pair, go over all monomers
      for (size_t k = 0; k < nm; k++) {
        size_t k3 = 3*k;
        size_t nat1k3 = nat1*k3;
        size_t nat2k3 = nat2*k3;
        for (size_t l = 0; l < 3; l++) {
          c1[k3 + l] = xyz1[nat1k3 + i3 + l];
          c2[k3 + l] = xyz2[nat2k3 + j3 + l];
        }
      }
      if (!do_grads) {
        disp += disp6(C6[ti*nt2 +tj], d6[ti*nt2 +tj], c1, c2, nm);
      } else {
        double g1[3*nm], g2[3*nm];
        std::fill(g1, g1 + 3*nm, 0.0);
        std::fill(g2, g2 + 3*nm, 0.0);
        disp += disp6(C6[ti*nt2 +tj], d6[ti*nt2 +tj], c1, c2, g1, g2, nm);
        for (size_t k = 0; k < nm; k++) {
          size_t k3 = 3*k;
          size_t nat1k3 = nat1*k3;
          size_t nat2k3 = nat2*k3;
          for (size_t l = 0; l < 3; l++) {
            grd1[nat1k3 + i3 + l] += g1[k3 + l];
            grd2[nat2k3 + j3 + l] += g2[k3 + l];
          }
        }
      }
    }
  }

  if (swaped) {
    std::vector<double> tmp2 = std::move(grd2);
    grd2 = std::move(grd1);
    grd1 = std::move(tmp2);
  }
  
  
  return disp;
}


}  // namespace disp