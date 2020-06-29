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

#include "pv_cell.h"
#include <cmath>
#include <fstream>
#include <cstdlib>
using namespace std;

namespace stringarma{

SolarCell::SolarCell(void)
{
	current_photogenerated = CURRENT_PHOTOGENERATED_REF;
	current_shortcut = CURRENT_SHORTCUT_REF;
	voltage_open_circuit = VOLTAGE_OPEN_CIRCUIT_REF;
	temperature_cell = 273+TEMPERATURE_CELL_REF;
	irradiance = IRRADIANCE_REF;
	current_cell = CURRENT_SHORTCUT_REF;
	voltage_cell = VOLTAGE_OPEN_CIRCUIT_REF;
	voltage_breakdown = VOLTAGE_BREAKDOWN_REF;
	current_reverse_saturation = CURRENT_REVERSE_SATURATION_REF;
	breakdown_alpha = BREAKDOWN_ALPHA_REF;
	soiling_factor = SOILING_FACTOR_REF;
	ideality_factor = IDEALITY_FACTOR_REF;
	resistance_series = RESISTANCE_SERIES_REF;
	resistance_shunt = RESISTANCE_SHUNT_REF;
	temperature_coeff = TEMPERATURE_COEFF_REF;
	voltage_temperature_coeff = VOLTAGE_TEMPERATURE_COEFF_REF;
	breakdown_exponent = BREAKDOWN_EXPONENT_REF;
}

SolarCell::SolarCell(const SolarCell &cell)
{
	current_photogenerated = cell.current_photogenerated;
	current_shortcut = cell.current_shortcut;
	voltage_open_circuit = cell.voltage_open_circuit;
	temperature_cell = cell.temperature_cell;
	irradiance = cell.irradiance;
	current_cell = cell.current_cell;
	voltage_cell = cell.voltage_cell;
	voltage_breakdown = cell.voltage_breakdown;
	current_reverse_saturation = cell.current_reverse_saturation;
	breakdown_alpha = cell.breakdown_alpha;
	soiling_factor = cell.soiling_factor;
	ideality_factor = cell.ideality_factor;
	resistance_series = cell.resistance_series;
	resistance_shunt = cell.resistance_shunt;
	temperature_coeff = cell.temperature_coeff;
	voltage_temperature_coeff = cell.voltage_temperature_coeff;
	breakdown_exponent = cell.breakdown_exponent;
}
int SolarCell::getIndex(void)
{
	return (index);
}
double SolarCell::getCurrentShortcut(void)
{
	return (current_shortcut);
}
double SolarCell::getCurrentPhotogenerated(void)
{
	return (current_photogenerated);
}
double SolarCell::getCurrentReverseSaturation(void)
{
	return (current_reverse_saturation);
}
double SolarCell::getVoltageOpenCircuit(void)
{
	return (voltage_open_circuit);
}
double SolarCell::getIrradiance(void)
{
	return (irradiance);
}
double SolarCell::getTemperatureCell(void)
{
	return (temperature_cell);
}
double SolarCell::getCurrentCell(void)
{
	return (current_cell);
}
double SolarCell::getVoltageCell(void)
{
	return (voltage_cell);
}
double SolarCell::getVoltageBreakdown(void)
{
	return (voltage_breakdown);
}
double SolarCell::getBreakdownAlpha(void)
{
	return(breakdown_alpha);
}
double SolarCell::getSoilingFactor(void)
{
	return(soiling_factor);
}
double SolarCell::getIdealityFactor(void)
{
	return(ideality_factor);
}
double SolarCell::getResistanceSeries(void)
{
	return(resistance_series);
}
double SolarCell::getResistanceShunt(void)
{
	return(resistance_shunt);
}
double SolarCell::getTemperatureCoeff(void)
{
	return(temperature_coeff);
}
double SolarCell::getVoltageTemperatureCoeff(void)
{
	return(voltage_temperature_coeff);
}
double SolarCell::getBreakdownExponent(void)
{
	return(breakdown_exponent);
}
void SolarCell::setIrradiance(double _G)
{
	irradiance = _G;
}
void SolarCell::setTemperatureCell(double _Tc)
{
	temperature_cell = _Tc+273;
}
void SolarCell::setCurrentReverseSaturation(void)
{
	double Eg;
	double x, y;
	double Tcrefo = TEMPERATURE_CELL_REF+273;
	Eg = 1,16 - 7.02e-4*pow(temperature_cell,2)/(temperature_cell+1108);
	x = ((ELECTRONS_CHARGE*Eg)/(ideality_factor*BOLTZMANN_CONST))*(1/Tcrefo-1/temperature_cell);
	y = temperature_cell/Tcrefo;
	current_reverse_saturation = CURRENT_REVERSE_SATURATION_REF*pow(y,3)*exp(x);
}
void SolarCell::setIndex(int _index)
{
	index = _index;
}
void SolarCell::setCurrentShortcut(void)
{
	current_shortcut = CURRENT_SHORTCUT_REF*(1 + temperature_coeff*(temperature_cell - TEMPERATURE_CELL_REF-273))*soiling_factor*irradiance/IRRADIANCE_REF;
	current_shortcut = floor(current_shortcut*100 + 0.5)/100;
}
void SolarCell::setCurrentPhotogenerated(void)
{
	current_photogenerated = CURRENT_PHOTOGENERATED_REF*(1 + temperature_coeff*(temperature_cell - TEMPERATURE_CELL_REF-273))*soiling_factor*irradiance/IRRADIANCE_REF;
	current_photogenerated = floor(current_shortcut*100 + 0.5)/100;
}
void SolarCell::setVoltageOpenCircuit(void)
{
	double lratio = log(irradiance/IRRADIANCE_REF);
	double Vt = BOLTZMANN_CONST*temperature_cell/ELECTRONS_CHARGE;
	voltage_open_circuit = VOLTAGE_OPEN_CIRCUIT_REF + voltage_temperature_coeff*(temperature_cell - TEMPERATURE_CELL_REF-273) + ideality_factor*Vt*lratio; // d'acord amb Sandia 2004
	voltage_open_circuit = floor(voltage_open_circuit*100 + 0.5)/100;
}
void SolarCell::setCurrentCell(double _Icell)
{
	current_cell = _Icell;
}
void SolarCell::setVoltageCell(double _Vcell)
{
	voltage_cell = _Vcell;
}
void SolarCell::setVoltageBreakdown(double _Vbreak)
{
	voltage_breakdown =_Vbreak;
}
void SolarCell::setBreakdownAlpha(double _alpha)
{
	breakdown_alpha = _alpha;
}
void SolarCell::setSoilingFactor(double _SF)
{
	soiling_factor = _SF;
}
void SolarCell::setIdealityFactor(double _n)
{
	ideality_factor = _n;
}
void SolarCell::setResistanceSeries(double _Rs)
{
	resistance_series = _Rs;
}
void SolarCell::setResistanceShunt(double _Rsh)
{
	resistance_shunt = _Rsh;
}
void SolarCell::setTemperatureCoeff(double _a)
{
	temperature_coeff = _a;
}
void SolarCell::setVoltageTemperatureCoeff(double _B)
{
	voltage_temperature_coeff = _B;
}
void SolarCell::setBreakdownExponent(double _breakdown_exponent)
{
	breakdown_exponent = _breakdown_exponent;
}
double SolarCell::calcFunctionC(void)
{
	double x,y,z,multi, f;
	double Vt = BOLTZMANN_CONST*temperature_cell/ELECTRONS_CHARGE;

	x = (voltage_cell + current_cell*resistance_series)/(ideality_factor*Vt);
	y = 1-(voltage_cell + current_cell*resistance_series)/voltage_breakdown;
	multi = 1+breakdown_alpha*pow(y,-breakdown_exponent); //
	z = (voltage_cell + current_cell*resistance_series)*multi/resistance_shunt;
	f = current_cell - current_photogenerated + current_reverse_saturation*(exp(x)-1) + z;

	return(f);
}
double SolarCell::calcFunctionCellDerivativeRespectCurrent(void)
{
	double x,fp, w, y, z, multi, rupt;
	double Vt = BOLTZMANN_CONST*temperature_cell/ELECTRONS_CHARGE;

	x = (voltage_cell + current_cell*resistance_series)/(ideality_factor*Vt);
	w = (current_reverse_saturation*resistance_series)/(ideality_factor*Vt);
	y = 1-(voltage_cell + current_cell*resistance_series)/voltage_breakdown;
	multi = 1+breakdown_alpha*pow(y,-breakdown_exponent);
	z = resistance_series*multi/resistance_shunt;
	rupt = breakdown_exponent*breakdown_alpha*(voltage_cell + current_cell*resistance_series)*pow(y,-breakdown_exponent-1)*resistance_series/(voltage_breakdown*resistance_shunt);

	fp = 1 + w*exp(x) + z + rupt;

	return(fp);
}
double SolarCell::calcFunctionCellDerivativeRespectVoltage(void)
{
	double x,fp, w, y, z, multi, rupt;
	double Vt = BOLTZMANN_CONST*temperature_cell/ELECTRONS_CHARGE;

	x = (voltage_cell + current_cell*resistance_series)/(ideality_factor*Vt);
	w = current_reverse_saturation/(ideality_factor*Vt);
	y = 1-(voltage_cell + current_cell*resistance_series)/voltage_breakdown;
	multi = 1+breakdown_alpha*pow(y,-breakdown_exponent);
	z = multi/resistance_shunt;
	rupt = breakdown_exponent*breakdown_alpha*(voltage_cell + current_cell*resistance_series)*pow(y,-breakdown_exponent-1)/(voltage_breakdown*resistance_shunt);
	fp = w*exp(x) + z + rupt;

	return(fp);
}

}
