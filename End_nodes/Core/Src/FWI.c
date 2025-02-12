#include "FWI.h"

#ifdef __cplusplus
extern "C"
{
#endif

double ffmc0 = 85.0;
double dmc0 = 6.0; 
double dc0 = 15.0; /* FFMC, DMC, and DC initialization */
double ffmc, dmc, dc, isi, bui, fwi;

static double Lf[] = {-1.6, -1.6, -1.6, .9, 3.8, 5.8, 6.4, 5., 2.4, .4, -1.6, -1.6};
static double Le[] = {6.5, 7.5, 9., 12.8, 13.9, 13.9, 12.4, 10.9, 9.4, 8., 7., 6.};


void FWI_init(void){
    ffmc0 = 85.0;
    dmc0 = 6.0;
    dc0 = 15.0; /* FFMC, DMC, and DC initialization */
}

/* Functions used to reduce the number of cycles */

static inline double fast_exp_neg(double x) {
    return 1.0 / (1.0 + x + 0.5 * x * x);
}

static inline double fastPow(double a, double b) {
    union {
        double d;
        int x[2];

    } u = { a };
    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}

static inline double fast_exp(double x) {
    return fastPow(2.718281828459045, x);
}
/* FFMC calculation */
double FFMCcalc(double T, double H, double W, double Ro, double Fo)
{
    double ffmc, Mo, Rf, Mr, Ed, Ew, Ko, Kd, Kl, Kw, M;
    double H100 = H / 100.0;
    double Hcomp = (100.0 - H) / 100.0;

    Mo = 147.2 * (101.0 - Fo) / (59.5 + Fo);

    // Eq 2
    if (Ro > 0.5)
    {
        Rf = Ro - 0.5;
        double expTerm = fast_exp_neg(100.0 / (251.0 - Mo));
        double expRf = fast_exp_neg(6.93 / Rf);
        double baseMr = 42.5 * Rf * expTerm * (1 - expRf);

        if (Mo <= 150.0) {
            Mr = Mo + baseMr;  // Eq 3a
        } else {
            Mr = Mo + baseMr + 0.0015 * (Mo - 150.0) * (Mo - 150.0) * sqrt(Rf);  // Eq 3b opti
        }

        Mo = (Mr > 250.0) ? 250.0 : Mr;
    }

    // Eq 4 (Ed)
    Ed = 0.942 * fastPow(H, 0.679) + 11.0 * fast_exp_neg(-(H - 100.0) / 10.0) + 0.18 * (21.1 - T) * (1.0 - fast_exp_neg(0.115 * H));

    if (Mo > Ed)
    {
        // Eq 6a et 6b opti
        double powH17 = fastPow(H100, 1.7);
        Ko = 0.424 * (1.0 - powH17) + 0.0694 * sqrt(W) * (1.0 - fastPow(H100, 8));
        Kd = Ko * 0.581 * fast_exp(0.0365 * T);
        M = Ed + (Mo - Ed) * fastPow(10.0, -Kd);
    }
    else
    {
        // Eq 5 (Ew)
        Ew = 0.618 * fastPow(H, 0.753) + 10.0 * fast_exp((H - 100.0) / 10.0) + 0.18 * (21.1 - T) * (1.0 - fast_exp_neg(0.115 * H));

        if (Mo < Ew)
        {
            // Eq 7a et 7b opti
            double powHc17 = fastPow(Hcomp, 1.7);
            Kl = 0.424 * (1.0 - powHc17) + 0.0694 * sqrt(W) * (1.0 - fastPow(Hcomp, 8));
            Kw = Kl * 0.581 * fast_exp(0.0365 * T);
            M = Ew - (Ew - Mo) * fastPow(10.0, -Kw);
        }
        else
        {
            M = Mo;
        }
    }

    // Eq 10
    ffmc = 59.5 * (250.0 - M) / (147.2 + M);
    return (ffmc > 101.0) ? 101.0 : ffmc;
}

/* DMC calculation */
double DMCcalc(double T, double H, double Ro, double Po, int I)
{
    double dmc;
    double Re, Mo, Mr, K, B, Pr;
    
    K = (T >= -1.1) ? (1.894 * (T + 1.1) * (100.0 - H) * Le[I - 1] * 0.0001) : 0.0;
    /*Eq. 16*/
    /*Eq. 17*/
    if (Ro <= 1.5)
        Pr = Po;
    else
    {
        Re = 0.92 * Ro - 1.27;
        Mo = 20. + fast_exp(5.6348 - Po / 43.43);
        if (Po <= 33.){
            B = 100. / (.5 + .3 * Po);
        }
        else
        {
            if (Po <= 65.)
                B = 14. - 1.3 * log(Po);
            else
                B = 6.2 * log(Po) - 17.2;
        }
        Mr = Mo + 1000. * Re / (48.77 + B * Re);
        Pr = 43.43 * (5.6348 - log(Mr - 20.));
    }
    if (Pr < 0.)
        Pr = 0.0;
    dmc = Pr + K;
    if (dmc <= 0.)
        dmc = 0.0;
    
    return dmc;
}
/*Eq. 11*/
/*Eq. 12*/
/*Eq. 13a*/
/*Eq. 13b*/
/*Eq. 13c*/
/*Eq. 14*/
/*Eq. 15*/
/* DC calculation */
double DCcalc(double T, double Ro, double Do, int I)
{
    double dc;
    double Rd, Qo, Qr, V, Dr;
    
    if (Ro > 2.8)
    {
        Rd = 0.83 * (Ro)-1.27;
        Qo = 800. * fast_exp_neg(Do / 400.);
        Qr = Qo + 3.937 * Rd;
        Dr = 400. * log(800. / Qr);
        if (Dr > 0.)
            Do = Dr;
        else
            Do = 0.0;
    }
    if (T > -2.8)
        V = 0.36 * (T + 2.8) + Lf[I - 1];
    else
        V = Lf[I - 1];
    if (V < 0.)
        V = .0;
    dc = Do + 0.5 * V;
    /*Eq. 18*/
    /*Eq. 19*/
    /*Eq. 20*/
    /*Eq. 21*/
    /*Eq. 22*/
    /*Eq. 23*/

    return dc;
}
/* ISI calculation */
double ISIcalc(double W, double F)   // ATTENTION swap the order of the arguments
{
    double isi;
    double Fw, M, Ff;
    M = 147.2 * (101 - F) / (59.5 + F);
    Fw = fast_exp(0.05039 * W);
    Ff = 91.9 * fast_exp(-.1386 * M) * (1. + fastPow(M, 5.31) / 4.93E7);
    isi = 0.208 * Fw * Ff;

    return isi;
}
/*Eq. 1*/
/*Eq. 24*/
/*Eq. 25*/
/*Eq. 26*/
/* BUI calculation */
double BUIcalc(double P, double D)
{
    double bui;
    if (P <= .4 * D)
        bui = 0.8 * P * D / (P + .4 * D);
    else
        bui = P - (1. - .8 * D / (P + .4 * D)) * (.92 + fastPow(.0114 * P, 1.7));
    if (bui < 0.0)
        bui = 0.0;

    return bui;
}


/*Eq. 27a*/
/*Eq. 27b*/
/* FWI calculation */
double FWIcalc(double R,double U)
{
    double fwi;
    double Fd, B;
    if (U <= 80.)
        Fd = .626 * fastPow(U, .809) + 2.;
    else
        Fd = 1000. / (25. + 108.64 * fast_exp_neg(.023 * U));
    B = .1 * R * Fd;
    if (B > 1.)
        fwi = fast_exp(2.72 * fastPow(.434 * log(B), .647));
    else
        fwi = B;

    return fwi;
}
/*Eq. 28a*/
/*Eq. 28b*/
/*Eq. 29*/
/*Eq. 30a*/
/*Eq. 30b*/


 void print_FWI(void){
 	APP_PRINTF("FFMC,\t DMC ,\t DC  ,\t ISI ,\t BUI ,\t FWI\n");
 	APP_PRINTF("%.2f,\t %.2f,\t %.2f,\t %.2f,\t %.2f,\t %.2f \n\n", ffmc, dmc, dc, isi, bui, fwi);
 }


#ifdef __cplusplus
}
#endif
