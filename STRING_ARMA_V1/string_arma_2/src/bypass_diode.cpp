#include "bypass_diode.h"
#include <cmath>
#include <fstream>
#include <cstdlib>
using namespace std;

#define Irref 5e-6 //10e-9
#define k 1.38e-23
#define Tcref 25.0
#define nby 1.5 //1.915 //  dades extretes d'SAM mòdul STP170S-24-Ab-1
#define q 1.602e-19

bypass_diode::bypass_diode(void)
{
	Tc = 273+Tcref;
	Ir = Irref;
	Idiode = 0;
}
void bypass_diode::setTc(double _Tc)
{
	Tc = _Tc+273;
}
void bypass_diode::setIr(void) // Model IET 2010, Wang, Hsu
{
	double Eg;
	double x, y;
	double Tcrefo = Tcref+273;
	Eg = 1,16 - 7.02e-4*pow(Tc,2)/(Tc+1108);
	x = ((q*Eg)/(nby*k))*(1/Tcrefo-1/Tc);
	y = Tc/Tcrefo;
	Ir = Irref*pow(y,3)*exp(x);
}
void bypass_diode::setIdiode(double _Id)
{
	Idiode = _Id;
}
double bypass_diode::calcIdiode(double Vdiode)
{
	double Vt = k*Tc/q;
	double x;
		
	x = Vdiode/(nby*Vt);
	Idiode = Ir*(exp(x)-1);
	return(Idiode);
}
double bypass_diode::getIdiode(void)
{
	return(Idiode);
}
double bypass_diode::calcderv(double Vd)
{
	double x,fp, w;
	double Vt = k*Tc/q;
		
	x = Vd/(nby*Vt);
	w = Ir/(nby*Vt);
	fp = w*exp(x);

	return(fp);
}