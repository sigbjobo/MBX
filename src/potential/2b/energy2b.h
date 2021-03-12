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

#ifndef ENERGY2B_H
#define ENERGY2B_H

#include <string>
#include <vector>
#include <iostream>

// 2B
#include "potential/2b/x2b-v9x.h"
#include "potential/2b/x2b-A1B2Z2_C-v1x.h"
// =====>> BEGIN SECTION INCLUDE2B <<=====
// ===>> PASTE YOUR CODE BELOW <<===
#include "potential/2b/x2b_A1B4_A1B4_deg4_exp0_v1x.h"
#include "potential/2b/x2b_A1B2Z2_C1D4_deg3_exp0_v1x.h"
#include "potential/2b/x2b_A1B2_A1B2_v1x.h"
#include "potential/2b/x2b_A1B2Z2_C1D2_v1x.h"
#include "potential/2b/OW_OW.h"

// =====>> END SECTION INCLUDE2B <<=====

/**
 * @file energy2b.h
 * @brief Contains the calls to the two-body energy with and without gradients
 */

/**
 * @namespace e2b
 * @brief Sets the namespace for the energy calls for the 2 body energy
 */
namespace e2b {

/**
 * @brief Gets the two body energy for a given set of dimers
 *
 * Given the monomer ids of the dimer
 * and the number of monomers, will return the total sum
 * of the 2b energy of those dimers. A dimer will be mon1-mon2.
 * @param[in] m1 Monomer 1 id
 * @param[in] m2 Monomer 2 id
 * @param[in] nm number of monomers (both m1 and m2 need to match)
 * @param[in] xyz1 coordinates of the monomer 1
 * @param[in] xyz2 coordinates of the monomer 2
 * @return Sum of the two-body energies of all the dimers passed as arguments
 */
double get_2b_energy(std::string m1, std::string m2, size_t nm, std::vector<double> xyz1, std::vector<double> xyz2);

/**
 * @brief Gets the two body energy for a given set of dimers
 *
 * Given the monomer ids of the dimer
 * and the number of monomers, will return the total sum
 * of the 2b energy of those dimers. A dimer will be mon1-mon2.
 * @param[in] m1 Monomer 1 id
 * @param[in] m2 Monomer 2 id
 * @param[in] nm number of monomers (both m1 and m2 need to match)
 * @param[in] xyz1 coordinates of the monomer 1
 * @param[in] xyz2 coordinates of the monomer 2
 * @param[in,out] grad1 gradients of the monomer 1. Gradients will be updated
 * @param[in,out] grad2 gradients of the monomer 2. Gradients will be updated
 * @param[in,out] virial. Virial will be updated
 * @return Sum of the two-body energies of all the dimers passed as arguments
 */
double get_2b_energy(std::string m1, std::string m2, size_t nm, std::vector<double> xyz1, std::vector<double> xyz2,
                     std::vector<double> &grad1, std::vector<double> &grad2, std::vector<double> *virial = 0);

 const OW_OW_namespace::OW_OW pot_ow_ow("ow", "ow");

}  // namespace e2b
#endif
