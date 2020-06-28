#include "solar_cell.h"
#include <cmath>
#include <fstream>
#include <cstdlib>
using namespace std;

/*
#define Ioref 1.26E-9 // 4.922e-10
#define Iphref 3.798 //5.142
#define Iscref 3.798 //5.14
#define Vocref 0.9// Vo dep�n proporcionalment d'n//0.6083 //extret de l'SAM (STP170S)
#define k 1.38e-23
#define Tcref 25.0
#define Gref 1000
#define SF 1  //Soiling Factor, 1 by default
#define n 1.5 //1.027 //  dades extretes d'SAM m�dul STP170S-24-Ab-1
#define Rs  0.00895//0.031//0.001//0.00572 //0.00572 //
#define Rsh 30.0 //1000.0 //15.0 //19.0 //225.0 // 15.0 //  2.416 // 10
#define q 1.602e-19
#define a 0.0004 // STP 180S // 0.0003
#define B -0.0023
#define m 3.0 // exponent ruptura
//#define Vbr -15.0 // tensi� ruptura
//#define roc 0.031
 *
 */

solar_cell::solar_cell(void)
{
	Iph = Iphref;
	Isc = Iscref;
	Voc = Vocref;
	Tc = 273+Tcref;
	G = Gref;
	Icell = Iscref;
	Vcell = Vocref;
	Vbreak = Vbr;
}
solar_cell::solar_cell(const solar_cell &cell)
{
	Iph = cell.Iph;
	Isc = cell.Isc;
	Voc = cell.Voc;
	Tc = cell.Tc;
	G = cell.G;
	Icell = cell.Icell;
	Vcell = cell.Vcell;
	Vbreak = cell.Vbreak;
}
int solar_cell::getIndex(void)
{
	return (index);
}
double solar_cell::getIsc(void)
{
	return (Isc);
}
double solar_cell::getIph(void)
{
	return (Iph);
}
double solar_cell::getIo(void)
{
	return (Io);
}
double solar_cell::getVoc(void)
{
	return (Voc);
}
double solar_cell::getG(void)
{
	return (G);
}
double solar_cell::getTc(void)
{
	return (Tc);
}
double solar_cell::getIcell(void)
{
	return (Icell);
}
double solar_cell::getVcell(void)
{
	return (Vcell);
}
double solar_cell::getVbreak(void)
{
	return (Vbreak);
}
void solar_cell::setG(double _G)
{
	G = _G;
}
void solar_cell::setTc(double _Tc)
{
	Tc = _Tc+273;
}
void solar_cell::setIo(void) // Model IET 2010, Wang, Hsu
{
	double Eg;
	double x, y;
	double Tcrefo = Tcref+273;
	Eg = 1,16 - 7.02e-4*pow(Tc,2)/(Tc+1108);
	x = ((q*Eg)/(n*k))*(1/Tcrefo-1/Tc);
	y = Tc/Tcrefo;
	Io = Ioref*pow(y,3)*exp(x);
}
void solar_cell::setIndex(int _index)
{
	index = _index;
}
void solar_cell::setIsc(void)
{
	Isc = Iscref*(1 + a*(Tc - Tcref-273))*SF*G/Gref;
	Isc = floor(Isc*100 + 0.5)/100;
}
void solar_cell::setIph(void)
{
	Iph = Iphref*(1 + a*(Tc - Tcref-273))*SF*G/Gref;
	Iph = floor(Isc*100 + 0.5)/100;
}
void solar_cell::setVoc(void)
{
	double lratio = log(G/Gref);
	double Vt = k*Tc/q;
	Voc = Vocref + B*(Tc - Tcref-273) + n*Vt*lratio; // d'acord amb Sandia 2004
	Voc = floor(Voc*100 + 0.5)/100;
}
void solar_cell::setIcell(double _Icell)
{
	Icell = _Icell;
}
void solar_cell::setVcell(double _Vcell)
{
	Vcell = _Vcell;
}
void solar_cell::setVbreak(double _Vbreak)
{
	Vbreak =_Vbreak;
}
double solar_cell::calcfcn(void)
{
	double x,y,z,multi, f;
	double Vt = k*Tc/q;
		
	x = (Vcell + Icell*Rs)/(n*Vt);
	y = 1-(Vcell + Icell*Rs)/Vbr;
	multi = 1+arev*pow(y,-m); // si arev=0 no s'hi inclou la ruptura
	z = (Vcell + Icell*Rs)*multi/Rsh;
	f = Icell - Iph + Io*(exp(x)-1) + z;

	return(f);
}
double solar_cell::calcderi(void)
{
	double x,fp, w, y, z, multi, rupt;
	double Vt = k*Tc/q;
		
	x = (Vcell + Icell*Rs)/(n*Vt);
	w = (Io*Rs)/(n*Vt);
	y = 1-(Vcell + Icell*Rs)/Vbr;
	multi = 1+arev*pow(y,-m); // si arev=0 no s'hi inclou la ruptura
	z = Rs*multi/Rsh;
	rupt = m*arev*(Vcell + Icell*Rs)*pow(y,-m-1)*Rs/(Vbr*Rsh);

	fp = 1 + w*exp(x) + z + rupt;

	return(fp);
}
double solar_cell::calcderv(void)
{
	double x,fp, w, y, z, multi, rupt;
	double Vt = k*Tc/q;
		
	x = (Vcell + Icell*Rs)/(n*Vt);
	w = Io/(n*Vt);
	y = 1-(Vcell + Icell*Rs)/Vbr;
	multi = 1+arev*pow(y,-m); // si arev=0 no s'hi inclou la ruptura
	z = multi/Rsh;
	rupt = m*arev*(Vcell + Icell*Rs)*pow(y,-m-1)/(Vbr*Rsh);
	fp = w*exp(x) + z + rupt;

	return(fp);
}
