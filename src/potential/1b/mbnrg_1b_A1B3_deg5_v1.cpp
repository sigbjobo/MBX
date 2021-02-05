#include "mbnrg_1b_A1B3_deg5_v1.h"

////////////////////////////////////////////////////////////////////////////////

namespace mbnrg_A1B3_deg5 {

mbnrg_A1B3_deg5_v1::mbnrg_A1B3_deg5_v1(const std::string mon1) {

    // =====>> BEGIN SECTION CONSTRUCTOR <<=====
    // =>> PASTE RIGHT BELOW THIS LINE <==


    if (mon1 == "nh3") {
        coefficients = std::vector<double> {
             1.116811996245609e+03, // 0
            -7.517715048812847e+02, // 1
             1.573590651106973e+02, // 2
             1.704006340331464e+03, // 3
            -1.435030669241911e+03, // 4
            -4.115009142925989e+02, // 5
             1.468939042686225e+03, // 6
            -7.074814769653756e+02, // 7
            -2.844025079493812e+03, // 8
             2.757673703447268e+03, // 9
            -7.095913309022434e+02, // 10
             1.297064859934411e+03, // 11
             2.984878732556175e+02, // 12
            -5.653529269891218e+02, // 13
             2.683857566974473e+02, // 14
            -8.257749537330783e+02, // 15
            -3.459768282839765e+02, // 16
            -1.001350565810922e+03, // 17
            -1.482188837225861e+03, // 18
            -2.753323779844940e+02, // 19
             7.857026508116727e+02, // 20
            -5.863389653730299e+02, // 21
            -1.702349870760441e+03, // 22
            -1.493518441928699e+02, // 23
            -1.445241402188607e+03, // 24
            -9.682088251128087e+02, // 25
             9.584883350042026e+02, // 26
            -3.136924552560703e+02, // 27
            -3.663218082091415e+02, // 28
             1.487657520870658e+02, // 29
            -9.610561144073049e+02, // 30
            -2.767180315027084e+03, // 31
             3.144111727490667e+02, // 32
            -1.224495903242498e+03, // 33
            -1.173121584876940e+02, // 34
            -1.030144658197513e+03, // 35
            -8.739056340858061e+02, // 36
            -1.126505072926453e+03, // 37
             1.843440673929408e+03, // 38
             1.321556355524737e+03, // 39
            -9.825563768823226e+01, // 40
             6.171604359126560e+02, // 41
            -2.509937014634541e+02, // 42
            -2.143698577031965e+03, // 43
             2.553233882128149e+02, // 44
             1.458718028390406e+03, // 45
             3.209723184752632e+02, // 46
             7.053541394672720e+02, // 47
             1.587754868567195e+03, // 48
             3.498717550384903e+02, // 49
            -6.852248465727416e+02, // 50
             1.428918951988390e+03, // 51
            -2.188243122101729e+02, // 52
             9.307606039589556e+02, // 53
             1.456307996344648e+02, // 54
             9.985252105431836e+01, // 55
             4.357946688454894e+01, // 56
            -2.102951111850094e+03, // 57
            -4.678070701089681e+01, // 58
             1.282079434867047e+03, // 59
            -3.015302759948478e+02, // 60
             1.030957233648502e+02, // 61
            -1.134737307638781e+03, // 62
            -5.167638628953775e+02, // 63
            -8.848600906390928e+02, // 64
             1.042131164138010e+02, // 65
             1.190895496126458e+03, // 66
            -5.553339596247069e+02, // 67
             9.954624069147055e+02, // 68
             3.348582611732175e+02, // 69
            -1.445712668376278e+03, // 70
             8.728317757834872e+02, // 71
            -1.587509946754196e+02, // 72
            -9.471063869140569e+02, // 73
             7.078168248882185e+02, // 74
             2.625113496152183e+03, // 75
            -1.164749677413355e+03, // 76
            -6.735594615623949e+02, // 77
             1.099402386895388e+03, // 78
             1.190620328827394e+03, // 79
             2.195207265112679e+02, // 80
            -6.815599551671258e+01, // 81
             3.755947298291478e+02, // 82
             3.565873406758595e+02, // 83
            -1.135060264389745e+03, // 84
            -1.143341264951762e+03, // 85
             2.028741339254373e+02, // 86
            -1.498827604976162e+03, // 87
             4.530300756827008e+02, // 88
             7.169958797117355e+02, // 89
             1.503994696094437e+03, // 90
            -2.669594930313537e+02, // 91
             2.857744416262154e+02, // 92
            -9.740039128100732e+02, // 93
            -6.834662993673521e+02, // 94
            -4.812003210172915e+01, // 95
             7.294934715785466e+02, // 96
             1.033320801273289e+03, // 97
             1.374110092681966e+02, // 98
            -7.146176669639941e+02, // 99
            -2.461712317367502e+02, // 100
             3.788456007334845e+03}; // 101
    m_k_x_intra_A_B_1 =  6.968469723310915e-01; // A^(-1))
    m_k_x_intra_B_B_1 =  5.000000178938833e-01; // A^(-1))
    m_ri =  7.000000000000000e+00; // A
    m_ro =  8.000000000000000e+00; // A

    } // end if mon1 == "nh3"
    // =====>> END SECTION CONSTRUCTOR <<=====
}

//----------------------------------------------------------------------------//

double mbnrg_A1B3_deg5_v1::f_switch(const double r, double& g)
{
    if (r > m_ro) {
        g = 0.0;
        return 0.0;
    } else if (r > m_ri) {
        const double t1 = M_PI/(m_ro - m_ri);
        const double x = (r - m_ri)*t1;
        g = - std::sin(x)*t1/2.0;
        return (1.0 + std::cos(x))/2.0;
    } else {
        g = 0.0;
        return 1.0;
    }
}

//----------------------------------------------------------------------------//

 std::vector<double> mbnrg_A1B3_deg5_v1::eval(const double *xyz1, const size_t n) {
    std::vector<double> energies(n,0.0);
    std::vector<double> energies_sw(n,0.0);

    std::vector<double> xyz(12);
    double sw = 0.0;
    polynomial my_poly;

    for (size_t j = 0; j < n; j++) {
        const double *mon1 = xyz1 + 12*j;


        if (false ) {
             continue;
        }

        std::copy(mon1, mon1 + 12, xyz.begin() + 0);


        const double* coords_A_1_a = xyz.data() + 0;

        const double* coords_B_1_a = xyz.data() + 3;

        const double* coords_B_2_a = xyz.data() + 6;

        const double* coords_B_3_a = xyz.data() + 9;


        double w12 =     -9.721486914088159e-02;  //from MBpol
        double w13 =     -9.721486914088159e-02;
        double wcross =   9.859272078406150e-02;

    
        variable vs[6];

        double xs[6];

        xs[0] = vs[0].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_1_a);
        xs[1] = vs[1].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_2_a);
        xs[2] = vs[2].v_exp(m_k_x_intra_A_B_1, coords_A_1_a, coords_B_3_a);
        xs[3] = vs[3].v_exp(m_k_x_intra_B_B_1, coords_B_1_a, coords_B_2_a);
        xs[4] = vs[4].v_exp(m_k_x_intra_B_B_1, coords_B_1_a, coords_B_3_a);
        xs[5] = vs[5].v_exp(m_k_x_intra_B_B_1, coords_B_2_a, coords_B_3_a);


        sw = 1.0;

        energies[j] = my_poly.eval(xs,coefficients.data());
        energies_sw[j] = energies[j]*sw;

    }
    return energies_sw;

}

//----------------------------------------------------------------------------//

std::vector<double> mbnrg_A1B3_deg5_v1::eval(const double *xyz1, double *grad1 , const size_t n, std::vector<double> *virial) {
    std::vector<double> energies(n,0.0);
    std::vector<double> energies_sw(n,0.0);

    std::vector<double> xyz(12);
    double sw = 0.0;
    polynomial my_poly;

    for (size_t j = 0; j < n; j++) {
        const double *mon1 = xyz1 + 12*j;


        if (false ) {
             continue;
        }

        std::vector<double> gradients(12,0.0);

        std::copy(mon1, mon1 + 12, xyz.begin() + 0);
        const double* coords_A_1_a = xyz.data() + 0;

        const double* coords_B_1_a = xyz.data() + 3;

        const double* coords_B_2_a = xyz.data() + 6;

        const double* coords_B_3_a = xyz.data() + 9;


        double* coords_A_1_a_g = gradients.data() + 0;

        double* coords_B_1_a_g = gradients.data() + 3;

        double* coords_B_2_a_g = gradients.data() + 6;

        double* coords_B_3_a_g = gradients.data() + 9;



        double w12 =     -9.721486914088159e-02;  //from MBpol
        double w13 =     -9.721486914088159e-02;
        double wcross =   9.859272078406150e-02;

    
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

        energies[j] = my_poly.eval(xs,coefficients.data(),gxs);
        energies_sw[j] = energies[j]*sw;

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
            gradients[0 + i] += 0.0 ;
        }


        for (size_t i = 0; i < 12; i++) {
            grad1[i + j*12] += gradients[0 + i];
        }


    }
    return energies_sw;

}

//----------------------------------------------------------------------------//
} // namespace mbnrg_A1B3_deg5
