#ifndef FWI_H
#define FWI_H

#ifdef __cplusplus
extern "C"
{
#endif
	
// #include "am_mcu_apollo.h"
// #include "am_bsp.h"
// #include "am_util.h"

#include <stdio.h>   // Added 
#include <stdint.h>  // Added
#include <math.h>

extern double ffmc0, dmc0, dc0;
extern double ffmc, dmc, dc, isi, bui, fwi;
extern uint8_t mth;

void FWI_init(void);
double FWIcalc(double R,double U);
double BUIcalc(double P, double D);
double ISIcalc(double W, double F);
double DCcalc(double T, double Ro, double Do, int I);
double DMCcalc(double T, double H, double Ro, double Po, int I);
double FFMCcalc(double T, double H, double W, double Ro, double Fo);
void print_FWI(void);

/*
        FFMCcalc(temp, rhum, wind, prcp, ffmc0, &ffmc);
        DMCcalc(temp, rhum, prcp, dmc0, mth, &dmc);
        DCcalc(temp, prcp, dc0, mth, &dc);
        ISIcalc(ffmc, wind, &isi);
        BUIcalc(dmc, dc, &bui);
        FWIcalc(isi, bui, &fwi);
        ffmc0 = ffmc;
        dmc0 = dmc;
        dc0 = dc;
*/
#ifdef __cplusplus
}
#endif

#endif
