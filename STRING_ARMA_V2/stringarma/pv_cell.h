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

namespace stringarma{

// Constants definitions
/// Breakdown alpha parameter.
constexpr double BREAKDOWN_ALPHA_REF {0.002};
/// Breakdown voltage [V].
constexpr double VOLTAGE_BREAKDOWN_REF {-15.0};
/// Reverse saturation current [A].
constexpr double CURRENT_REVERSE_SATURATION_REF {1.26E-9};
/// Photogenerated current of reference, in case it is not specified [A].
constexpr double CURRENT_PHOTOGENERATED_REF {3.798};
/// Shortcut current of reference, in case it is not specified [A].
constexpr double CURRENT_SHORTCUT_REF {3.798};
/// Open circuit voltage of reference, in case it is not specified [A].
constexpr double VOLTAGE_OPEN_CIRCUIT_REF {0.9};
/// Boltzmann constant [J/ºK].
constexpr double BOLTZMANN_CONST {1.38e-23};
/// Temperature of the cell of reference, in case it is not specified [ºC].
constexpr double TEMPERATURE_CELL_REF {25.0};
/// Irradiance of reference, in case it is not specified [W/m2].
constexpr double IRRADIANCE_REF {1000};
/// Soiling Factor, 1 by default.
constexpr double SOILING_FACTOR_REF {1};
/// Ideality factor
constexpr double IDEALITY_FACTOR_REF {1.5};
/// Total resistance of the cell in series.
constexpr double RESISTANCE_SERIES_REF {0.00895};
/// Total shunt resistance of the cell.
constexpr double RESISTANCE_SHUNT_REF {30.0};
/// Charge of an electron [C].
constexpr double ELECTRONS_CHARGE {1.602e-19};
/// Temperature coefficient. Depends on the material, but this one belongs to silicon [A/ºC].
constexpr double TEMPERATURE_COEFF_REF {0.0004};
/// Voltage temperature coefficient. Depends on the material, but this one belongs to silicon [V/ºC].
constexpr double VOLTAGE_TEMPERATURE_COEFF_REF {-0.0023};
/// Breakdown exponent.
constexpr double BREAKDOWN_EXPONENT_REF {3.0};

/** Represents a PV cell, the most basic element of a solar generator.
 *
 * This class contains all the parameters that define a single PV cell and
 * to perform the calculations needed. Only the operational parameters of a
 * PV cell are considered as attributes of this class. For intrinsic parameters
 * of a PV cell, such as those that depend on the PV cell's material, they are
 * implemented as constant and can not be edited. These values correspond to
 *  a silicon, multicrystalline PV cell.
 *
 * The editable attributes of this class are:
 * - **index** Identifier of the cell.
 * - **Iph** Photogenerated current [A].
 * - **Io** Reverse saturation current [A].
 * - **Isc** Shortcut current [A].
 * - **Voc** Open circuit voltage [V].
 * - **Tc** Temperature of the cell [ºC].
 * - **G** Irradiance [W/m2].
 * - **Icell** Current [A].
 * - **Vcell** Voltage [V].
 * - **Vbreak** Breakdown voltage [V].
 *
 * When it is not specified in the constructor of the class some attributes are initialized with reference values. These reference values are:
 * - **Ioref** The reverse saturation current of reference is 1.26E-9 A.
 * - **Iphref** The photogenerated current of reference is 3.798 A.
 * - **Iscref** The shortcut current of reference is 3.798 A.
 * - **Vocref** The open circuit voltage of reference is 0.9 V.
 * - **Tcref** The temperature of the cell of reference is 25.0 ºC.
 * - **Gref** The irradiance of reference is 1000 W/m2.
 *
 * The mathematical models of this library use some constants or approximations. These parameters can be set by the user.
 * The values of reference used related to this class are:
 * - \f$ \alpha \f$ Breakdown alpha parameter: 0.002
 * - **Vbr** Breakdown voltage: -15.0 V
 * - **k** Boltzmann constant: 1.38e-23 J/ºK
 * - **SF** Soiling Factor: 1
 * - **Rs** Total resistance of the cell in series: 0.00895
 * - **Rsh** Total shunt resistance of the cell: 30.0
 * - **q** Charge of an electron: 1.602e-19 C
 * - **a** Temperature coefficient: 0.0004 A/ºC
 * - **B** Voltage temperature coefficient: -0.0023 V/ºC
 * - **m** Breakdown exponent: 3
 *
 * @see SolarString
 * @note The theoretical concepts behind this class are explained in the @ref solarCell_ch section of the @ref mainPage.
 * @warning This library contemplates the calculations of solar panels under mismatched conditions where irradiance (G) and temperature of the cell Tc are different across the facility. Scenarios where the cells that compose the panels have different intern parameters are NOT in the scope of this library and will not compute.
 */
class SolarCell
{
protected:
	/// Index of the cell. Serves as an identifier (ID) of the cell once it is grouped inside a string.
	int index;
	/// Photogenerated current [A].
	double current_photogenerated;
	/// Reverse saturation current [A].
	double current_reverse_saturation;
	/// Shortcut current [A].
	double current_shortcut;
	/// Open circuit voltage [V].
	double voltage_open_circuit;
	/// Temperature of the cell [ºC].
	double temperature_cell;
	/// Irradiance [W/m2].
	double irradiance;
	/// Current through the cell [A].
	double current_cell;
	/// Voltage between the terminals of the cell [V].
	double voltage_cell;
	/// Breakdown voltage [V].
	double voltage_breakdown;
	/// Breakdown alpha
	double breakdown_alpha;
	/// Soiling factor
	double soiling_factor;
	/// Ideality factor
	double ideality_factor;
	/// Total resistance of the cell in series.
	double resistance_series;
	/// Total shunt resistance of the cell.
	double resistance_shunt;
	/// Temperature coefficient.
	double temperature_coeff;
	/// Voltage temperature coefficient.
	double voltage_temperature_coeff;
	/// Breakdown exponent.
	double breakdown_exponent;

public:
	/**
	 * Constructor of the class solar_cell.
	 * Uses all the reference values for the attributes.
	 */
	SolarCell(void);
	/**
	 * Constructor of the class solar_cell.
	 *
	 * Uses the same attributes as the solar_cell object introduced as a parameter.
	 * @param solar_cell object to copy the attributes from.
	 */
	SolarCell(const SolarCell&);
	/**
	 * Set an integer value for the index.
	 * @param Integer number of the index.
	 */
	void setIndex (int);
	/**
	 * Set a double value for the irradiance [W/m2].
	 * @param Doble value of the new irradiance [W/m2].
	 */
	void setIrradiance(double);
	/**
	 * Set a double value for the temperature of the cell [ºC].
	 * @param Doble value of the new temperature [ºC].
	 */
	void setTemperatureCell(double);

	/// Updates the value for the reverse saturation current [A] according to the current value of the temperature of the cell Tc.
	void setCurrentReverseSaturation(void);
	/// Updates the value for the shortcut current [A] according to the current values of the temperature of the cell Tc and the irradiance G.
	void setCurrentShortcut(void);
	/// Updates the value for the photogenerated current [A] according to the current values of the temperature of the cell Tc and the irradiance G.
	void setCurrentPhotogenerated(void);
	//void setVoc(double);
	/// Updates the value for the open circuit voltage [V] according to the current values of the temperature of the cell Tc and the irradiance G.
	void setVoltageOpenCircuit(void);
	/**
	 * Set a double value for the current [A].
	 * @param Double value of the cell's current [A].
	 */
	void setCurrentCell(double);
	/**
	 * Set a double value for the voltage [V].
	 * @param Double value of the cell's voltage [V].
	 */
	void setVoltageCell(double);
	/**
	 * Set a double value for the breakdown voltage [V].
	 * @param Double value of the cell's breakdown voltage [V].
	 */
	void setVoltageBreakdown(double);
	/**
	 * Set a double value for the alpha parameter.
	 * @param Double value of the alpha parameter.
	 */
	void setBreakdownAlpha(double);
	/**
	 * Sets the soiling factor.
	 * @param SF A double type with the value of soiling factor.
	 */
	void setSoilingFactor(double);
	/**
	 * Sets the ideality factor.
	 * @param n A double type with the value of ideality factor.
	 */
	void setIdealityFactor(double);
	/**
	 * Sets the total resistance of the cell in series.
	 * @param Rs A double type with the value of total resistance of the cell in series.
	 */
	void setResistanceSeries(double);
	/**
	 * Sets the total shunt resistance of the cell.
	 * @param Rsh A double type with the value of total shunt resistance of the cell.
	 */
	void setResistanceShunt(double);
	/**
	 * Sets the temperature coefficient.
	 * @param a A double type with the value of temperature coefficient.
	 */
	void setTemperatureCoeff(double);
	/**
	 * Sets the voltage temperature coefficient.
	 * @param B A double type with the value of voltage temperature coefficient.
	 */
	void setVoltageTemperatureCoeff(double);
	/**
	 * Sets the breakdown exponent.
	 * @param m A double type with the value of breakdown exponent.
	 */
	void setBreakdownExponent(double);
	/**
	 * Gets the cell's index.
	 * @returns An integer type with the value of the index.
	 */
	int getIndex(void);
	/**
	 * Gets the irradiance [W/m2].
	 * @returns A double type with the value of the Irradiance [W/m2].
	 */
	double getIrradiance(void);
	/**
	 * Gets the temperature of the cell [ºC].
	 * @returns A double type with the value of the temperature of the cell [ºC].
	 */
	double getTemperatureCell(void);
	/**
	 * Gets the reverse saturation current [A].
	 * @returns A double type with the value of the reverse saturation current [A].
	 */
	double getCurrentReverseSaturation(void);
	/**
	 * Gets the shortcut current [A].
	 * @returns A double type with the value of the shortcut current [A].
	 */
	double getCurrentShortcut(void);
	/**
	 * Gets the photogenerated current [A].
	 * @returns A double type with the value of the photogenerated current [A].
	 */
	double getCurrentPhotogenerated(void);
	/**
	 * Gets the open circuit voltage [V].
	 * @returns A double type with the value of the open circuit voltage [V].
	 */
	double getVoltageOpenCircuit(void);
	/**
	 * Gets the cell's current [A].
	 * @returns A double type with the value of the current [A].
	 */
	double getCurrentCell(void);
	/**
	 * Gets the cell's voltage [V].
	 * @returns A double type with the value of the voltage [V].
	 */
	double getVoltageCell(void);
	/**
	 * Gets the breakdown voltage [V].
	 * @returns A double type with the value of the breakdown voltage [V].
	 */
	double getVoltageBreakdown(void);
	/**
	 * Gets the alpha parameter.
	 * @returns A double type with the value of alpha parameter.
	 */
	double getBreakdownAlpha(void);
	/**
	 * Gets the soiling factor.
	 * @returns A double type with the value of soiling factor.
	 */
	double getSoilingFactor(void);
	/**
	 * Gets the ideality factor.
	 * @returns A double type with the value of ideality factor.
	 */
	double getIdealityFactor(void);
	/**
	 * Gets the total resistance of the cell in series.
	 * @returns A double type with the value of total resistance of the cell in series.
	 */
	double getResistanceSeries(void);
	/**
	 * Gets the total shunt resistance of the cell.
	 * @returns A double type with the value of total shunt resistance of the cell.
	 */
	double getResistanceShunt(void);
	/**
	 * Gets the temperature coefficient.
	 * @returns A double type with the value of temperature coefficient.
	 */
	double getTemperatureCoeff(void);
	/**
	 * Gets the voltage temperature coefficient.
	 * @returns A double type with the value of voltage temperature coefficient.
	 */
	double getVoltageTemperatureCoeff(void);
	/**
	 * Gets the breakdown exponent.
	 * @returns A double type with the value of breakdown exponent.
	 */
	double getBreakdownExponent(void);
	/**
	 * Calculates the fc function described in the @ref math part of the @ref mainPage.
	 *
	 * The current values of Vcell and Icell are used to calculate this function.
	 *
	 * @returns A double type with the value of the funtion fc.
	 * @see @ref math
	 */
	double calcFunctionC(void);
	/**
	 * Calculates the partial derivative respect the current of the cell, Icell, of the fc function described in the @ref math part of the @ref mainPage.
	 *
	 * It is used to build the jacobian matrix explained in the @ref math_newton_part2 section.
	 * The current values of Vcell and Icell are used to calculate this function.
	 *
	 * @returns A double type with the value of the partial derivative respect the current of the cell of the funtion fc.
	 * @see @ref math
	 */
	double calcFunctionCellDerivativeRespectCurrent(void);
	/**
	 * Calculates the partial derivative respect the voltage of the cell, Vcell, of the fc function described in the @ref math part of the @ref mainPage.
	 *
	 * It is used to build the jacobian matrix explained in the @ref math_newton_part2 section.
	 * The current values of Vcell and Icell are used to calculate this function.
	 *
	 * @returns A double type with the value of the partial derivative respect the voltage of the cell of the funtion fc.
	 * @see @ref math
	 */
	double calcFunctionCellDerivativeRespectVoltage(void);
	};

}
