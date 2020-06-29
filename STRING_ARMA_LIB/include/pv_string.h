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

#include "pv_cell.h"
#include "pv_diode.h"
#include <vector>
#include <list>

namespace stringarma{

/**
 * Defines the total parameters of a group of PV cells in the same string that have the same short-circuit current.
 */
struct TotalsOfCellsGroup {
		/**
		 * Vector with the physical position index of every cell in the group.
		 * The size of this vector is equal to the number of cells included in this group.
		 */
		std::vector<int> index;
		/// Short-circuit current of every cell in this group.
		double current_shortcircuit;
		/// Sum of all breakdown voltages of the PV cells in this group.
		double sum_voltage_breakdown;
		/// Sum of all open circuit voltages of the PV cells in this group.
		double sum_voltage_open_circuit;
		/**
		 * Sum of all breakdown voltages of the PV cells calculated on the group.
		 * @see [Solar string's theoretical documentation](@ref string_ch)
		 */
		double sum_voltage_breakdown_in_group;
	};

/**
 * Represents a string of solar cells group under the same bypass diode.
 * However, the diode can be missing, broken or non-active. This class contains all the details of the components of the string.
 * The cells in the string are divided in groups according to their short-circuit current Isc. These groups are distinguished:
 * - **Active cells groups**: Cells working under their own short-circuit current Isc.
 * - **Non-active cells groups**: Cells working under a different current from their Isc (a lower value).
 * - **Breakdown cells groups**: Cells working in the breakdown zone of the cell. Therefore working under a different current from their Isc (a higher value).
 *
 * Given the total current and voltage between the terminals of the string, this class can find an initial estimation of the state of every component. The values used are the following:
 * - **Non-active cells**: Current is imposed by the rest of the panel or an active group in the string. Working voltage is its open circuit voltage.
 * - **Breakdown cells**: Current is imposed by the rest of the panel or an active group in the string. Working voltage is its breakdown voltage.
 * - **Active cells**: Current is its short-circuit current. Voltage is deducted from the total voltage in the string, the diode's voltage and voltage in the rest of the groups.
 *
 * @see SolarCell
 * @see BypassDiode
 * @note The theoretical concepts behind this class are explained in the @ref string_ch section of the @ref mainPage.
 */
class SolarString
{
public:
	/**
	 * Array of solar_cell objects.
	 * This is a representation of the PV cells contained in this string. The cells in this array must have the same manufacturing properties, but the electrical or physical working values may differ.
	 * @see SolarCell
	 */
	SolarCell *cells_array;
	/**
	 * bypass_diode object.
	 * Represents the bypass diode of the string.
	 */
	BypassDiode diode_bypass;
	/**
	 * List that contains all the info about the different groups of cells in the string that share the same short-circuit current Isc.
	 * Every element in the list is a TableStr struct with the info of the group of cells.
	 *
	 * @see TotalsOfCellsGroup structure.
	 */
	std::list<TotalsOfCellsGroup> groupsByCurrentShortcircuit;
	/**
	 * Number of cells contained in the string.
	 */
	int string_size;

	friend class SolarSolver;

private:

	/**
	 * Knee voltage of the bypass diode [V].
	 */
	double voltage_knee_diode;
	/**
	 * Current through the bypass diode [A].
	 */
	double current_diode;
	/**
	 * Indicates whether the string of PV cells has a by-pass diode or not. By default it is 1.
	 * @returns An integer data type. 1 indicates that there is a diode, 0 indicates that there is not.
	 */
	int with_diode;
	/**
	 * Sum of all the open circuit voltage of the cells in the string [V].
	 */
	double sum_voltage_open_circuit;
	/**
	 * Sum of the breakdown voltage of all the cells in the string [V].
	 */
	double sum_voltage_breakdown;
	/**
	 * Voltage between the terminals of the string [V].
	 */
	double voltage_string;
	/**
	 * Sum of the voltage between the terminals of every cell in the string [V].
	 */
	double sum_voltage_all_cells;

public:
	/**
	 * Constructor of the class solar_string.
	 * Uses all the reference values for the attributes.
	 */
	SolarString (void); //constructor 1
	/**
	 *  Destructor of the class solar_string.
	 */
	~SolarString (void);
	/**
	 * Indicates whether the string of PV cells has a by-pass diode or not. By default it is 1.
	 * @returns An integer data type. 1 indicates that there is a diode, 0 indicates that there is not.
	 */
	int getWithDiode(void);
	/**
	 * Gets the minimum short-circuit current in the string [A].
	 * @returns Double data type with the value of the minimum short-circuit current [A].
	 */
	double getMinimCurrentShortcircuit (void);
	/**
	 * Gets the sum of all the open circuit voltage of the cells in the string [V].
	 * @returns Double data type with the value of the sum of open circuit voltages [V].
	 */
	double getSumVoltageOpenCircuit (void);
	/**
	 * Gets the sum of the breakdown voltage of all the cells in the string [V].
	 * @returns Double data type with the value of the sum of breakdown voltages [V].
	 */
	double getSumVoltageBreakdown (void);
	/**
	 * Gets the voltage between the terminals of the string [V].
	 * @returns Double data type with the value of the voltage in the string [V].
	 */
	double getVoltageString (void);
	/**
	 * Gets the sum of the voltage between the terminals of every cell in the string [V].
	 * This value can be different than the obtained with getVstring.
	 * @returns Double data type with the value of the sum of the voltages in every cell [V].
	 */
	double getSumVoltageAllCells (void);
	/**
	 * Gets the voltage between the terminals of the bypass diode [V].
	 * @returns Double data type value of the voltage in the bypass diode [V].
	 */
	double getVoltageDiode (void);
	/**
	 * Updates the Svoc attribute with the current value of Voc of every cell.
	 */
	void setSumVoltageOpenCircuit (void);
	/**
	 * Updates the Svbreak attribute with the current value of Vbreak of every cell.
	 */
	void setSumVoltageBreakdown (void);
	/**
	 * Set a new value for the voltage between the terminals of the string.
	 * @param Vstring New voltage between the terminals of the string [V].
	 */
	void setVoltageString (double);
	/**
	 * Updates the value of the sum of the voltage between the terminals of every cell in the string (Svcell) with its current value.
	 *
	 * It does the sum again. In case any value of any cell has changed.
	 */
	void setSumVoltageAllCells (void);
	/**
	 * Updates the value of the sum of the breakdown voltage (SVbrx) of every group of cells in CellsGr.
	 *
	 * If there is no bypass diode in the string, it is equal to the sum of the breakdown voltage between the terminals of every cell.
	 * If there is a bypass diode, it is obtained as explained in the [theorical documentation](@ref string_part2).
	 */
	void setSumVolageBreakdownInGroup (void);
	/**
	 * Set a new value for the knee voltage of the bypass diode of the string.
	 * @param Vdiode New knee voltage of the bypass diode of the string [V].
	 */
	void setVoltageDiode (double);

public:

	/**
	 * Fills the array cells_array with cells like the one provided as parameter and update them.
	 *
	 * The cells are set with the proper values of Irradiance and Temperature. Their electrical parameters are updated
	 * according to these values. Then sorts and groups the cells according to their short-circuit current.
	 *
	 * @param sc solar_cell object that represents all the cells in the string.
	 * @param string_input The first value of the pair is the state of the bypass diode. Every element in the vector represents a cell, and the pair of doubles its values of irradiance and temperature.
	 * @see solarCell_ch
	 */
	void updateStringsData (std::pair<bool,std::vector<std::pair<double,double>>> &, SolarCell &);
	/**
	 * Approximates the initial values for the iterative method.
	 *
	 * Given the total current and voltage between terminals in the string, assigns the electrical working point of every component in the string.
	 * This function first finds the state of the bypass diode and then the state and electrical conditions of every component.
	 * No value is returned, the changes are done in the solar_cell and bypass_diode objects contained by the string object.
	 *
	 * @param Iin Total current through the string [A].
	 * @param Vin Total potential difference between terminals of the string [V].
	 */
	void findInitialState (double&, double&);


private:


	/**
	 * Sets an index for every cells and updated their electrical parameters according to the current values of temperature and irradiance.
	 */
	void updateElectricalParameters (void);
	/**
	 * Fill the CellsGr list with the different groups of cells under the same working conditions.
	 *
	 * It also calculates the corresponding electrical parameters of each group.
	 */
	void updateGroupsByShortcircuitCurrent (void);
	/**
	 * Once the CellsGr has been filled with all the cells in the string, and it has been sorted, this method groups them under the same working conditions.
	 *
	 * This method is used inside genLlist().
	 */
	void sortGroupsByShortcircuitCurrent (void);
	/**
	 * Looks for the minimum of an array of double type pointers.
	 * @param Fa Array of double pointers.
	 * @return The minimum of the elements in the array.
	 */
	double minimumInArray (double*);
	/**
	 * Approximation of initial values when there is no diode.
	 *
	 * Given certain values for the total current and voltage between terminals in the string, this function assign the proper electrical working point to every component in the string in the case that the bypass diode is missing, broken or just in non-conducting state.
	 *
	 * @param Iin Total current through the string [A].
	 * @param Vin Total potential difference between terminals of the string [V].
	 */
	void findInitialStateWithoutDiode (double &,double &);
	/**
	 * Approximation of initial values when there is diode.
	 *
	 * Given certain values for the total current and voltage between terminals in the string, this function assign the proper electrical working point to every component in the string in the case that the bypass diode is in conducting state.
	 *
	 * @param Iin Total current through the string [A].
	 */
	void findInitialStateWithDiode (double &);
};

}
