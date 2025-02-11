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

/* FFMC calculation */
double FFMCcalc(double T, double H, double W, double Ro, double Fo)
{
    double ffmc;
    double Mo, Rf, Mr, Ed, Ew, Ko, Kd, Kl, Kw, M, F;
    Mo = 147.2 * (101. - Fo) / (59.5 + Fo); /*Eq. 1 in */
    if (Ro > 0.5)
    {                  /*van Wagner and Pickett (1985) */
        Rf = Ro - 0.5; /*Eq. 2*/
        if (Mo <= 150.)
            Mr = Mo +
                 42.5 * Rf * (exp(-100. / (251. - Mo))) * (1 - exp(-6.93 / Rf)); /*Eq. 3a*/
        else
            Mr = Mo +
                 42.5 * Rf * (exp(-100. / (251. - Mo))) * (1 - exp(-6.93 / Rf)) +
                 .0015 * pow(Mo - 150., 2.) * pow(Rf, .5); /*Eq. 3b*/
        if (Mr > 250.)
            Mr = 250.;
        Mo = Mr;
    }
    Ed = 0.942 * pow(H, .679) + 11. * exp((H - 100.) / 10.) + .18 * (21.1 - T) * (1. - exp(-.115 * H)); /*Eq. 4*/
    if (Mo > Ed)
    {
        Ko = 0.424 * (1. - pow(H / 100., 1.7)) + 0.0694 * pow(W, .5) * (1. - pow(H / 100., 8.)); /*Eq. 6a*/
        Kd = Ko * .581 * exp(0.0365 * T);                                                        /*Eq. 6b*/
        M = Ed + (Mo - Ed) * pow(10., -Kd);                                                      /*Eq. 8*/
    }
    else
    {
        Ew = 0.618 * pow(H, .753) + 10. * exp((H - 100.) / 10.) + .18 * (21.1 - T) * (1. - exp(-.115 * H)); /*Eq. 5*/
        if (Mo < Ew)
        {
            Kl = 0.424 * (1. - pow((100. - H) / 100., 1.7)) + 0.0694 * pow(W, .5) * (1 - pow((100. - H) / 100., 8.)); /*Eq. 7a*/
            Kw = Kl * .581 * exp(0.0365 * T);                                                                         /*Eq. 7b*/
            M = Ew - (Ew - Mo) * pow(10., -Kw);                                                                       /*Eq. 9*/
        }
        else
            M = Mo;
    }
    ffmc = 59.5 * (250. - M) / (147.2 + M); /*Eq. 10*/
    if (ffmc > 101.0)
        ffmc = 101.0;
    
    return ffmc;
}

/* DMC calculation */
double DMCcalc(double T, double H, double Ro, double Po, int I)
{
    double dmc;
    double Re, Mo, Mr, K, B, P, Pr;
    
    if (T >= -1.1)
        K = 1.894 * (T + 1.1) * (100. - H) * Le[I - 1] * 0.0001;       // 10^-4 instead of 10^-6
    else
        K = 0.;
    /*Eq. 16*/
    /*Eq. 17*/
    if (Ro <= 1.5)
        Pr = Po;
    else
    {
        Re = 0.92 * Ro - 1.27;
        // Mo = 20. + 280.0 / exp(0.023 * Po);           // same as Eq. 12 (checked with an example)
        Mo = 20. + exp(5.6348 - Po / 43.43);
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
    if (dmc <= 0.0)
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
    double Rd, Qo, Qr, V, D, Dr;
    
    if (Ro > 2.8)
    {
        Rd = 0.83 * (Ro)-1.27;
        Qo = 800. * exp(-Do / 400.);
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
    double Fw, M, Ff, R;
    M = 147.2 * (101 - F) / (59.5 + F);
    Fw = exp(0.05039 * W);
    Ff = 91.9 * exp(-.1386 * M) * (1. + pow(M, 5.31) / 4.93E7);
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
    double U;
    if (P <= .4 * D)
        bui = 0.8 * P * D / (P + .4 * D);
    else
        bui = P - (1. - .8 * D / (P + .4 * D)) * (.92 + pow(.0114 * P, 1.7));
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
    double Fd, B, S;
    if (U <= 80.)
        Fd = .626 * pow(U, .809) + 2.;
    else
        Fd = 1000. / (25. + 108.64 * exp(-.023 * U));
    B = .1 * R * Fd;
    if (B > 1.)
        fwi = exp(2.72 * pow(.434 * log(B), .647));
    else
        fwi = B;

    return fwi;
}
/*Eq. 28a*/
/*Eq. 28b*/
/*Eq. 29*/
/*Eq. 30a*/
/*Eq. 30b*/


/* Main Program */


// void print_FWI(void){
// 	am_util_stdio_printf("FFMC,\t DMC ,\t DC  ,\t ISI ,\t BUI ,\t FWI\n");
// 	am_util_stdio_printf("%.2f,\t %.2f,\t %.2f,\t %.2f,\t %.2f,\t %.2f \n\n", ffmc, dmc, dc, isi, bui, fwi);
// }
void print_FWI(void){
    printf("FFMC,\t DMC ,\t DC  ,\t ISI ,\t BUI ,\t FWI\n");
    printf("%.2f,\t %.2f,\t %.2f,\t %.2f,\t %.2f,\t %.2f \n\n", ffmc, dmc, dc, isi, bui, fwi);
}


#ifdef __cplusplus
}
#endif
