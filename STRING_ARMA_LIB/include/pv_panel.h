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

#pragma once

#include <iostream>
#include <vector>
#include "pv_string.h"

namespace stringarma{

/**
 * Represents a solar panel of a PV generator.
 *
 * This object contains all the information to create the PV panel.
 * It reads an input file with all the information related to the panel structure and temperature and
 * irradiance values for every cell.
 * Although, it does not create SolarString objects.
 * It only contains the information, since no calculations are done. It also contains the information related to the
 * PV cell and bypass diode that will conform the panel.
 * Only allows the creation of panels with cells with the same properties.
 *
 * @see SolarCell
 * @see BypassDiode
 * @see SolarString
 */
class SolarPanel
{
private:
	/**
	 * Vector to pass the input file information to the SolarSolver class.
	 */
	std::vector<std::pair<bool,std::vector<std::pair<double,double>>>> string_info;
	/**
	 * SolarCell object that will be cloned to build the entire panel.
	 * The properties of the cells are set in this one.
	 */
	stringarma::SolarCell cell_panel;
	/**
	 * Number of strings contained in this panel.
	 */
	int panel_size;
	/**
	 * Knee voltage of the bypass diodes in the panel [V].
	 */
	double voltage_knee_diode;

	friend class SolarSolver;

public:
	/**
	 * Constructor of the class solar_panel.
	 *
	 * @param *filepath Absolute path of the input file with the operational data.
	 * Its format should follow the one described in [the User's Guide](@ref input_file).
 	 * @see [Input file format](@ref input_file)
	 */
	SolarPanel(std::string);

	SolarPanel();

	/**
	 * Set a double value for the breakdown voltage [V] of the cells in the panel.
	 * @param Double value of the cell's breakdown voltage [V].
	 */
	void setCellVoltageBreakdown (double);
	/**
	 * Set a double value for the alpha parameter of the cells in the panel.
	 * @param Double value of the alpha parameter.
	 */
	void setCellBreakdownAlpha(double);
	/**
	 * Sets the soiling factor of the cells in the panel.
	 * @param SF A double type with the value of soiling factor.
	 */
	void setCellSoilingFactor(double);
	/**
	 * Sets the ideality factor of the cells in the panel.
	 * @param n A double type with the value of ideality factor.
	 */
	void setCellIdealityFactor(double);
	/**
	 * Sets the total resistance of the cell in series of the cells in the panel.
	 * @param Rs A double type with the value of total resistance of the cell in series.
	 */
	void setCellResistanceSeries(double);
	/**
	 * Sets the total shunt resistance of the cells in the panel.
	 * @param Rsh A double type with the value of total shunt resistance of the cell.
	 */
	void setCellResistanceShunt(double);
	/**
	 * Sets the temperature coefficient of the cells in the panel.
	 * @param a A double type with the value of temperature coefficient.
	 */
	void setCellTemperatureCoeff(double);
	/**
	 * Sets the voltage temperature coefficient of the cells in the panel.
	 * @param B A double type with the value of voltage temperature coefficient.
	 */
	void setCellVoltageTemperatureCoeff(double);
	/**
	 * Sets the breakdown exponent of the cells in the panel.
	 * @param m A double type with the value of breakdown exponent.
	 */
	void setCellBreakdownExponent(double);
	/**
	 * Sets the knee voltage of the bypass diodes in the panel [V].
	 * @param Vknee Knee voltage of the bypass diodes in the panel.
	 */
	void setVoltageKneeDiode(double);
	/**
	 * Gets the breakdown voltage [V] of the cells in the panel.
	 * @returns A double type with the value of the breakdown voltage [V].
	 */
	double getCellVoltageBreakdown(void);
	/**
	 * Gets the alpha parameter of the cells in the panel.
	 * @returns A double type with the value of alpha parameter.
	 */
	double getCellBreakdownAlpha(void);
	/**
	 * Gets the soiling factor of the cells in the panel.
	 * @returns A double type with the value of soiling factor.
	 */
	double getCellSoilingFactor(void);
	/**
	 * Gets the ideality factor of the cells in the panel.
	 * @returns A double type with the value of ideality factor.
	 */
	double getCellIdealityFactor(void);
	/**
	 * Gets the total resistance in series of the cells in the panel.
	 * @returns A double type with the value of total resistance of the cell in series.
	 */
	double getCellResistanceSeries(void);
	/**
	 * Gets the total shunt resistance of the cells in the panel.
	 * @returns A double type with the value of total shunt resistance of the cell.
	 */
	double getCellResistanceShunt(void);
	/**
	 * Gets the temperature coefficient of the cells in the panel.
	 * @returns A double type with the value of temperature coefficient.
	 */
	double getCellTemperatureCoeff(void);
	/**
	 * Gets the voltage temperature coefficient of the cells in the panel.
	 * @returns A double type with the value of voltage temperature coefficient.
	 */
	double getCellVoltageTemperatureCoeff(void);
	/**
	 * Gets the breakdown exponent of the cells in the panel.
	 * @returns A double type with the value of breakdown exponent.
	 */
	double getCellBreakdownExponent(void);
	/**
	 * Gets the knee voltage of the bypass diodes in the panel [V].
	 * @return The knee voltage of the bypass diodes in the panel [V].
	 */
	double getVoltageKneeDiode(void);

	/**
	 * Returns the number of strings in the panel.
	 * @return Integer with the number of strings in the panel.
	 */
	int getPanelSize();

	/**
	 * Reads the input file with the operational data described in [the User's Guide](@ref input_file).
	 *
	 * Reads the information and generates the panel according to it.
	 * @param filepath String with the absolute path of the input file.
	 * @returns A vector of pairs. Every element represents a string. Every pair contains a bool value, representing the state of the diode, and a vector of pairs of double values, the G and Tc correspondingly, representing every cell in the string.
	 */
	std::vector<std::pair<bool,std::vector<std::pair<double,double>>>> readInput(std::string);

};

}
