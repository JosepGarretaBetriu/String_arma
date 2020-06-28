/* Preprocessor directive to compile this file only once.
 * It is unsupported.
 * Should be replaced by a Header Guard. Ex:
 * #ifndef SOLAR_CELL
 * #define SOLAR_CELL
*/
#pragma once

//#define arev 0.002 // alfa de ruptura
//#define Vbr -15.0 // tensi� ruptura

// TODO: Decide if these constants should be in upper case. It would follow the standards but lose meaning
// Constants definitions
/// Breakdown alpha parameter.
constexpr double arev {0.002}; // alfa de ruptura
/// Breakdown voltage [V].
constexpr double Vbr {-15.0}; // tensi� ruptura
/// Reverse saturation current [A].
constexpr double Ioref {1.26E-9}; // 4.922e-10
/// Photogenerated current of reference, in case it is not specified [A].
constexpr double Iphref {3.798}; //5.142
/// Shortcut current of reference, in case it is not specified [A].
constexpr double Iscref {3.798}; //5.14
/// Open circuit voltage of reference, in case it is not specified [A].
constexpr double Vocref {0.9};// Vo dep�n proporcionalment d'n//0.6083 //extret de l'SAM (STP170S)
/// Boltzmann constant [J/ºK].
constexpr double k {1.38e-23};
/// Temperature of the cell of reference, in case it is not specified [ºC].
constexpr double Tcref {25.0};
/// Irradiance of reference, in case it is not specified [W/m2].
constexpr double Gref {1000};
/// Soiling Factor, 1 by default.
constexpr double SF {1};

constexpr double n {1.5}; //1.027 //  dades extretes d'SAM m�dul STP170S-24-Ab-1
/// Total resistance of the cell in series.
constexpr double Rs {0.00895};//0.031//0.001//0.00572 //0.00572 //
/// Total shunt resistance of the cell.
constexpr double Rsh {30.0}; //1000.0 //15.0 //19.0 //225.0 // 15.0 //  2.416 // 10
/// Charge of an electron [C].
constexpr double q {1.602e-19};
/// Temperature coefficient. Depends on the material, but this one belongs to silicon [A/ºC].
constexpr double a {0.0004}; // STP 180S // 0.0003
/// Voltage temperature coefficient. Depends on the material, but this one belongs to silicon [V/ºC].
constexpr double B {-0.0023};
/// Breakdown exponent.
constexpr double m {3.0}; // exponent ruptura

/** Represents a PV cell, the most basic element of a solar generator.
 *
 * This class contains all the parameters that define a single PV cell and to perform the calculations needed. Only the operational parameters of a PV cell are considered as attributes of this class. For intrinsic parameters of a PV cell, such as those that depend on the PV cell's material, they are implemented as constant and can not be edited. These values correspond to a silicon, multicrystalline PV cell.
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
 * The mathematical models of this library use some constants or approximations. The values used related to this class are:
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
 * @see solar_string()
 * @note The theoretical concepts behind this class are explained in the @ref solarCell_ch section of the @ref mainPage.
 * @warning This library contemplates the calculations of solar panels under mismatched conditions where irradiance (G) and temperature of the cell (Tc) are different across the facility. Scenarios where the cells that compose the panels have different intern parameters are NOT in the scope of this library and will not compute.
 */
class solar_cell
{
protected:
	/// Index of the cell. Serves as an identifier (ID) of the cell once it is grouped inside a string.
	int index; // NOU
	/// Photogenerated current [A].
	double Iph;
	/// Reverse saturation current [A].
	double Io;
	/// Shortcut current [A].
	double Isc;
	/// Open circuit voltage [V].
	double Voc;
	/// Temperature of the cell [ºC].
	double Tc;
	/// Irradiance [W/m2].
	double G;
	/// Current through the cell [A].
	double Icell;
	/// Voltage between the terminals of the cell [V].
	double Vcell;
	/// Breakdown voltage [V].
	double Vbreak;
	
public:
	/**
	 * Constructor of the class solar_cell.
	 * Uses all the reference values for the attributes.
	 */
	solar_cell(void);
	//solar_cell(const char *filename); //nou
	//solar_cell(double, double);
	/**
	 * Constructor of the class solar_cell.
	 *
	 * Uses the same attributes as the solar_cell object introduced as a parameter.
	 * @param &cell solar_cell object to copy the attributes from.
	 */
	solar_cell(const solar_cell&);
	/**
	 * Set an integer value for the index.
	 * @param _index Integer number of the index.
	 */
	void setIndex (int);
	/**
	 * Set a double value for the irradiance [W/m2].
	 * @param _G Double value of the new irradiance [W/m2].
	 */
	void setG(double);
	/**
	 * Set a double value for the temperature of the cell [ºC].
	 * @param _Tc Double value of the new temperature [ºC].
	 */
	void setTc(double);

	/// Updates the value for the reverse saturation current [A] according to the current value of the temperature of the cell Tc.
	void setIo(void);
	/// Updates the value for the shortcut current [A] according to the current values of the temperature of the cell Tc and the irradiance G.
	void setIsc(void);
	/// Updates the value for the photogenerated current [A] according to the current values of the temperature of the cell Tc and the irradiance G.
	void setIph(void);
	//void setVoc(double);
	/// Updates the value for the open circuit voltage [V] according to the current values of the temperature of the cell Tc and the irradiance G.
	void setVoc(void);
	/**
	 * Set a double value for the current [A].
	 * @param _Icell Double value of the cell's current [A].
	 */
	void setIcell(double);
	/**
	 * Set a double value for the voltage [V].
	 * @param _Vcell Double value of the cell's voltage [V].
	 */
	void setVcell(double);
	/**
	 * Set a double value for the breakdown voltage [V].
	 * @param _Vbreak Double value of the cell's breakdown voltage [V].
	 */
	void setVbreak (double);
	/**
	 * Gets the cell's index.
	 * @returns An integer type with the value of the index.
	 */
	int getIndex(void);
	/**
	 * Gets the reverse saturation current [A].
	 * @returns A double type with the value of the reverse saturation current [A].
	 */
	double getIo(void);
	/**
	 * Gets the photogenerated current [A].
	 * @returns A double type with the value of the photogenerated current [A].
	 */
	double getIph(void);
	/**
	 * Gets the shortcut current [A].
	 * @returns A double type with the value of the shortcut current [A].
	 */
	double getIsc(void);
	/**
	 * Gets the open circuit voltage [V].
	 * @returns A double type with the value of the open circuit voltage [V].
	 */
	double getVoc(void);
	/**
	 * Gets the irradiance [W/m2].
	 * @returns A double type with the value of the Irradiance [W/m2].
	 */
	double getG(void);
	/**
	 * Gets the temperature of the cell [ºC].
	 * @returns A double type with the value of the temperature of the cell [ºC].
	 */
	double getTc(void);
	/**
	 * Gets the cell's current [A].
	 * @returns A double type with the value of the current [A].
	 */
	double getIcell(void);
	/**
	 * Gets the cell's voltage [V].
	 * @returns A double type with the value of the voltage [V].
	 */
	double getVcell(void);
	/**
	 * Gets the breakdown voltage [V].
	 * @returns A double type with the value of the breakdown voltage [V].
	 */
	double getVbreak(void);
	/**
	 * Calculates the fc function described in the @ref math part of the @ref mainPage.
	 *
	 * The current values of Vcell and Icell are used to calculate this function.
	 *
	 * @returns A double type with the value of the funtion fc.
	 * @see @ref math
	 */
	double calcfcn(void);
	/**
	 * Calculates the partial derivative respect the current of the cell, Icell, of the fc function described in the @ref math part of the @ref mainPage.
	 *
	 * It is used to build the jacobian matrix explained in the @ref math_newton_part2 section.
	 * The current values of Vcell and Icell are used to calculate this function.
	 *
	 * @returns A double type with the value of the partial derivative respect the current of the cell of the funtion fc.
	 * @see @ref math
	 */
	double calcderi(void);
	/**
	 * Calculates the partial derivative respect the voltage of the cell, Vcell, of the fc function described in the @ref math part of the @ref mainPage.
	 *
	 * It is used to build the jacobian matrix explained in the @ref math_newton_part2 section.
	 * The current values of Vcell and Icell are used to calculate this function.
	 *
	 * @returns A double type with the value of the partial derivative respect the voltage of the cell of the funtion fc.
	 * @see @ref math
	 */
	double calcderv(void);
	};
