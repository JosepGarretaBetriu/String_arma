/*
 * Published under the General Public License GNU (VERSION 3)
 *
 * Copyright (c) 2017 Joan Ferran Salaet Pereira
 * Copyright (c) 2020 Josep Garreta Betriu
 * Copyright (c) (2017-2020) Universitat Politecnica de Catalunya (UPC)
 *
 * This file is part of Stringarma.
 *
 *   Stringarma is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Stringarma is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Stringarma.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pv_diode.h"
#include <cmath>
#include <fstream>
#include <cstdlib>
using namespace std;

namespace stringarma{

/// Reverse saturation current [A].
#define CURRENT_REVERSE_SATURATION_REF 5.6e-6
/// Boltzmann constant [J/ºK].
#define BOLTZMANN_CONST 1.38e-23
/// Temperature of the diode [ºC].
#define TEMPERATURE_DIODE_REF 25.0
/// Charge of an electron [C].
#define ELECTRONS_CHARGE 1.602e-19
/// Ideality factor of reference.
#define IDEALITY_FACTOR_REF 1.5

BypassDiode::BypassDiode(void)
{
	temperature_diode = 273+TEMPERATURE_DIODE_REF;
	current_reverse_saturation = CURRENT_REVERSE_SATURATION_REF;
	current_diode = 0;
	ideality_factor = IDEALITY_FACTOR_REF;
}
void BypassDiode::setTemperatureDiode(double _Td)
{
	temperature_diode = _Td+273;
}
void BypassDiode::setCurrentReverseSaturation(void) // Model IET 2010, Wang, Hsu
{
	double Eg;
	double x, y;
	double Tdrefo = TEMPERATURE_DIODE_REF+273;
	Eg = 1,16 - 7.02e-4*pow(temperature_diode,2)/(temperature_diode+1108);
	x = ((ELECTRONS_CHARGE*Eg)/(ideality_factor*BOLTZMANN_CONST))*(1/Tdrefo-1/temperature_diode);
	y = temperature_diode/Tdrefo;
	current_reverse_saturation = CURRENT_REVERSE_SATURATION_REF*pow(y,3)*exp(x);
}
void BypassDiode::setCurrentDiode(double _Id)
{
	current_diode = _Id;
}
void BypassDiode::setIdealityFactor(double _n)
{
	ideality_factor = _n;
}
double BypassDiode::getCurrentDiode(void)
{
	return(current_diode);
}
double BypassDiode::getCurrentReverseSaturation(void)
{
	return(current_reverse_saturation);
}
double BypassDiode::getTemperatureDiode(void)
{
	return(temperature_diode);
}
double BypassDiode::getIdealityFactor(void)
{
	return(ideality_factor);
}
double BypassDiode::calcFunctionD(double Vdiode)
{
	double Vt = BOLTZMANN_CONST*temperature_diode/ELECTRONS_CHARGE;
	double x;

	x = Vdiode/(ideality_factor*Vt);
	current_diode = current_reverse_saturation*(exp(x)-1);
	return(current_diode);
}
double BypassDiode::calcFuntionDiodeDerivativeRespectVoltage(double Vd)
{
	double x,fp, w;
	double Vt = BOLTZMANN_CONST*temperature_diode/ELECTRONS_CHARGE;

	x = Vd/(ideality_factor*Vt);
	w = current_reverse_saturation/(ideality_factor*Vt);
	fp = w*exp(x);

	return(fp);
}

}
