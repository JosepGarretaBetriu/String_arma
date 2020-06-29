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

/**
 * Represents a common component of a photovoltaic generator, a bypass diode.
 * These diodes are placed in anti-parallel configuration with one or more cells. The cells grouped by the same bypass diode are considered strings.
 * This class contains all the parameters that define a diode and to perform the calculations needed.
 *
 * When the class is created, it contains the following reference values:
 * - **Irref** The reverse saturation current is 5e-6 A.
 * - **Tcref** The temperature of the cell equals 25.0 ÂºC.
 * - **m_ref** Ideality factor of 1.5.
 *
 * Other mathematical constants used in the calculations are:
 * - **k** Boltzmann constant: 1.38e-23 J/ÂºK
 * - **q** Charge of an electron: 1.602e-19 C
 *
 * @see SolarCell
 * @see SolarString
 * @note The theoretical concepts behind this class are explained in the @ref bypass_ch section of the @ref mainPage.
 */
class BypassDiode
{
protected:
	/// Reverse saturation current [A].
	double current_reverse_saturation;
	/// Current temperature of the bypass diode [ÂºC].
	double temperature_diode;
	/// Current through the diode [A].
	double current_diode;
	/// Ideality factor.
	double ideality_factor;

public:
	/**
	 * Constructor of the class bypass_diode.
	 * Uses all the reference values.
	 */
	BypassDiode(void);
	/**
	 * Set a double value for the Temperature of the diode [ºC].
	 * @param Double value for the temperature of the diode [ºC].
	 */
	void setTemperatureDiode(double);
	/// Updates the value for the reverse saturation current [A] according to the current value of the temperature of the diode Tc.
	void setCurrentReverseSaturation(void);
	/**
	 * Updates the value for the current in the diode [A].
	 * @param Id Double value for the diode's current [A].
	 */
	void setCurrentDiode(double);
	/**
	 * Updates the value od the ideality factor of the bypass diode.
	 * @param m Ideality factor.
	 */
	void setIdealityFactor(double);
	/**
	 * Gets the diode's reverse saturation current [A].
	 * @returns A double type with the value of the reverse saturation current [A].
	 */
	double getCurrentReverseSaturation(void);
	/**
	 * Gets the diode's temperature [ºC].
	 * @returns A double type with the value of the temperature [ºC].
	 */
	double getTemperatureDiode(void);
	/**
	 * Gets the diode's current [A].
	 * @returns A double type with the value of the current [A].
	 */
	double getCurrentDiode(void);
	/**
	 * Gets the ideality factor applied in the bypass diode.
	 * @returns Ideality factor.
	 */
	double getIdealityFactor(void);
	/**
	 * Calculates the fd function described in the @ref bypass_ch part of the @ref mainPage.
	 * @param Double value of the diode's voltage Vd [V].
	 * @returns A double type with the value of the funtion fd [A].
	 */
	double calcFunctionD (double);
	/**
	 * Calculates the partial derivative respect the voltage of the diode, Vd, of the fd function described in the @ref math part of the @ref mainPage.
	 * @param Double value of the diode's voltage Vd [V].
	 * @returns A double type with the value of the partial derivative respect the voltage of the diode of the funtion fd.
	 * @see @ref math
	 */
	double calcFuntionDiodeDerivativeRespectVoltage(double);
	};

}
