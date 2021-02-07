#include "mbnrg_1b_A1B3_deg5_v1.h"

////////////////////////////////////////////////////////////////////////////////

namespace mbnrg_A1B3_deg5 {

mbnrg_A1B3_deg5_v1::mbnrg_A1B3_deg5_v1(const std::string mon1) {
    // =====>> BEGIN SECTION CONSTRUCTOR <<=====
    // =>> PASTE RIGHT BELOW THIS LINE <==

    if (mon1 == "nh3") {
        coefficients = std::vector<double>{1.961327776767162e+03,   // 0
                                           -4.647660981268293e+02,  // 1
                                           4.912065936167311e+02,   // 2
                                           8.870964103725303e+02,   // 3
                                           -3.078053235492915e+02,  // 4
                                           -9.725393208450128e+02,  // 5
                                           1.042654137509166e+03,   // 6
                                           -1.988406022746139e+03,  // 7
                                           -2.732361676210664e+03,  // 8
                                           9.738849816506524e+02,   // 9
                                           -1.022338996816570e+02,  // 10
                                           1.079961826556835e+03,   // 11
                                           5.591580581591958e+02,   // 12
                                           3.045265996363357e+02,   // 13
                                           1.607372483606588e+02,   // 14
                                           -1.238931348926434e+03,  // 15
                                           2.375150117394387e+02,   // 16
                                           -1.129883233799168e+03,  // 17
                                           6.397533281591905e+02,   // 18
                                           -6.648945099003804e+02,  // 19
                                           4.533143020473243e+02,   // 20
                                           -4.757884093872732e+02,  // 21
                                           -3.550729903959455e+02,  // 22
                                           -1.855667360879671e+02,  // 23
                                           -8.436784784400273e+02,  // 24
                                           7.100990489370147e+02,   // 25
                                           9.770087223013893e+02,   // 26
                                           6.321497989011543e+00,   // 27
                                           -2.859520341376873e+02,  // 28
                                           2.640722983125067e+02,   // 29
                                           -8.194231449358879e+02,  // 30
                                           -2.155927812730570e+03,  // 31
                                           1.854997073802514e+02,   // 32
                                           -1.255834698024808e+03,  // 33
                                           2.407116985723990e+01,   // 34
                                           -7.301608271489379e+02,  // 35
                                           -9.880750434555304e+02,  // 36
                                           -1.264214889406122e+03,  // 37
                                           1.631478363279926e+03,   // 38
                                           -8.273333351786130e+00,  // 39
                                           3.304764264318333e+02,   // 40
                                           2.243015601238309e+03,   // 41
                                           -1.124047127276311e+02,  // 42
                                           -1.183819432661426e+03,  // 43
                                           -2.373032689615990e+02,  // 44
                                           1.196207299866592e+03,   // 45
                                           2.842980889209292e+02,   // 46
                                           1.517369157885648e+02,   // 47
                                           1.170377273485258e+03,   // 48
                                           7.634777394285536e+02,   // 49
                                           -6.926387211280016e+02,  // 50
                                           6.172043763422173e+02,   // 51
                                           -2.744551348417594e+02,  // 52
                                           8.767109783515691e+02,   // 53
                                           2.528481678295148e+02,   // 54
                                           9.006549851320649e+01,   // 55
                                           1.342792695276735e+03,   // 56
                                           -1.747758011470426e+03,  // 57
                                           8.806071553996981e+02,   // 58
                                           1.069469542891833e+03,   // 59
                                           -8.823386036738342e+02,  // 60
                                           1.576242054108609e+02,   // 61
                                           -6.882801619705290e+02,  // 62
                                           -4.781093703016188e+02,  // 63
                                           -6.712950402475369e+02,  // 64
                                           5.326129013691987e+01,   // 65
                                           6.021726047637742e+02,   // 66
                                           -5.527352675096699e+02,  // 67
                                           4.598625765315025e+02,   // 68
                                           1.049741451195215e+02,   // 69
                                           -1.315809350879695e+03,  // 70
                                           4.904472127153100e+02,   // 71
                                           -1.212910021561671e+02,  // 72
                                           -9.122383533929444e+02,  // 73
                                           5.169794373395308e+02,   // 74
                                           1.894030056326287e+03,   // 75
                                           -8.012406678189270e+02,  // 76
                                           -8.463938366420407e+02,  // 77
                                           8.841696450246126e+02,   // 78
                                           1.382099585200515e+03,   // 79
                                           -7.333253857472941e+01,  // 80
                                           -5.123587491434187e+02,  // 81
                                           1.332412207531990e+02,   // 82
                                           4.681133990969122e+02,   // 83
                                           -1.820278335519936e+03,  // 84
                                           -7.534047668634139e+02,  // 85
                                           4.247752504604114e+02,   // 86
                                           -8.950836859302690e+02,  // 87
                                           1.036411620108930e+03,   // 88
                                           1.600375469597167e+03,   // 89
                                           1.618104797269819e+03,   // 90
                                           -8.572128993658885e+01,  // 91
                                           -1.158063036797459e+02,  // 92
                                           -9.883611121213838e+02,  // 93
                                           -5.751170398604190e+02,  // 94
                                           -6.254967598427901e+01,  // 95
                                           6.775279122825178e+02,   // 96
                                           1.044603464883941e+03,   // 97
                                           -4.010423627087533e+02,  // 98
                                           -5.063121554294690e+02,  // 99
                                           -3.220457282257753e+03,  // 100
                                           2.495866573366807e+03};  // 101
        m_k_x_intra_A_B_1 = 7.369614879454734e-01;                  // A^(-1))
        m_k_x_intra_B_B_1 = 3.157211905474817e-01;                  // A^(-1))
        m_ri = 5.000000000000000e+00;                               // A
        m_ro = 6.500000000000000e+00;                               // A

    }  // end if mon1 == "nh3"

    // =====>> END SECTION CONSTRUCTOR <<=====
}

//----------------------------------------------------------------------------//

double mbnrg_A1B3_deg5_v1::f_switch(const double r, double& g) {
    if (r > m_ro) {
        g = 0.0;
        return 0.0;
    } else if (r > m_ri) {
        const double t1 = M_PI / (m_ro - m_ri);
        const double x = (r - m_ri) * t1;
        g = -std::sin(x) * t1 / 2.0;
        return (1.0 + std::cos(x)) / 2.0;
    } else {
        g = 0.0;
        return 1.0;
    }
}

//----------------------------------------------------------------------------//

std::vector<double> mbnrg_A1B3_deg5_v1::eval(const double* xyz1, const size_t n) {
    std::vector<double> energies(n, 0.0);
    std::vector<double> energies_sw(n, 0.0);

    std::vector<double> xyz(12);
    double sw = 0.0;
    polynomial my_poly;

    for (size_t j = 0; j < n; j++) {
        const double* mon1 = xyz1 + 12 * j;

        if (false) {
            continue;
        }

        std::copy(mon1, mon1 + 12, xyz.begin() + 0);

        const double* coords_A_1_a = xyz.data() + 0;

        const double* coords_B_1_a = xyz.data() + 3;

        const double* coords_B_2_a = xyz.data() + 6;

        const double* coords_B_3_a = xyz.data() + 9;

        double w12 = -9.721486914088159e-02;  // from MBpol
        double w13 = -9.721486914088159e-02;
        double wcross = 9.859272078406150e-02;

        variable vs[6];

        double xs[6];

        xs[0] = vs[0].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_1_a);
        xs[1] = vs[1].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_2_a);
        xs[2] = vs[2].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_3_a);
        xs[3] = vs[3].v_exp(m_k_x_intra_B_B_1, coords_B_1_a, coords_B_2_a);
        xs[4] = vs[4].v_exp(m_k_x_intra_B_B_1, coords_B_1_a, coords_B_3_a);
        xs[5] = vs[5].v_exp(m_k_x_intra_B_B_1, coords_B_2_a, coords_B_3_a);

        sw = 1.0;

        energies[j] = my_poly.eval(xs, coefficients.data());
        energies_sw[j] = energies[j] * sw;
    }
    return energies_sw;
}

//----------------------------------------------------------------------------//

std::vector<double> mbnrg_A1B3_deg5_v1::eval(const double* xyz1, double* grad1, const size_t n,
                                             std::vector<double>* virial) {
    std::vector<double> energies(n, 0.0);
    std::vector<double> energies_sw(n, 0.0);

    std::vector<double> xyz(12);
    double sw = 0.0;
    polynomial my_poly;

    for (size_t j = 0; j < n; j++) {
        const double* mon1 = xyz1 + 12 * j;

        if (false) {
            continue;
        }

        std::vector<double> gradients(12, 0.0);

        std::copy(mon1, mon1 + 12, xyz.begin() + 0);
        const double* coords_A_1_a = xyz.data() + 0;

        const double* coords_B_1_a = xyz.data() + 3;

        const double* coords_B_2_a = xyz.data() + 6;

        const double* coords_B_3_a = xyz.data() + 9;

        double* coords_A_1_a_g = gradients.data() + 0;

        double* coords_B_1_a_g = gradients.data() + 3;

        double* coords_B_2_a_g = gradients.data() + 6;

        double* coords_B_3_a_g = gradients.data() + 9;

        double w12 = -9.721486914088159e-02;  // from MBpol
        double w13 = -9.721486914088159e-02;
        double wcross = 9.859272078406150e-02;

        variable vs[6];

        double xs[6];

        double gxs[6];

        xs[0] = vs[0].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_1_a);
        xs[1] = vs[1].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_2_a);
        xs[2] = vs[2].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_3_a);
        xs[3] = vs[3].v_exp(m_k_x_intra_B_B_1, coords_B_1_a, coords_B_2_a);
        xs[4] = vs[4].v_exp(m_k_x_intra_B_B_1, coords_B_1_a, coords_B_3_a);
        xs[5] = vs[5].v_exp(m_k_x_intra_B_B_1, coords_B_2_a, coords_B_3_a);

        sw = 1.0;

        energies[j] = my_poly.eval(xs, coefficients.data(), gxs);
        energies_sw[j] = energies[j] * sw;

        for (size_t i = 0; i < 6; i++) {
            gxs[i] *= sw;
        }

        vs[0].grads(gxs[0], coords_A_1_a_g, coords_B_1_a_g, coords_A_1_a, coords_B_1_a);
        vs[1].grads(gxs[1], coords_A_1_a_g, coords_B_2_a_g, coords_A_1_a, coords_B_2_a);
        vs[2].grads(gxs[2], coords_A_1_a_g, coords_B_3_a_g, coords_A_1_a, coords_B_3_a);
        vs[3].grads(gxs[3], coords_B_1_a_g, coords_B_2_a_g, coords_B_1_a, coords_B_2_a);
        vs[4].grads(gxs[4], coords_B_1_a_g, coords_B_3_a_g, coords_B_1_a, coords_B_3_a);
        vs[5].grads(gxs[5], coords_B_2_a_g, coords_B_3_a_g, coords_B_2_a, coords_B_3_a);

        for (size_t i = 0; i < 3; i++) {
            gradients[0 + i] += 0.0;
        }

        for (size_t i = 0; i < 12; i++) {
            grad1[i + j * 12] += gradients[0 + i];
        }

        if (virial != 0) {
            (*virial)[0] += -coords_A_1_a[0] * coords_A_1_a_g[0] - coords_B_1_a[0] * coords_B_1_a_g[0] -
                            coords_B_2_a[0] * coords_B_2_a_g[0] - coords_B_3_a[0] * coords_B_3_a_g[0];

            (*virial)[1] += -coords_A_1_a[0] * coords_A_1_a_g[1] - coords_B_1_a[0] * coords_B_1_a_g[1] -
                            coords_B_2_a[0] * coords_B_2_a_g[1] - coords_B_3_a[0] * coords_B_3_a_g[1];

            (*virial)[2] += -coords_A_1_a[0] * coords_A_1_a_g[2] - coords_B_1_a[0] * coords_B_1_a_g[2] -
                            coords_B_2_a[0] * coords_B_2_a_g[2] - coords_B_3_a[0] * coords_B_3_a_g[2];

            (*virial)[4] += -coords_A_1_a[1] * coords_A_1_a_g[1] - coords_B_1_a[1] * coords_B_1_a_g[1] -
                            coords_B_2_a[1] * coords_B_2_a_g[1] - coords_B_3_a[1] * coords_B_3_a_g[1];

            (*virial)[5] += -coords_A_1_a[1] * coords_A_1_a_g[2] - coords_B_1_a[1] * coords_B_1_a_g[2] -
                            coords_B_2_a[1] * coords_B_2_a_g[2] - coords_B_3_a[1] * coords_B_3_a_g[2];

            (*virial)[8] += -coords_A_1_a[2] * coords_A_1_a_g[2] - coords_B_1_a[2] * coords_B_1_a_g[2] -
                            coords_B_2_a[2] * coords_B_2_a_g[2] - coords_B_3_a[2] * coords_B_3_a_g[2];

            (*virial)[3] = (*virial)[1];
            (*virial)[6] = (*virial)[2];
            (*virial)[7] = (*virial)[5];
        }
    }
    return energies_sw;
}

//----------------------------------------------------------------------------//
}  // namespace mbnrg_A1B3_deg5
