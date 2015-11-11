#include "math.h"
#include "foc.h"
#define PI (3.141592653589793)

const double PWM = 1; 
const double SQRT3 = 1.732050807568877;

struct PWM_Times {
    float a;
    float b;
    float c;
};

struct Clarke_Park_Result {
    float d;
    float q;
    float alpha;
    float beta;  
};


struct Clarke_Park_Result clarke_parke_trans(float iu, float iv, float iw, float rotor_angle) {
    float alpha = (2 * (iu - (iv/2) - (iw/2))) / 3;
    float beta = (2*(SQRT3/2 * iv - SQRT3/2 * iw))/3;
    float rotor_rad = rotor_angle * PI / 180;
    float d = cos(rotor_rad) * alpha + sin(rotor_rad) * beta;
    float q = cos(rotor_rad) * beta - sin(rotor_rad) * alpha;
    struct Clarke_Park_Result result = { d, q, alpha, beta };
    return result;
}


struct PWM_Times PWM_Times_vector_calc(float T1s, float T2s) {
    float T1 = PWM * T1s;
    float T2 = PWM * T2s;
    // results for return struct
    float Tc = (PWM - T1 - T2)/2;
    float Tb = Tc + T1;
    float Ta = Tb + T2;
    struct PWM_Times times = { Ta, Tb, Tc };
    return times;
}

struct SV sector_SVPWM(float uum, float uvm, float uwm) {
    // 0 - 60 degrees
    if (uum >= 0) {
        if (uvm >= 0) {
            struct PWM_Times times = PWM_Times_vector_calc(uvm, uum);
            struct SV space_vector = { times.a, times.b, times.c };
            return space_vector;
        } else {
            // 120 - 180 degrees
            if (uwm >= 0) {
                struct PWM_Times times = PWM_Times_vector_calc(uum, uwm);
                struct SV space_vector = { times.b, times.c, times.a };
                return space_vector;
            } else {
                // 60-120 degrees
                struct PWM_Times times = PWM_Times_vector_calc(-uvm, -uwm);
                struct SV space_vector = { times.a, times.c, times.b };
                return space_vector;
            }
        } 
    } else {
        if (uvm >= 0) {
            // 240 - 300 degrees
            if (uwm >= 0) {
                struct PWM_Times times = PWM_Times_vector_calc(uwm, uvm);
                struct SV space_vector = { times.c, times.a, times.b };
                return space_vector;
            } else {
                // 300-0 degrees
                struct PWM_Times times = PWM_Times_vector_calc(-uwm, -uum);
                struct SV space_vector = { times.b, times.a, times.c };
                return space_vector;
            }
        } else {
            // 180 - 240 degrees
            struct PWM_Times times = PWM_Times_vector_calc(-uum, -uvm);
            struct SV space_vector = { times.c, times.b, times.a };
            return space_vector;
        }
        
    }
}



struct SV mod_inv_clarke_park(float d, float q, float rotor_angle) {
    float rotor_rad = rotor_angle * PI / 180;
    float alpha = cos(rotor_rad) * d - sin(rotor_rad) * q;
    float beta = sin(rotor_rad) * d + cos(rotor_rad) * q;
    float uum = beta;
    float uvm = 0.5 * (SQRT3 * alpha - beta) ;
    float uwm = -0.5 * (beta + SQRT3 * alpha);
    struct SV return_vector = { uum, uvm, uwm };
    return return_vector;
}

struct SV get_Iuvw_For_Angle(float rotor_angle) {
    float u = cos(rotor_angle*2*PI/360);
    float v = cos(rotor_angle*2*PI/360-PI*2/3);
    float w = cos(rotor_angle*2*PI/360-PI*4/3);
    struct SV result = { u, v, w };
    return result; 
}

struct SV get_vector_for_angle(float rotor_angle) {
   struct SV iuvw = get_Iuvw_For_Angle(rotor_angle); 
   struct Clarke_Park_Result cpr = clarke_parke_trans( iuvw.u,  iuvw.v,  iuvw.w,  rotor_angle);
   struct SV in = mod_inv_clarke_park(cpr.d,cpr.q,rotor_angle);
   struct SV out = sector_SVPWM(in.u,in.v,in.w);
   return out;
}







