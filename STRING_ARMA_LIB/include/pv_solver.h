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

/*
 *	This file uses the unmodified Armadillo C++ Linear Algebra Library:
 *
 *    Armadillo C++ Linear Algebra Library
 *	Copyright 2008-2020 Conrad Sanderson (http://conradsanderson.id.au)
 *	Copyright 2008-2016 National ICT Australia (NICTA)
 *	Copyright 2017-2020 Arroyo Consortium
 *	Copyright 2017-2020 Data61, CSIRO
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <list>
#include "pv_panel.h"

namespace stringarma{

#define MAX_ITERATIONS_REF 50
#define EPSILON_REF 0.01

/**
 * Structure to gather global information of a group of cells that share, at least, the same short-circuit current.
 */
struct CellsGroup {
	/// Short-circuit current of all the cells in the group.
	double current_shortcircuit;
	/**
	 * Breakdown voltage of the group.
	 * Calculated by adding all the breakdown voltages calculated in the group Vbrx of every cell.
	 *
	 * @see [Definition of Vbrx in the Theoretical documentation.](@ref string_part2)
	 */
	double sum_voltage_breakdown_in_group;
	/**
	 * Sum of all the open circuit voltage of the active cells.
	 * Sum of all the open circuit voltage of the cells in the group that are actually driving its short-circuit current (active cells) [V].
	 */
	double sum_voltage_open_circuit_all_cells;
	/**
	 * Sum of all the open circuit voltage of the non-active cells.
	 * Sum of all the open circuit voltage of the cells in the group that are not driving its short-circuit current (non-active cells), but an inferior current [V].
	 */
	double sum_voltage_open_circuit_non_active_cells;
	/// Number of cells in this group.
	int group_size;
	/// Voltage between the terminals of the panel where either a change in the distribution of the tensions or currents in the panel will take place.
	double limit_voltage;

};

/**
 * Structure with info of groups of cells with the same Isc and Vbrx.
 * Contains global information (inMapSum) and detailed information (inMapDetails) about groups of cells with the same short-circuit current and breakdown voltage.
 *
 * @see inVectorCell
 */
struct SameIshortcircuitAndVbreakdownGroup {
	/**
	 * Global information of the group of cells with the same Isc and Vbrx.
	 *
	 * Contains the short-circuit current of the group and the total sum of certain parameters.
	 * The grupString's Limit attribute stored in this structure refers to the internal limits.
	 *
	 * These "internal" limits are the total voltage in the panel needed to get every bypass diode in conducting state.
	 * In case there's no diode, the limit will match the lower external limit.
	 * The internal limits represent a change in the distribution of the total voltage.
	 *
	 * @see grupString
	 */
	CellsGroup sum_same_i_shortcircuit_and_v_breakdown_group;
	/**
	 * Detailed information of the group of cells with the same Isc and Vbrx.
	 *
	 * The cells in the group are split in smaller groups that share the same string.
	 * Every entry in the list contains a pair with an integer corresponding to the index of the string and a grupString structure with the grouped info of this group.
	 */
	std::list<std::pair<int,CellsGroup>> detailed_same_i_shortcircuit_and_v_breakdown_group;
};
/**
 * Vector meant to contain info of groups of cells with the same short-circuit current (Isc).
 *
 * Contains global information (inVectorSum) and detailed information (inVectorDetail).
 * Global information refers to the sum of certain parameters.
 * Detailed information distinguish smaller groups that share the same Isc and Vbrx.
 *
 * @see inMapCell
 */
struct SameIshortcircuitGroup {
	/**
	 * Global information of the group of cells with the same short-circuit current.
	 *
	 * Contains the short-circuit current of the group and the total sum of certain parameters.
	 * The grupString's Limit attribute stored in this structure refers to the external limits.
	 * These "external" limits are the total voltage in the panel needed to get every cell into breakdown. The external limits represent a change in the total current.
	 *
	 * @see grupString
	 */
	CellsGroup sum_same_i_shortcircuit_group;
	/**
	 * Detailed information of the group of cells with the same short-circuit current.
	 * The cells in the group are split in smaller groups that share the same breakdown voltage calculated in the group Vbrx.
	 * Every entry in the map is composed by key, which is a double corresponding to Vbrx, and a inMapCell structure with the information of this reduced group.
	 *
	 */
	std::map<double,SameIshortcircuitAndVbreakdownGroup> detailed_same_i_shortcircuit_group;
};

/**
 * Comparison function object for the multimap.
 * Compares the Isc of both groups.
 * In case they are equal, compares the SVbr of the groups.
 *
 * @returns True in case the Isc of the first element is lower than the first one. If they are equal, returns TRUE if the first element has higher SVbr.
 */
struct Classcomp
{
	bool operator()(const std::pair<double,double> &k1, const std::pair<double,double> &k2)
	{
		return ((k1.first < k2.first)||((k1.first == k2.first)&&(k1.second > k2.second)));
	}
};

/**
 *
 */
class SolarSolver
{
protected:
	/// Number of strings in the panel.
	int number_strings;
	/// Array of SolarString objects that compose the PV panel.
	SolarString *string_array;
	/// Main vector where all the info will be organized by short-circuit current, breakdown voltage and number of string.
	std::vector <SameIshortcircuitGroup> panel_vector;
	/// Maximum number of iterations to solve the Newton-Raphson iterative method.
	int max_iterations;
	/// Condition of convergence.
	double epsilon;

public:
	/**
	 * Constructor of the class SolarSolver.
	 *
	 * It automatically generates the objects to represent the class, update their parameters and classify them into groups in order to calculate the initial estimation.
	 * @param The SolarPanel object with the information to be simulated already loaded.
	 */
	SolarSolver(stringarma::SolarPanel&);
	/**
	 * Set a double value for the maximum number of iterations to solve the Newton-Raphson iterative method.
	 * @param Double value for the maximum number of iterations.
	 */
	void setMaxIterations(int);
	/**
	 * Set a double value for the condition of convergence (epsilon).
	 * @param Double value for the condition of convergence.
	 */
	void setEpsilon(double);
	/**
	 * Gets the maximum number of iterations to solve the Newton-Raphson iterative method.
	 * @returns A double type with the value of the maximum number of iterations.
	 */
	int getMaxIterations(void);
	/**
	 * Gets the condition of convergence (epsilon).
	 * @returns A double type with the condition of convergence (epsilon).
	 */
	double getEpsilon(void);
	/**
	 * Calculates the I-V characteristic of the SolarPanel object introduced in the constructor of the SolarSolver object.
	 * The resulting characteristic is stored in a file, specified as a parameter.
	 * @param output_path Full path of the file where to store the I-V characteristic. If the file exists it will be replaced. If it doesn't, it will be created.
	 */
	void calcIVcharacteristic(std::string);
	/**
	 * Calculates the I-V characteristic of the SolarPanel object introduced in the constructor of the SolarSolver object.
	 * The resulting characteristic is stored in a file, specified as a parameter.
	 * @param output_path Full path of the file where to store the I-V characteristic. If the file exists it will be replaced. If it doesn't, it will be created.
	 * @param start_v First voltage value in the characteristic.
	 * @param end_v Last voltage value in the characteristic.
	 * @param numb_points Number of points in the characteristic.
	 */
	void calcIVcharacteristic(std::string, double, double, int);
	/**
	 * Calculates the state the SolarPanel object introduced in the constructor of the SolarSolver object for a single value of voltage.
	 * The resulting .csv file, specified as a parameter, contains the number of string, position in the string, irradiance, temperature, current and voltage of every cell.
	 * It also contains the current of every diode.
	 * @param output_path Full path of the file where to store the calculated state. If the file exists it will be replaced. If it doesn't, it will be created.
	 * @param Vpan Total voltage in the panel.
	 */
	void calcState(std::string, double);

protected:

	/**
	 * @brief Fulfills the multimap structure with the data contained in the array of SolarString objects.
	 * The groups of cells of different strings working under the same breakdown current (Isc) and voltage (Vbr) are grouped inside the multimap.
	 *
	 * @param &MultiMapElDets Type multimap. The key is composed by two double values: Iscx and Vbrx in this order. The SameIshortcircuitAndVbreakdownGroup struct contains the corresponding sum_same_i_shortcircuit_and_v_breakdown_group (CellsGroup struct) with the total sums of the parameters of the groups of groups, and the detailed_same_i_shortcircuit_and_v_breakdown_group list of pairs with the integer pointing the number of the string and the CellsGroup struct with the specifics of this group.
	 * @param *&panel Array of SolarString objects.
	 */
	void retrieveDataFromStringArray (std::multimap <std::pair<double,double>, SameIshortcircuitAndVbreakdownGroup, Classcomp> &MultiMapElDets);

	/**
	 * Returns the "first upper limit" of the I-V characteristic. That is the sum of Voc of every cell.
	 */
	double findMaxVoltageLimit();

	/**
	 * Fulfills the voltage_limit parameter of the sum_same_i_shortcircuit_group CellsGroup.
	 * These "external" limits are the total voltage in the panel needed to get every cell into breakdown or to get the bypass diode to conducting state, whatever happens first.
	 * Notice that it is possible (and panels are designed so) that the bypass diode will enter conduction state before
	 * the breakdown.
	 *
	 * The external limits represent a change in the total current.
	 *
	 * @param LTO Maximum voltage limit of the panel (sum of all the open circuit voltages).
	 */
	void findVoltageLimitsForChangesInCurrent (const double LTO);
	/**
	 * Fulfills the voltage_limit parameter of the sum_same_i_shortcircuit_and_v_breakdown_group CellsGroup.
	 * These "internal" limits are the total voltage in the panel needed to get every bypass diode in conducting state.
	 * In case there's no diode, the limit will match the lower external limit.
	 * These limits are relative to the inferior voltage limit for changes in current.
	 *
	 * The internal limits represent a change in the distribution of the total voltage.
	 */
	void findVoltageLimitsForChangesInVoltage();
	/**
	 * Fulfills the vector with the information contained in the array of strings that represents the panel.
	 * It also organize all this info in the vector. By Isc, then by Isc and Vbrx, and by Isc, Vbrx and Index of string.
	 * In addition, calculates totals of every group and the limits of the I-V characteristic.
	 */
	void generatePanelVector();
	/**
	 * Given the external voltage limits (for changes in current), and numbering every zone in between them (from higher V zones to lower V zones),
	 * returns the operational zone that belongs to a certain voltage.
	 * LT2 -----zone 2------LT1------zone 1------LT0-----zone 0  where LT2<LT1<LT0
	 * @param Vin Total voltage in the panel [V].
	 * @returns An integer of the working zone.
	 */
	int findWorkingZone (double Vin);
	/**
	 * Adds the voltage to the strings corresponding to the cells that belong to an upper working zone than the current one.
	 *
	 * The working zone 0 is understood as the highest one.
	 * This implies that the active group of cells (imposing their Isc) has a higher Isc than the studied cells.
	 * This is possible if either the cells are in breakdown or the bypass diode of the string is conducting.
	 *
	 * @param m The current working zone.
	 * @param vVector Vector containing the voltage of every string. The index of the vector matches the number of the string.
	 * @returns No value. The vVector is updated.
	 * @see findWorkingZone()
	 */
	void calcUpperZones(int m, std::vector <double> &vVector);
	/**
	 * Adds the voltage to the strings corresponding to the cells that belong to a lower working zone than the current one.
	 *
	 * The working zone 0 is understood as the highest one.
	 * This implies that the active group of cells (imposing their Isc) has a lower Isc than the studied cells.
	 * Then the cells are in non-active state.
	 *
	 * @param m The current working zone.
	 * @param vVector Vector containing the voltage of every string. The index of the vector matches the number of the string.
	 * @returns No value. The vVector is updated.
	 * @see findWorkingZone()
	 */
	void calcLowerZones(int m, std::vector <double> &vVector);
	/**
	 * Adds the voltage to the strings corresponding to the cells that belong to the current working zone.
	 *
	 * This implies that the internal voltage limits (for changes in voltage) shall be taken into account.
	 * Since diodes may be or not conducting, in this zone cells can be in any state.
	 *
	 * @param m The current working zone.
	 * @param vVector Vector containing the voltage of every string. The index of the vector matches the number of the string.
	 * @returns No value. The vVector is updated.
	 * @see findWorkingZone()
	 */
	void calcMiddleZones(int m, double Vpan, std::vector <double> &vVector);
	/**
	 * Returns the total current that the panel will generate given a certain voltage.
	 * @param Vpan Total voltage in the panel [V].
	 */
	double findTotalCurrent(double Vpan);
	/**
	 * Given a total voltage through the panel, assigns the corresponding voltage to every string.
	 * @param Vpan Total voltage in the panel [V].
	 * @param VString Vector with the values of the voltage in every string. The index of the vector matches the index of the string in the panel.
	 * @returns No value. But the VString vector is updated with the calculated values.
	 */
	void assignStringVoltages(double Vpan, std::vector <double> &VString);
	/**
	 * Calculates the state of a given PV panel (an array of SolarString objects) by using the Newton-Raphson iterative method.
	 *
	 * Certain parameters such as the convergence condition or the maximum number of iteration can be set through other member methods.
	 * @param st Array of SolarString objects to be solved.
	 * @param Vp Total voltage in the panel [V].
	 * @param dimX Total number of variables. That is the total number of cells plus the number of strings plus one (the total current).
	 * @param nS Number of strings.
	 * @returns The total current generated by the panel. The values of voltage and current through every component of the panel are updated in the corresponding object.
	 */
	double calcNewtonRaphson (SolarString *st, double Vp, int _dimX, int nS);
	/**
	 * Returns a column matrix with the initial estimate of the solution to start the Newton-Raphson method.
	 * @param st Array of SolarString objects.
	 * @param n Total number of variables. That is the total number of cells plus the number of strings plus one (the total current).
	 * @param nS Number of strings.
	 * @returns The column with the initial estimation of the solution.
	 */
	//Col<double> loadInitialValues(solar_string *st, int n, int nS);

};

}
