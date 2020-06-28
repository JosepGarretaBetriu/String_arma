#pragma once

/**
 * Represents a common component of a photovoltaic generator, a bypass diode.
 * These diodes are placed in anti-parallel configuration with one or more cells. The cells grouped by the same bypass diode are considered strings.
 * This class contains all the parameters that define a diode and to perform the calculations needed.
 *
 * When the class is created, it contains the following reference values:
 * - **Irref** The reverse saturation current is 5e-6 A.
 * - **Tcref** The temperature of the cell equals 25.0 ºC.
 *
 * Other mathematical constants used in the calculations are:
 * - **k** Boltzmann constant: 1.38e-23 J/ºK
 * - **q** Charge of an electron: 1.602e-19 C
 * - **m** Ideality factor of 1.5.
 *
 * @see solar_cell
 * @see solar_string
 * @note The theoretical concepts behind this class are explained in the @ref bypass_ch section of the @ref mainPage.
 */
class bypass_diode
{
protected:
	/// Reverse saturation current [A].
	double Ir;
	/// Current temperature of the bypass diode [ºC].
	double Tc;
	/// Current through the diode [A].
	double Idiode;
	
public:
	/**
	 * Constructor of the class bypass_diode.
	 * Uses all the reference values.
	 */
	bypass_diode(void);
	//bypass_diode(const bypass_diode&);

	/**
	 * Set a double value for the Temperature of the diode [ºC].
	 * @param _Tc Double value for the temperature of the diode [ºC].
	 */
	void setTc(double);
	/// Updates the value for the reverse saturation current [A] according to the current value of the temperature of the diode Tc.
	void setIr(void);
	/**
	 * Calculates the fd function described in the @ref bypass_ch part of the @ref mainPage.
	 * @param Vdiode Double value of the diode's voltage Vd [V].
	 * @returns A double type with the value of the funtion fd [A].
	 */
	double calcIdiode (double);
	/**
	 * Updates the value for the current in the diode [A].
	 * @param _Id Double value for the diode's current [A].
	 */
	void setIdiode(double);
	//void setVdiode(double);

	/**
	 * Gets the diode's reverse saturation current [A].
	 * @returns A double type with the value of the reverse saturation current [A].
	 * @warning NOT IMPLEMENTED
	 */
	double getIr(void);
	/**
	 * Gets the diode's temperature [ºC].
	 * @returns A double type with the value of the temperature [ºC].
	 * @warning NOT IMPLEMENTED
	 */
	double getTc(void);
	/**
	 * Gets the diode's current [A].
	 * @returns A double type with the value of the current [A].
	 */
	double getIdiode(void);
	//double getVdiode(void);

	/**
	 * Calculates the partial derivative respect the voltage of the diode, Vd, of the fd function described in the @ref math part of the @ref mainPage.
	 * @param Vd Double value of the diode's voltage Vd [V].
	 * @returns A double type with the value of the partial derivative respect the voltage of the diode of the funtion fd.
	 * @see @ref math
	 */
	double calcderv(double);
	};
