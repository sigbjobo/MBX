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

#include "fields.h"
#include <iomanip>

namespace elec {

ElectricFieldHolder::ElectricFieldHolder(size_t n) {
    maxnmon = n;
    v0_ = std::vector<double>(maxnmon);
    v1_ = std::vector<double>(maxnmon);
    v2_ = std::vector<double>(maxnmon);
    v3_ = std::vector<double>(maxnmon);
    v4_ = std::vector<double>(maxnmon);
    v5_ = std::vector<double>(maxnmon);
    v6_ = std::vector<double>(maxnmon);
    v7_ = std::vector<double>(maxnmon);
    v8_ = std::vector<double>(maxnmon);
    v9_ = std::vector<double>(maxnmon);
    v10_ = std::vector<double>(maxnmon);
    v11_ = std::vector<double>(6 * maxnmon);
}

////////////////////////////////////////////////////////////////////////////////

void ElectricFieldHolder::CalcPermanentElecField(
    double *xyz1, double *xyz2, double *chg1, double *chg2, size_t mon1_index, size_t mon2_index_start,
    size_t mon2_index_end, size_t nmon1, size_t nmon2, size_t site_i, size_t site_j, double Ai, double Asqsqi,
    double aCC, double aCC1_4, double g34, double *Efqx_mon1, double *Efqy_mon1, double *Efqz_mon1, double *phi1,
    double *phi2, double *Efq2, double elec_scale_factor, double ewald_alpha, bool use_pbc,
    const std::vector<double> &box, const std::vector<double> &box_inverse, double cutoff, bool use_ghost,
    const std::vector<size_t> &islocal, const size_t isl1_offset, const size_t isl2_offset, size_t m2_offset,
    std::vector<double> *virial) {
    // Shifts that will be useful in the loops
    const size_t nmon12 = nmon1 * 2;
    const size_t nmon22 = nmon2 * 2;
    const size_t site_i3 = site_i * 3;
    const size_t site_j3 = site_j * 3;
    const size_t site_inmon1 = site_i * nmon1;
    const size_t site_jnmon2 = site_j * nmon2;
    const size_t site_inmon13 = nmon1 * site_i3;
    const size_t site_jnmon23 = nmon2 * site_j3;

    // Coordinates x, y and z of site i of monomer 1
    const double xyzmon1_x = xyz1[site_inmon13 + mon1_index];
    const double xyzmon1_y = xyz1[site_inmon13 + nmon1 + mon1_index];
    const double xyzmon1_z = xyz1[site_inmon13 + nmon12 + mon1_index];

    // Fill vectors with zeros in the desired range
    // temporary virial holder
    std::fill(v11_.begin(), v11_.end(), 0.0);

    const double PIQSRT = std::sqrt(M_PI);

    double v7 = 0.0;
    double v8 = 0.0;
    double v9 = 0.0;
    double v10 = 0.0;

// Store rijx, rijy and rijz in vectors
#pragma omp simd reduction(+ : v7, v8, v9, v10)
    for (size_t m = mon2_index_start; m < mon2_index_end; m++) {
        bool accum2 = false;
        if (!use_ghost) accum2 = true;
        size_t isls = islocal[isl1_offset] + islocal[m + isl2_offset + m2_offset];
        if (use_ghost && isls) accum2 = true;

        if (accum2) {
            double scale = 1.0;
            if (use_ghost && (isls == 1)) scale = 0.5;

            double rijx = xyzmon1_x - xyz2[site_jnmon23 + m];           // rijx
            double rijy = xyzmon1_y - xyz2[site_jnmon23 + nmon2 + m];   // rijy
            double rijz = xyzmon1_z - xyz2[site_jnmon23 + nmon22 + m];  // rijz

            // Apply the minimum image convention via fractional coordinates
            // It is probably a good idea to identify orthorhombic cases and write a faster version for them
            if (use_pbc) {
                // Convert to fractional coordinates
                double fracrijx = box_inverse[0] * rijx + box_inverse[3] * rijy + box_inverse[6] * rijz;
                double fracrijy = box_inverse[1] * rijx + box_inverse[4] * rijy + box_inverse[7] * rijz;
                double fracrijz = box_inverse[2] * rijx + box_inverse[5] * rijy + box_inverse[8] * rijz;

                // Put in the range 0 to 1
                fracrijx -= std::floor(fracrijx + 0.5);
                fracrijy -= std::floor(fracrijy + 0.5);
                fracrijz -= std::floor(fracrijz + 0.5);

                // Convert back to cartesian coordinates
                rijx = box[0] * fracrijx + box[3] * fracrijy + box[6] * fracrijz;
                rijy = box[1] * fracrijx + box[4] * fracrijy + box[7] * fracrijz;
                rijz = box[2] * fracrijx + box[5] * fracrijy + box[8] * fracrijz;
            }

            double v3 = rijx * rijx + rijy * rijy + rijz * rijz;  // r2

            // Store a*(r/A)^4 in vector
            double v5 = aCC * v3 * v3 * Asqsqi;  // a*(r/A)^4

            // Convert r2 -> 1/r
            v3 = 1 / std::sqrt(v3);

            // Cheesy way to apply cutoffs, for now!
            v3 *= (v3 < 1.0 / cutoff ? 0 : 1);

            // Store the attenuated coulomb operator in vector
            double v4 = (elec_scale_factor - erf(ewald_alpha / (v3 + 1e-30))) * v3;  // (1-erf(alpha r))/r

            if (!use_pbc) {
                // Rescale v3 to ensure right behavior in no PBC conditions
                v3 *= elec_scale_factor;
                v4 = v3;
            }

            // Compute gammq and store result in vector. This loop is not vectorizable
            double v6 = gammq(0.75, v5) * elec_scale_factor;  // gammq

            // Finalize computation of electric field

#if NO_THOLE
            const double exp1 = 0;
            v6 = 0;
#else
            const double exp1 = elec_scale_factor * std::exp(-v5);
#endif
            // Terms needed for the Ewald direct space field, see equation 2.8 of
            // A. Y. Toukmaji, C. Sagui, J. Board and T. A. Darden, J. Chem. Phys., 113 10913 (2000).
            const double exp_alpha2r2 = std::exp(-ewald_alpha * ewald_alpha / (v3 * v3));
            const bool use_ewald = use_pbc;
            const double ewaldterm = use_ewald ? 2 * exp_alpha2r2 * ewald_alpha / PIQSRT : 0;

            // Screening functions
            const double s1r = v4 - exp1 * v3;
            const double s0r = (s1r + aCC1_4 * Ai * g34 * v6);
            const double s1r3 = (s1r + ewaldterm) * v3 * v3;

            // Compute contribution to the field phi
            // Storing the contrib to mon 1 in vector to make it vectorizable

            // Assuming phi will be at1mon1_index at1m2 at1m3 .. for same type of mons
            v7 += scale * s0r * chg2[site_jnmon2 + m];
            phi2[site_jnmon2 + m] += scale * s0r * chg1[site_inmon1 + mon1_index];

            // Field will be as xyz xxxxyyyyzzzzat1 xxxxxyyyyzzzz at2...
            const double s1r3ci = scale * s1r3 * chg1[site_inmon1 + mon1_index];
            const double s1r3cj = scale * s1r3 * chg2[site_jnmon2 + m];

            // Compute the three components of the permanent electric field
            // Storing contributions to mon1 in vectors to make the loop vectorizable

            v8 += s1r3cj * rijx;
            v9 += s1r3cj * rijy;
            v10 += s1r3cj * rijz;

            Efq2[site_jnmon23 + m] -= s1r3ci * rijx;
            Efq2[site_jnmon23 + nmon2 + m] -= s1r3ci * rijy;
            Efq2[site_jnmon23 + nmon22 + m] -= s1r3ci * rijz;

            // update virial
            if (virial != 0) {
                double dvr = scale * chg2[site_jnmon2 + m] * chg1[site_inmon1 + mon1_index] * s1r3 * constants::COULOMB;
                double dvx = dvr * rijx;
                double dvy = dvr * rijy;
                double dvz = dvr * rijz;

                v11_[0 * maxnmon + m] = rijx * dvx;
                v11_[1 * maxnmon + m] = rijx * dvy;
                v11_[2 * maxnmon + m] = rijx * dvz;
                v11_[3 * maxnmon + m] = rijy * dvy;
                v11_[4 * maxnmon + m] = rijy * dvz;
                v11_[5 * maxnmon + m] = rijz * dvz;
            }

        }  // if(accum2)
    }

    // Add up the contributions to the mon1 site
    *phi1 = v7;
    *Efqx_mon1 = v8;
    *Efqy_mon1 = v9;
    *Efqz_mon1 = v10;

    if (virial != 0) {
        for (size_t m = mon2_index_start; m < mon2_index_end; m++) {
            // condensate virial
            (*virial)[0] += v11_[0 * maxnmon + m];
            (*virial)[1] += v11_[1 * maxnmon + m];
            (*virial)[2] += v11_[2 * maxnmon + m];

            (*virial)[4] += v11_[3 * maxnmon + m];
            (*virial)[5] += v11_[4 * maxnmon + m];

            (*virial)[8] += v11_[5 * maxnmon + m];

            (*virial)[3] = (*virial)[1];
            (*virial)[6] = (*virial)[2];
            (*virial)[7] = (*virial)[5];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ElectricFieldHolder::CalcDipoleElecField(double *xyz1, double *xyz2, double *mu1, double *mu2, size_t mon1_index,
                                              size_t mon2_index_start, size_t mon2_index_end, size_t nmon1,
                                              size_t nmon2, size_t site_i, size_t site_j, double Asqsqi, double aDD,
                                              double *Efd2, double *Efdx_mon1, double *Efdy_mon1, double *Efdz_mon1,
                                              double ewald_alpha, bool use_pbc, const std::vector<double> &box,
                                              const std::vector<double> &box_inverse, double cutoff, bool use_ghost,
                                              const std::vector<size_t> &islocal, const size_t isl1_offset,
                                              const size_t isl2_offset) {
    // Shifts that will be useful in the loops
    const size_t nmon12 = nmon1 * 2;
    const size_t nmon22 = nmon2 * 2;
    const size_t site_i3 = site_i * 3;
    const size_t site_j3 = site_j * 3;
    const size_t site_inmon13 = nmon1 * site_i3;
    const size_t site_jnmon23 = nmon2 * site_j3;

    // Coordinates x, y and z of site i of monomer 1
    const double xyzmon1_x = xyz1[site_inmon13 + mon1_index];
    const double xyzmon1_y = xyz1[site_inmon13 + nmon1 + mon1_index];
    const double xyzmon1_z = xyz1[site_inmon13 + nmon12 + mon1_index];

    double v0 = 0.0;
    double v1 = 0.0;
    double v2 = 0.0;

    double alpha_pi_term = ewald_alpha == 0 ? 0 : 1 / (std::sqrt(M_PI) * ewald_alpha);
    double two_alpha_squared = 2.0 * ewald_alpha * ewald_alpha;
    alpha_pi_term *= two_alpha_squared;

    const double cutoffsq = cutoff * cutoff;
#pragma omp simd reduction(+ : v0, v1, v2)
    for (size_t m = mon2_index_start; m < mon2_index_end; m++) {
        bool accum2 = !use_ghost;
        size_t isls = islocal[isl1_offset] + islocal[m + isl2_offset];
        if (use_ghost && isls) accum2 = true;

        if (accum2) {
            double scale = (use_ghost && (isls == 1)) ? 0.5 : 1.0;

            // Distances between sites i and j from mon1 and mon2
            double rijx = xyzmon1_x - xyz2[site_jnmon23 + m];
            double rijy = xyzmon1_y - xyz2[site_jnmon23 + nmon2 + m];
            double rijz = xyzmon1_z - xyz2[site_jnmon23 + nmon22 + m];

            // Apply the minimum image convention via fractional coordinates
            // It is probably a good idea to identify orthorhombic cases and write a faster version for them
            if (use_pbc) {
                // Convert to fractional coordinates
                double fracrijx = box_inverse[0] * rijx + box_inverse[3] * rijy + box_inverse[6] * rijz;
                double fracrijy = box_inverse[1] * rijx + box_inverse[4] * rijy + box_inverse[7] * rijz;
                double fracrijz = box_inverse[2] * rijx + box_inverse[5] * rijy + box_inverse[8] * rijz;
                // Put in the range 0 to 1
                fracrijx -= std::floor(fracrijx + 0.5);
                fracrijy -= std::floor(fracrijy + 0.5);
                fracrijz -= std::floor(fracrijz + 0.5);
                // Convert back to Cartesian coordinates
                rijx = box[0] * fracrijx + box[3] * fracrijy + box[6] * fracrijz;
                rijy = box[1] * fracrijx + box[4] * fracrijy + box[7] * fracrijz;
                rijz = box[2] * fracrijx + box[5] * fracrijy + box[8] * fracrijz;
            }

            const double rsq = rijx * rijx + rijy * rijy + rijz * rijz;
            if (rsq < cutoffsq) {
                const double ri = 1 / sqrt(rsq);
                const double risq = ri * ri;

                // Now build the Ewald generalization of the Coulomb operator and its derivatives, see
                // Toukmaji, Sagui, Board, and Darden, JCP, 113 10913 (2000)
                // particularly equations 2.8 and 2.9.  When alpha is zero these fall out to just be
                // r^-1, r^-3, r^-5
                double r_alpha = ewald_alpha * sqrt(rsq);
                double exp_alpha2_r2 = exp(-r_alpha * r_alpha);
                double bn1 = (erfc(r_alpha) * ri + alpha_pi_term * exp_alpha2_r2) * risq;
                double bn2 = (3 * bn1 + alpha_pi_term * two_alpha_squared * exp_alpha2_r2) * risq;

                // Some values that will be used in the screening functions
                const double rA4 = rsq * rsq * Asqsqi;
// TODO look at the exponential function intel vec
#if NO_THOLE
                const double exp1 = 0;
#else
                const double exp1 = std::exp(-aDD * rA4);
#endif

                // Get screening functions
                const double s1r3 = scale * (bn1 - exp1 * ri * risq);
                const double s2r5_3 = scale * (bn2 - (3 + 4 * aDD * rA4) * exp1 * ri * risq * risq);
                const double ts2x = s2r5_3 * rijx;
                const double ts2y = s2r5_3 * rijy;
                const double ts2z = s2r5_3 * rijz;

                // Contributions to the dipole electric field to site i of mon1
                // Stored in vectors to make the loop vectorizable

                // Component x
                v0 += (ts2x * rijx - s1r3) * mu2[site_jnmon23 + m] + ts2x * rijy * mu2[site_jnmon23 + nmon2 + m] +
                      ts2x * rijz * mu2[site_jnmon23 + nmon22 + m];

                // Component y
                v1 += ts2y * rijx * mu2[site_jnmon23 + m] + (ts2y * rijy - s1r3) * mu2[site_jnmon23 + nmon2 + m] +
                      ts2y * rijz * mu2[site_jnmon23 + nmon22 + m];

                // Component z
                v2 += ts2z * rijx * mu2[site_jnmon23 + m] + ts2z * rijy * mu2[site_jnmon23 + nmon2 + m] +
                      (ts2z * rijz - s1r3) * mu2[site_jnmon23 + nmon22 + m];

                // Contributions to the dipole electric field to site j of mon2
                // Component x
                Efd2[site_jnmon23 + m] += (ts2x * rijx - s1r3) * mu1[site_inmon13 + mon1_index] +
                                          ts2x * rijy * mu1[site_inmon13 + nmon1 + mon1_index] +
                                          ts2x * rijz * mu1[site_inmon13 + nmon12 + mon1_index];

                // Component y
                Efd2[site_jnmon23 + nmon2 + m] += (ts2y * rijx) * mu1[site_inmon13 + mon1_index] +
                                                  (ts2y * rijy - s1r3) * mu1[site_inmon13 + nmon1 + mon1_index] +
                                                  ts2y * rijz * mu1[site_inmon13 + nmon12 + mon1_index];

                // Component z
                Efd2[site_jnmon23 + nmon22 + m] += (ts2z * rijx) * mu1[site_inmon13 + mon1_index] +
                                                   ts2z * rijy * mu1[site_inmon13 + nmon1 + mon1_index] +
                                                   (ts2z * rijz - s1r3) * mu1[site_inmon13 + nmon12 + mon1_index];
            }  // if(r<cutoff)
        }      // if(accum2)
    }

    // Setting the values to the output
    *Efdx_mon1 = v0;
    *Efdy_mon1 = v1;
    *Efdz_mon1 = v2;
}

////////////////////////////////////////////////////////////////////////////////

void ElectricFieldHolder::CalcElecFieldGrads(
    double *xyz1, double *xyz2, double *chg1, double *chg2, double *mu1, double *mu2, size_t mon1_index,
    size_t mon2_index_start, size_t mon2_index_end, size_t nmon1, size_t nmon2, size_t site_i, size_t site_j,
    double aDD, double aCD, double Asqsqi, double *grdx, double *grdy, double *grdz, double *phi1, double *phi2,
    double *grd2, double elec_scale_factor, double ewald_alpha, bool use_pbc, const std::vector<double> &box,
    const std::vector<double> &box_inverse, double cutoff, bool use_ghost, const std::vector<size_t> &islocal,
    const size_t isl1_offset, const size_t isl2_offset, std::vector<double> *virial) {
    // Shifts that will be useful in the loops
    const size_t nmon12 = nmon1 * 2;
    const size_t nmon22 = nmon2 * 2;
    const size_t site_i3 = site_i * 3;
    const size_t site_j3 = site_j * 3;
    const size_t site_inmon1 = site_i * nmon1;
    const size_t site_jnmon2 = site_j * nmon2;
    const size_t site_inmon13 = nmon1 * site_i3;
    const size_t site_jnmon23 = nmon2 * site_j3;

    // Coordinates x, y and z of site i of monomer 1
    const double xyzmon1_x = xyz1[site_inmon13 + mon1_index];
    const double xyzmon1_y = xyz1[site_inmon13 + nmon1 + mon1_index];
    const double xyzmon1_z = xyz1[site_inmon13 + nmon12 + mon1_index];

    // Fill vectors with zeros in the desired range
    std::fill(v0_.begin() + mon2_index_start, v0_.begin() + mon2_index_end, 0.0);
    std::fill(v1_.begin() + mon2_index_start, v1_.begin() + mon2_index_end, 0.0);
    std::fill(v2_.begin() + mon2_index_start, v2_.begin() + mon2_index_end, 0.0);
    std::fill(v3_.begin() + mon2_index_start, v3_.begin() + mon2_index_end, 0.0);
    std::fill(v11_.begin(), v11_.end(), 0.0);  // holders for the virial during vectorized loop

#pragma omp simd
    for (size_t m = mon2_index_start; m < mon2_index_end; m++) {
        bool accum2 = false;
        if (!use_ghost) accum2 = true;
        size_t isls = islocal[isl1_offset] + islocal[m + isl2_offset];
        if (use_ghost && isls) accum2 = true;

        if (accum2) {
            double scale = 1.0;
            if (use_ghost && (isls == 1)) scale = 0.5;

            // Distances between sites i and j from mon1 and mon2
            const double rawrijx = xyzmon1_x - xyz2[site_jnmon23 + m];
            const double rawrijy = xyzmon1_y - xyz2[site_jnmon23 + nmon2 + m];
            const double rawrijz = xyzmon1_z - xyz2[site_jnmon23 + nmon22 + m];

            // Apply the minimum image convention via fractional coordinates
            // It is probably a good idea to identify orthorhombic cases and write a faster version for them
            double minrijx, minrijy, minrijz;
            if (use_pbc) {
                // Convert to fractional coordinates
                const double fracrijx = box_inverse[0] * rawrijx + box_inverse[3] * rawrijy + box_inverse[6] * rawrijz;
                const double fracrijy = box_inverse[1] * rawrijx + box_inverse[4] * rawrijy + box_inverse[7] * rawrijz;
                const double fracrijz = box_inverse[2] * rawrijx + box_inverse[5] * rawrijy + box_inverse[8] * rawrijz;
                // Put in the range 0 to 1
                const double minfracrijx = fracrijx - std::floor(fracrijx + 0.5);
                const double minfracrijy = fracrijy - std::floor(fracrijy + 0.5);
                const double minfracrijz = fracrijz - std::floor(fracrijz + 0.5);
                // Convert back to Cartesian coordinates
                minrijx = box[0] * minfracrijx + box[3] * minfracrijy + box[6] * minfracrijz;
                minrijy = box[1] * minfracrijx + box[4] * minfracrijy + box[7] * minfracrijz;
                minrijz = box[2] * minfracrijx + box[5] * minfracrijy + box[8] * minfracrijz;
            }
            const double rijx = use_pbc ? minrijx : rawrijx;
            const double rijy = use_pbc ? minrijy : rawrijy;
            const double rijz = use_pbc ? minrijz : rawrijz;

            const double rijx2 = rijx * rijx;
            const double rijy2 = rijy * rijy;
            const double rijz2 = rijz * rijz;
            const double rsq = rijx2 + rijy2 + rijz2;
            const double r = std::sqrt(rsq);
            const double ri = r < cutoff ? 1 / r : 0;
            const double risq = ri * ri;
            const double rsqsq = rsq * rsq;
            // Some values that will be used in the screening functions
            const double rA4 = rsqsq * Asqsqi;
            // TODO look at the exponential function intel vec

            const double adrA4 = aDD * 4.0 * rA4;
            const double acrA4 = aCD * 4.0 * rA4;
#if NO_THOLE
            const double exp1d = 0;
            const double exp1c = 0;
#else
            const double exp1d = std::exp(-aDD * rA4);
            const double exp1c = elec_scale_factor * std::exp(-aCD * rA4);
#endif

            // Now build the Ewald generalization of the Coulomb operator and its derivatives, see
            // Toukmaji, Sagui, Board, and Darden, JCP, 113 10913 (2000)
            // particularly equations 2.8 and 2.9.  When alpha is zero these fall out to just be
            // r^-1, r^-3, r^-5
            double r_alpha = ewald_alpha * r;
            double alpha_pi_term = ewald_alpha == 0 ? 0 : 1 / (std::sqrt(M_PI) * ewald_alpha);
            double exp_alpha2_r2 = exp(-r_alpha * r_alpha);
            double two_alpha_squared = 2.0 * ewald_alpha * ewald_alpha;
            double erfterm = erf(r_alpha);
            double bn0 = (1 - erfterm) * ri;
            double bn0_cd = (elec_scale_factor - erfterm) * ri;
            alpha_pi_term *= two_alpha_squared;
            double bn1 = (bn0 + alpha_pi_term * exp_alpha2_r2) * risq;
            double bn1_cd = (bn0_cd + alpha_pi_term * exp_alpha2_r2) * risq;
            alpha_pi_term *= two_alpha_squared;
            double bn2 = (3 * bn1 + alpha_pi_term * exp_alpha2_r2) * risq;
            double bn2_cd = (3 * bn1_cd + alpha_pi_term * exp_alpha2_r2) * risq;
            alpha_pi_term *= two_alpha_squared;
            double bn3 = (5 * bn2 + alpha_pi_term * exp_alpha2_r2) * risq;

            // Get screening functions - old version to be untouched and then deleted
            const double s2r5_3d = bn2 - (3 + adrA4) * exp1d * ri * risq * risq;

            const double s3r7_15d = bn3 - (15 + 4 * adrA4 + adrA4 * adrA4) * exp1d * risq * risq * ri * risq;
            const double s3r7_15x2 = s3r7_15d * rijx2;
            const double s3r7_15y2 = s3r7_15d * rijy2;
            const double s3r7_15z2 = s3r7_15d * rijz2;
            const double s1r3c = bn1_cd - exp1c * ri * risq;
            const double rxs1r3c = rijx * s1r3c;
            const double rys1r3c = rijy * s1r3c;
            const double rzs1r3c = rijz * s1r3c;
            const double s2r5_3c = bn2_cd - (3 + acrA4) * exp1c * risq * ri * risq;
            // Tensors
            const double ts2x = s2r5_3c * rijx;
            const double ts2y = s2r5_3c * rijy;
            const double ts2z = s2r5_3c * rijz;

            // T_alpha_beta_gamma tensor
            const double t3_0 = s3r7_15x2 * rijx - s2r5_3d * 3.0 * rijx;         // x x x
            const double t3_1 = s3r7_15x2 * rijy - s2r5_3d * rijy;               // x x y
            const double t3_2 = s3r7_15x2 * rijz - s2r5_3d * rijz;               // x x z
            const double t3_3 = s3r7_15y2 * rijx - s2r5_3d * rijx;               // x y y
            const double t3_4 = s3r7_15d * rijx * rijy * rijz;                   // x y z
            const double t3_5 = s3r7_15z2 * rijx - s2r5_3d * rijx;               // x z z
            const double t3_6 = s3r7_15d * rijy2 * rijy - s2r5_3d * 3.0 * rijy;  // y y y
            const double t3_7 = s3r7_15y2 * rijz - s2r5_3d * rijz;               // y y z
            const double t3_8 = s3r7_15z2 * rijy - s2r5_3d * rijy;               // y z z
            const double t3_9 = s3r7_15z2 * rijz - s2r5_3d * 3.0 * rijz;         // z z z

            // T_alpha_beta tensor
            const double t2_0 = ts2x * rijx - s1r3c;  // alpha = x, beta = x
            const double t2_1 = ts2x * rijy;
            const double t2_2 = ts2x * rijz;
            const double t2_3 = ts2y * rijy - s1r3c;
            const double t2_4 = ts2y * rijz;
            const double t2_5 = ts2z * rijz - s1r3c;

            // Charge times the dipole component
            const double ci_mjx = chg1[site_inmon1 + mon1_index] * mu2[site_jnmon23 + m];
            const double cj_mix = chg2[site_jnmon2 + m] * mu1[site_inmon13 + mon1_index];
            const double ci_mjy = chg1[site_inmon1 + mon1_index] * mu2[site_jnmon23 + nmon2 + m];
            const double cj_miy = chg2[site_jnmon2 + m] * mu1[site_inmon13 + nmon1 + mon1_index];
            const double ci_mjz = chg1[site_inmon1 + mon1_index] * mu2[site_jnmon23 + nmon22 + m];
            const double cj_miz = chg2[site_jnmon2 + m] * mu1[site_inmon13 + nmon12 + mon1_index];

            // Dipole times dipole
            const double mu1xmu2x = mu1[site_inmon13 + mon1_index] * mu2[site_jnmon23 + m];
            const double mu1xmu2y = mu1[site_inmon13 + mon1_index] * mu2[site_jnmon23 + nmon2 + m];
            const double mu1xmu2z = mu1[site_inmon13 + mon1_index] * mu2[site_jnmon23 + nmon22 + m];
            const double mu1ymu2x = mu1[site_inmon13 + nmon1 + mon1_index] * mu2[site_jnmon23 + m];
            const double mu1ymu2y = mu1[site_inmon13 + nmon1 + mon1_index] * mu2[site_jnmon23 + nmon2 + m];
            const double mu1ymu2z = mu1[site_inmon13 + nmon1 + mon1_index] * mu2[site_jnmon23 + nmon22 + m];
            const double mu1zmu2x = mu1[site_inmon13 + nmon12 + mon1_index] * mu2[site_jnmon23 + m];
            const double mu1zmu2y = mu1[site_inmon13 + nmon12 + mon1_index] * mu2[site_jnmon23 + nmon2 + m];
            const double mu1zmu2z = mu1[site_inmon13 + nmon12 + mon1_index] * mu2[site_jnmon23 + nmon22 + m];

            // Gradients Charge-Dipole
            // Site site_i
            // Note. Sign changed
            v0_[m] = scale * ((cj_mix - ci_mjx) * t2_0      // x x
                              + (cj_miy - ci_mjy) * t2_1    // x y
                              + (cj_miz - ci_mjz) * t2_2);  // x z
            v1_[m] = scale * ((cj_mix - ci_mjx) * t2_1      // y x
                              + (cj_miy - ci_mjy) * t2_3    // y y
                              + (cj_miz - ci_mjz) * t2_4);  // y z
            v2_[m] = scale * ((cj_mix - ci_mjx) * t2_2      // z x
                              + (cj_miy - ci_mjy) * t2_4    // z y
                              + (cj_miz - ci_mjz) * t2_5);  // z z

            // Site site_j
            grd2[site_jnmon23 + m] -= v0_[m];
            grd2[site_jnmon23 + nmon2 + m] -= v1_[m];
            grd2[site_jnmon23 + nmon22 + m] -= v2_[m];

            // Update field
            // Site site_i
            v3_[m] = scale * (rxs1r3c * mu2[site_jnmon23 + m] + rys1r3c * mu2[site_jnmon23 + nmon2 + m] +
                              rzs1r3c * mu2[site_jnmon23 + nmon22 + m]);

            // Site site_j
            phi2[site_jnmon2 + m] -=
                scale * (rxs1r3c * mu1[site_inmon13 + mon1_index] + rys1r3c * mu1[site_inmon13 + nmon1 + mon1_index] +
                         rzs1r3c * mu1[site_inmon13 + nmon12 + mon1_index]);

            // Gradients Dipole-Dipole
#if DIRECT_ONLY
            const double gx = 0;
            const double gy = 0;
            const double gz = 0;
#else
            const double gx = mu1xmu2x * t3_0     // x x x
                              + mu1xmu2y * t3_1   // x x y
                              + mu1xmu2z * t3_2   // x x z
                              + mu1ymu2x * t3_1   // x y x
                              + mu1ymu2y * t3_3   // x y y
                              + mu1ymu2z * t3_4   // x y z
                              + mu1zmu2x * t3_2   // x z x
                              + mu1zmu2y * t3_4   // x z y
                              + mu1zmu2z * t3_5;  // x z z

            const double gy = mu1xmu2x * t3_1     // y x x
                              + mu1xmu2y * t3_3   // y x y
                              + mu1xmu2z * t3_4   // y x z
                              + mu1ymu2x * t3_3   // y y x
                              + mu1ymu2y * t3_6   // y y y
                              + mu1ymu2z * t3_7   // y y z
                              + mu1zmu2x * t3_4   // y z x
                              + mu1zmu2y * t3_7   // y z y
                              + mu1zmu2z * t3_8;  // y z z

            const double gz = mu1xmu2x * t3_2     // z x x
                              + mu1xmu2y * t3_4   // z x y
                              + mu1xmu2z * t3_5   // z x z
                              + mu1ymu2x * t3_4   // z y x
                              + mu1ymu2y * t3_7   // z y y
                              + mu1ymu2z * t3_8   // z y z
                              + mu1zmu2x * t3_5   // z z x
                              + mu1zmu2y * t3_8   // z z y
                              + mu1zmu2z * t3_9;  // z z z
#endif
            // Site site_i
            v0_[m] += scale * gx;
            v1_[m] += scale * gy;
            v2_[m] += scale * gz;

            // Site site_j
            grd2[site_jnmon23 + m] -= scale * gx;
            grd2[site_jnmon23 + nmon2 + m] -= scale * gy;
            grd2[site_jnmon23 + nmon22 + m] -= scale * gz;

            // update virial
            if (virial != 0) {
                v11_[0 * maxnmon + m] = -rijx * v0_[m] * constants::COULOMB;
                v11_[1 * maxnmon + m] = -rijx * v1_[m] * constants::COULOMB;
                v11_[2 * maxnmon + m] = -rijx * v2_[m] * constants::COULOMB;

                v11_[3 * maxnmon + m] = -rijy * v1_[m] * constants::COULOMB;
                v11_[4 * maxnmon + m] = -rijy * v2_[m] * constants::COULOMB;

                v11_[5 * maxnmon + m] = -rijz * v2_[m] * constants::COULOMB;
            }
        }  // if(accum2)
    }

    // Compress vectors to double
    *grdx = 0.0;
    *grdy = 0.0;
    *grdz = 0.0;
    *phi1 = 0.0;
    for (size_t m = mon2_index_start; m < mon2_index_end; m++) {
        *grdx += v0_[m];
        *grdy += v1_[m];
        *grdz += v2_[m];
        *phi1 += v3_[m];
        // condensate virial
        if (virial != 0) {
            (*virial)[0] += v11_[0 * maxnmon + m];
            (*virial)[1] += v11_[1 * maxnmon + m];
            (*virial)[2] += v11_[2 * maxnmon + m];

            (*virial)[4] += v11_[3 * maxnmon + m];
            (*virial)[5] += v11_[4 * maxnmon + m];

            (*virial)[8] += v11_[5 * maxnmon + m];

            (*virial)[3] = (*virial)[1];
            (*virial)[6] = (*virial)[2];
            (*virial)[7] = (*virial)[5];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace elec

////////////////////////////////////////////////////////////////////////////////
