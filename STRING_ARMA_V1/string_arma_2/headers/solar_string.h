#include "solar_cell.h"
#include "bypass_diode.h"
#include <vector>
#include <list>
using namespace std;

/**
 * Defines the total parameters of a group of PV cells in the same string that have the same shortcut current.
 */
struct TableStr { // descripci� de l'estat de les cel�les agrupades
// el nombre de membres del grup �s la mida del vector
		/**
		 * Vector with the physical position index of every cell in the group.
		 * The size of this vector is equal to the number of cells included in this group.
		 */
		vector<int> index; // llista d'�ndexs als membres
		/// Shortcut current of every cell in this group.
		double IscGrup;
		/// Sum of all breakdown voltages of the PV cells in this group.
		double SVbr;
		/// Sum of all open circuit voltages of the PV cells in this group.
		double SVoc;
		/// Sum of all breakdown voltages of the PV cells calculated on the group.
		/// @see [Solar string's theoretical documentation](@ref string_ch)
		double SVbrx; // Tensi� de ruptura calculada en el grup
		//void calcSVbr (solar_string *Str, int);
	};
//list<TableStr> 
/*struct NodeCellsGr { // nova estructura proposada per informar de les cel�les
		int *index; // assignaci� din�mica de grups de cel�les
		double IscGrup;
		double SVbr;
		double SVoc;
		NodeCellsGr *next; // punter a un nou nus de la llista
};*/

/**
 * Represents a string of solar cells group under the same bypass diode.
 * However, the diode can be missing, broken or non-active. This class contains all the details of the components of the string.
 * The cells in the string are divided in groups according to their shortcut current Isc. These groups are distinguished:
 * - **Active cells groups**: Cells working under their own shortcut current Isc.
 * - **Non-active cells groups**: Cells working under a different current from their Isc (a lower value).
 * - **Breakdown cells groups**: Cells working in the breakdown zone of the cell. Therefore working under a different current from their Isc (a higher value).
 *
 * Given the total current and voltage between the terminals of the string, this class can find an initial estimation of the state of every component. The values used are the following:
 * - **Non-active cells**: Current is imposed by the rest of the panel or an active group in the string. Working voltage is its open circuit voltage.
 * - **Breakdown cells**: Current is imposed by the rest of the panel or an active group in the string. Working voltage is its breakdown voltage.
 * - **Active cells**: Current is its shortcut current. Voltage is deducted from the total voltage in the string, the diode's voltage and voltage in the rest of the groups.
 *
 * @see solar_cell
 * @see bypass_diode
 * @note The theoretical concepts behind this class are explained in the @ref string_ch section of the @ref mainPage.
 */
class solar_string
{
public:
	/**
	 * Number of cells contained in the string.
	 */
	int midaCorda;
	/**
	 * Voltage between bypass diode terminals [V].
	 */
	double Vdiode;
	/**
	 * Current through the bypass diode [A].
	 */
	double Idiode;
	/**
	 * Array of solar_cell objects.
	 * This is a representation of the PV cells contained in this string. The cells in this array must have the same manufacturing properties, but the electrical or physical working values may differ.
	 * @see solar_cell
	 */
	solar_cell *corda;
	/**
	 * bypass_diode object.
	 * Represents the bypass diode of the string.
	 */
	bypass_diode rm3_c;

private:
	/**
	 * Indicates whether the string of PV cells has a by-pass diode or not. By default it is 1.
	 * @returns An integer data type. 1 indicates that there is a diode, 0 indicates that there is not.
	 */
	int ambDiode;
	// TODO: Delete attribute
	/**
	 * Deprecated attribute.
	 * @warning NOT IMPLEMENTED
	 */
	bool isDiodeOn;
	//TODO: why midaCorda and *corda are not private? it's not good to get the library's users working with pointers
	//int midaCorda;
	//solar_cell *corda;
	/**
	 * Sum of all the open circuit voltage of the cells in the string [V].
	 */
	double Svoc;
	/**
	 * Sum of the breakdown voltage of all the cells in the string [V].
	 */
	double Svbreak;
	/**
	 * Voltage between the terminals of the string [V].
	 */
	double Vstring;
	/**
	 * Sum of the voltage between the terminals of every cell in the string [V].
	 */
	double Svcell;
	// TODO: Delete attribute
	/**
	 * Deprecated attribute.
	 * @warning NOT IMPLEMENTED
	 */
	double It;
	// TODO: Delete attribute
	/**
	 * Deprecated attribute.
	 * @warning NOT IMPLEMENTED
	 */
	vector<double> strBounds; // limits de les regions de treball del string
public:
	//int numGrups;
	//TableStr *infoStr;
	//TODO: look for a way to document here the attributes of the struct
	/**
	 * List that contains all the info about the different groups of cells in the string that share the same shortcut current Isc.
	 * Every element in the list is a TableStr struct with the info of the group of cells.
	 *
	 * @see TableStr structure
	 */
	list<TableStr> CellsGr;

public:
	/**
	 * Constructor of the class solar_string.
	 * Uses all the reference values for the attributes.
	 */
	solar_string (void); //constructor 1
	//solar_string (bool, double, int); //constructor 2

	// TODO: DEPRECATED
	solar_string& operator= (const solar_string&); //overloading of assignment

	/**
	 *  Destructor of the class solar_string.
	 */
	~solar_string (void); //destructor
	/**
	 * Indicates whether the string of PV cells has a by-pass diode or not. By default it is 1.
	 * @returns An integer data type. 1 indicates that there is a diode, 0 indicates that there is not.
	 */
	int getambDiode(void);
	/**
	 * Gets the minimum shortcut current in the string [A].
	 * @returns Double data type with the value of the minimum shortcut current [A].
	 */
	double getIscmin (void);
	/**
	 * Gets the sum of all the open circuit voltage of the cells in the string [V].
	 * @returns Double data type with the value of the sum of open circuit voltages [V].
	 */
	double getSvoc (void);
	/**
	 * Gets the sum of the breakdown voltage of all the cells in the string [V].
	 * @returns Double data type with the value of the sum of breakdown voltages [V].
	 */
	double getSvbr (void);
	/**
	 * Gets the voltage between the terminals of the string [V].
	 * @returns Double data type with the value of the voltage in the string [V].
	 */
	double getVstring (void);
	/**
	 * Gets the sum of the voltage between the terminals of every cell in the string [V].
	 * This value can be different than the obtained with getVstring.
	 * @returns Double data type with the value of the sum of the voltages in every cell [V].
	 */
	double getSvcell (void);
	/**
	 * Gets the voltage between the terminals of the bypass diode [V].
	 * @returns Double data type value of the voltage in the bypass diode [V].
	 */
	double getVdiode (void);
	/**
	 * Reads a specified document and uses its information to update the fields of Irradiance and Temperature of the specified string.
	 * @param m Number (int) of the string to be updated.
	 * @param *filename Full path of the file or relative path from the project's directory. TODO: Change to full path only.
	 * @attention The document must be .csv file with a certain configuration. Please, check the [User's Guide](@ref usersGuide) to see the guide template for the input document format.
	 * @see [Input file format](@ref input_file)
	 */
	void update_physical (int, const char *);
	/**
	 * Sets an index for every cells and updated their electrical parameters according to the current values of temperature and irradiance.
	 */
	void update_electrical (void);
	/**
	 * Set all the cells in the string are like the one provided as parameter.
	 * Fills the array *corda with as many solar_cell objects as indicated in midaCorda.
	 * @param sc solar_cell object that represents all the cells in the string.
	 * @see solarCell_ch
	 */
	void inici_corda (solar_cell);
	/**
	 * Updates the Svoc attribute with the current value of Voc of every cell.
	 */
	void setSvoc (void);
	/**
	 * Updates the Svbreak attribute with the current value of Vbreak of every cell.
	 */
	void setSvbr (void);
	/**
	 * Set a new value for the voltage between the terminals of the string.
	 * @param Vstring New voltage between the terminals of the string [V].
	 */
	void setVstring (double);
	/**
	 * Updates the value of the sum of the voltage between the terminals of every cell in the string (Svcell) with its current value.
	 *
	 * It does the sum again. In case any value of any cell has changed.
	 */
	void setSvcell (void);
	/**
	 * @warning NOT IMPLEMENTED
	 * TODO: delete
	 */
	void setIdiode (double);
	/**
	 * @warning NOT IMPLEMENTED
	 * TODO: delete
	 */
	double getIdiode (void);
	/**
	 * @warning NOT IMPLEMENTED
	 * TODO: delete
	 */
	void sort_string (void);
	/**
	 * @warning NOT IMPLEMENTED
	 * TODO: delete
	 */
	int grups (void);
	/**
	 * @warning NOT IMPLEMENTED
	 * TODO: delete
	 */
	void classifica (int, int);
	/**
	 * Updates the value of the sum of the breakdown voltage of all the cells in the string.
	 *
	 * If there is no bypass diode in the string, it is equal to the sum of the voltage between the terminals of every cell.
	 * If there is a bypass diode, it is obtained as explained in the [theorical documentation](@ref string_part2).
	 */
	void setSVbrx (void); //Tensi� de ruptura de tot el grup (calculada sobre el string)
	/**
	 * Fill the CellsGr list with the different groups of cells under the same working conditions.
	 *
	 * It also calculates the corresponding electrical parameters of each group.
	 */
	void genLlist (void);
	// TODO: AccLlist shouldn't be public
	/**
	 * Once the CellsGr has been filled with all the cells in the string, and it has been sorted, this method groups them under the same working conditions.
	 *
	 * This method is used inside genLlist().
	 */
	void AccLlist (void);
	/**
	 * @warning NOT IMPLEMENTED
	 * TODO: delete
	 */
	void setBounds (void);
	/**
	 * Approximates the initial values for the iterative method.
	 *
	 * Given the total current and voltage between terminals in the string, assigns the electrical working point of every component in the string.
	 * This function first finds the state of the bypass diode and then the state and electrical conditions of every component.
	 * No value is returned, the changes are done in the solar_cell and bypass_diode objects contained by the string object.
	 *
	 * @param &Iin Total current through the string [A].
	 * @param &Vin Total potential difference between terminals of the string [V].
	 */
	void findValues (double&, double&);
	/**
	 * Looks for the minimum of an array of double type pointers.
	 * @param *Fa Array of double pointers.
	 * @return The minimum of the elements in the array.
	 */
	double minim (double*);
private:
	/**
	 * Approximation of initial values when there is no diode.
	 *
	 * Given certain values for the total current and voltage between terminals in the string, this function assign the proper electrical working point to every component in the string in the case that the bypass diode is missing, broken or just in non-conducting state.
	 *
	 * @param Iin Total current through the string [A].
	 * @param Vin Total potential difference between terminals of the string [V].
	 */
	void findOption1 (double &,double &);
	/**
	 * Approximation of initial values when there is diode.
	 *
	 * Given certain values for the total current and voltage between terminals in the string, this function assign the proper electrical working point to every component in the string in the case that the bypass diode is in conducting state.
	 *
	 * @param &_Iin Total current through the string [A].
	 * @param &_Vin Total potential difference between terminals of the string [V].
	 */
	void findOption2 (double &,double &);
};
