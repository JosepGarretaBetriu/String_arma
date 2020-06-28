#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>
//#include <assert.h>
#include <map>
//#include <functional>
#include <vector>
//#include <set>
//#include <bitset>
//#include <iterator>
#include "solar_cell.h"
#include "solar_string.h"
#include "bypass_diode.h"
#include "calcs.h"
//#include "gnuplot.h"

//#define Io 2.60  // 5.65
//#define Vo 0.0 // 0.35

// TODO: Hardcoded value. IMPORTANT ENHANCEMENT: It should be automatically detected by reading the file
#define numStrings 3
using namespace std;

/**
 * Structure to gather global information of a group of cells that share, at least, the same shortcut current.
 */
struct grupString { //dades d'un grup de cel�les de String
	/// Shortcut current of all the cells in the group.
	double Isc; // corrent del grup
	/**
	 * Breakdown voltage of the group.
	 * Calculated by adding all the breakdown voltages calculated in the group Vbrx of every cell.
	 *
	 * @see [Definition of Vbrx in the Theoretical documentation.](@ref string_part2)
	 */
	double SVbr; // tensi� de ruptura del grup
	/**
	 * Sum of all the open circuit voltage of the active cells.
	 * Sum of all the open circuit voltage of the cells in the group that are actually driving its shortcut current (active cells) [V].
	 */
	double SVoc; // tensi� de circuit obert de les cel�les actives del grup
	/**
	 * Sum of all the open circuit voltage of the non-active cells.
	 * Sum of all the open circuit voltage of the cells in the group that are not driving its shortcut current (non-active cells), but an inferior current [V].
	 */
	double SVocr; // tensi� de circuit obert de les cel�les no actives del grup
	/// Number of cells in this group.
	int N; // nombre de cel�les actives del grup
	/// Voltage between the terminals of the panel where either a change in the distribution of the tensions or currents in the panel will take place.
	double Limit; // l�mit de la regi� d'activitat del grup
	
};

/*Definici� d'estructura b�sica de detall d'elements amb mateixes caracter�stiques*/
/**
 * Structure with info of groups of cells with the same Isc and Vbrx.
 * Contains global information (inMapSum) and detailed information (inMapDetails) about groups of cells with the same shortcut current and breakdown voltage.
 *
 * @see inVectorCell
 */
struct inMapCell {
	/**
	 * Global information of the group of cells with the same Isc and Vbrx.
	 *
	 * Contains the shortcut current of the group and the total sum of certain parameters.
	 * The grupString's Limit attribute stored in this structure refers to the internal limits.
	 *
	 * These "internal" limits are the total voltage in the panel needed to get every bypass diode in conducting state.
	 * In case there's no diode, the limit will match the lower external limit.
	 * The internal limits represent a change in the distribution of the total voltage.
	 *
	 * @see grupString
	 */
	grupString inMapSum; // totals del grup amb mateix Isc i mateixa Vbr
	// TODO: turn list into a map?
	/**
	 * Detailed information of the group of cells with the same Isc and Vbrx.
	 *
	 * The cells in the group are split in smaller groups that share the same string.
	 * Every entry in the list contains a pair with an integer corresponding to the index of the string and a grupString structure with the grouped info of this group.
	 */
	list<pair<int,grupString>> inMapDetails; // llista amb els detalls dels grups de cel�les
	//amb mateix Isc i mateixa Vbr
};
/*Definici� de l'estructura b�sica que anir� dins del vector de panell*/

/**
 * Vector meant to contain info of groups of cells with the same shortcut current (Isc).
 *
 * Contains global information (inVectorSum) and detailed information (inVectorDetail).
 * Global information refers to the sum of certain parameters.
 * Detailed information distinguish smaller groups that share the same Isc and Vbrx.
 *
 * @see inMapCell
 */
struct inVectorCell {
	/**
	 * Global information of the group of cells with the same shortcut current.
	 *
	 * Contains the shortcut current of the group and the total sum of certain parameters.
	 * The grupString's Limit attribute stored in this structure refers to the external limits.
	 * These "external" limits are the total voltage in the panel needed to get every cell into breakdown. The external limits represent a change in the total current.
	 *
	 * @see grupString
	 */
	grupString inVectorSum;
	/**
	 * Detailed information of the group of cells with the same shortcut current.
	 * The cells in the group are split in smaller groups that share the same breakdown voltage calculated in the group Vbrx.
	 * Every entry in the map is composed by key, which is a double corresponding to Vbrx, and a inMapCell structure with the information of this reduced group.
	 *
	 */
	map<double,inMapCell> inVectorDetails;
};
/* Detecci� d'igualtat de corrent en Vector de Panell */
bool equalIsc (const inVectorCell &iVC1, const inVectorCell &iVC2 )
{
	return iVC1.inVectorSum.Isc == iVC2.inVectorSum.Isc;
}
/*comparacio en el multimapa de strings*/
/**
 * Comparison function object for the multimap.
 * Compares the Isc of both groups.
 * In case they are equal, compares the SVbr of the groups.
 *
 * @returns True in case the Isc of the first element is lower than the first one. If they are equal, returns TRUE if the first element has higher SVbr.
 */
struct Classcomp
{
	bool operator()(const pair<double,double> &k1, const pair<double,double> &k2) //operador definit per a les comparacions
	{
		return ((k1.first < k2.first)||((k1.first == k2.first)&&(k1.second > k2.second)));
	}
};
/* generaci� de multimapa de detalls de grups*/
/**
 * @brief Fulfills the multimap structure with the data contained in the array of solar_string objects.
 * The groups of cells of different strings working under the same breakdown current (Isc) and voltage (Vbr) are grouped inside the multimap.
 *
 * @param &MultiMapElDets Type multimap. The key is composed by two double values: Iscx and Vbrx in this order. The inMapCell struct contains the corresponding inMapSum (grupString struct) with the total sums of the parameters of the groups of groups, and the inMapDetails list of pairs with the integer pointing the number of the string and the grupString struct with the specifics of this group.
 * @param *&panel Array of solar_string objects.
 */
void genMultiMapDet (multimap <pair<double,double>, inMapCell, Classcomp> &MultiMapElDets, solar_string *&panel)
{
	double Iscx, Vbrx;
	// TODO: NOT USED
	double SVbrPrev = 0.0; //comptador de del�les pr�vies en ruptura
	grupString gS;
	inMapCell iMC;
	pair<double,double> clau;
	// Iterates all the strings in the panel.
	// Since the groups are ordered, the ones with lesser Isc will be first (breakdown,actives and cutted)
	for (int j=0; j<numStrings; j++){
		// Iterator of the groups of cells with the same parameters in the string
		list<TableStr>::iterator itList = panel[j].CellsGr.begin();
		while (itList !=panel[j].CellsGr.end()){// Comen�a el bucle de les cel�les amb ruptura abans que el d�ode sigui On
			//if (itList->SVbrx > itList->SVbr) break; // Si hi ha ruptura abans que el diode sigui On, bucle tallat
			
			// Stores locally all the values. It takes all the cells in the group as 'active cells'
			Iscx = itList->IscGrup;
			gS.N = itList->index.size();
			gS.SVoc = itList->SVoc;
			gS.SVocr = 0.0;
			//gS.SVbr = itList->SVbr;
			gS.SVbr = itList->SVbrx; //canvi fet el 25 set.
			SVbrPrev += gS.SVbr;
			
			Vbrx = itList->SVbrx/gS.N; //tesi� de ruptura individual
			clau = make_pair(Iscx,Vbrx); //clau del mapa

			// If the group analyzed is under breakdown end the while loop.
			// Since they're ordered the groups (if exist) under breakdown will go first.
			if (itList->SVbrx > itList->SVbr) break; // Si hi ha ruptura abans que el diode sigui On, bucle tallat
			
			iMC.inMapDetails.push_back(make_pair(j,gS));
			iMC.inMapSum = gS;
			MultiMapElDets.insert(make_pair(clau,iMC));
			iMC.inMapDetails.clear(); // buidem la llista a iMC

			advance(itList,1);
		}
		// Only enters this loop if the iterator is not at the end --> If there's group/s in breakdown
		if (itList != panel[j].CellsGr.end()){ // Si hi ha d�ode hi haur� un grup de cel�les actives quan d�ode on
			advance(itList,1);
			while(itList !=panel[j].CellsGr.end()){
				// If there's group/s in breakdown we have to correct the SVoc
				// TODO How about N? Shoudn't  it be fixed?
				gS.SVocr += itList->SVoc; // TODO what? restem de la caiguda en el d�ode bypass la resta de Voc
				advance(itList,1);
			}
			iMC.inMapDetails.push_back(make_pair(j,gS)); // llista de strings
			iMC.inMapSum = gS;

			MultiMapElDets.insert(make_pair(clau,iMC));
			iMC.inMapDetails.clear(); // buidem la llista a iMC

		}
	}
	/*Integra elements amb mateix corrent i tensi� de ruptura */
	// Unifies elements with the same Isc and Vbr. Across all the multimap. This implies the entire panel.
	// Instantiates a pair of multimaps
	pair<multimap<pair<double,double>,inMapCell, Classcomp>::iterator, multimap<pair<double,double>,inMapCell, Classcomp>::iterator> ret;
	// Temporal iterator to keep track of the last element
	multimap<pair<double,double>,inMapCell, Classcomp>::iterator itmap1;
	// Iterator 2 iterates across our filled multimap
	for (multimap<pair<double,double>,inMapCell, Classcomp>::iterator itmap2 = MultiMapElDets.begin(); itmap2 != MultiMapElDets.end(); ++itmap2){
		itmap1 = itmap2;
		ret = MultiMapElDets.equal_range(itmap2->first); //selecci� dels grups amb mateixa Vbr i Isc
		advance(itmap1,1);
		while (itmap1 != ret.second){
			itmap2->second.inMapSum.SVbr += itmap1->second.inMapSum.SVbr;
			itmap2->second.inMapSum.SVoc += itmap1->second.inMapSum.SVoc;
			itmap2->second.inMapSum.SVocr += itmap1->second.inMapSum.SVocr;
			itmap2->second.inMapSum.N += itmap1->second.inMapSum.N;

			itmap2->second.inMapDetails.push_back(*itmap1->second.inMapDetails.begin());
			itmap1 = MultiMapElDets.erase(itmap1); //esborra repeticions
			}
	}
}
/* Genera extrem de tensions de la corba IV */
/*
 * Returns the "first upper limit" of the I-V characteristic. That is the sum of Voc of every cell.
 */
double genLTO (const vector <inVectorCell> &PVect){
	double LTO = 0;
	for(int k=0; k<PVect.size(); ++k){
		LTO += (PVect[k].inVectorSum.SVocr);
	}
	return LTO;
}
/*generaci� dels l�mits externs dels grups Isc */
/*
 * Fulfills the Limit parameter of the inVectorSum grupString.
 * These "external" limits are the total voltage in the panel needed to get every cell into breakdown.
 * Notice that it is possible (and panels are designed so) that the bypass diode will enter conduction state before
 * the breakdown.
 *
 * The external limits represent a change in the total current.
 */
void genLTi (vector <inVectorCell> &PVect, const double LTO){
	double LTi = LTO;
	for (int k=0; k<PVect.size(); ++k){
		LTi -= PVect[k].inVectorSum.SVoc; //canvi dia 25 set.
		LTi += PVect[k].inVectorSum.SVbr;
		PVect[k].inVectorSum.Limit = LTi;
	}
}
/* generaci� dels l�mits interns dels grups Isc */
/*
 * Fulfills the Limit parameter of the inMapSum grupString.
 * These "internal" limits are the total voltage in the panel needed to get every bypass diode in conducting state.
 * In case there's no diode, the limit will match the lower external limit.
 *
 * The internal limits represent a change in the distribution of the total voltage.
 */
void genLRi(vector <inVectorCell> &PVect){
	int N, Ngr;
	double Voffset; // tensi� consistent en la ruptura i les circuit obert residuals
	map <double, inMapCell>::reverse_iterator itMap;
	for (int k=0; k<PVect.size(); ++k){
		N = PVect[k].inVectorSum.N;
		Voffset = -PVect[k].inVectorSum.SVbr; // 29 set
			for (itMap = PVect[k].inVectorDetails.rbegin(); itMap != PVect[k].inVectorDetails.rend(); ++itMap){
				Ngr = itMap->second.inMapSum.N;
				itMap->second.inMapSum.Limit = N*itMap->first + Voffset; // itMap->first �s Vbr individual
				N -= Ngr; // reducci� del nombre de grups per a cada iteraci�
				Voffset += itMap->second.inMapSum.SVbr; // increment de Voffset amb Vbr del grup
			}
	}
}
/* generaci� del vector de panell */
/*
 * Fulfills the vector with the information contained in the array of strings that represents the panel.
 * It also organize all this info in the vector. By Isc, then by Isc and Vbrx, and by Isc, Vbrx and Index of string.
 * In addition, calculates totals of every group and the limits of the I-V characteristic.
 */
void genPanelVector (vector <inVectorCell> &PVect, solar_string *&panel){
	multimap <pair<double,double>, inMapCell, Classcomp> MMPanel;
	inVectorCell iVC;
	/*
	 * Fills the MMPanel multimap with the info in the solar_string objects in panel and orders it
	 * by groups of cells with the same breakdown current and voltage
	 */
	genMultiMapDet(MMPanel, panel);
	// Iterates the multimap to update the Isc and initialize the rest of the values.
	// Every group is a new entry in the vector of inVectorCell
	for (multimap<pair<double,double>,inMapCell, Classcomp>::iterator itmap = MMPanel.begin(); itmap != MMPanel.end(); ++itmap){
		iVC.inVectorSum.Isc = itmap->first.first;
		iVC.inVectorSum.N = 0;
		iVC.inVectorSum.SVbr = 0;
		iVC.inVectorSum.SVoc = 0;
		iVC.inVectorSum.SVocr = 0;
		PVect.push_back(iVC);
	}
	vector <inVectorCell>::iterator it;
	// If there is any group with the same Isc, all but the first one are removed
	it = unique(PVect.begin(),PVect.end(), equalIsc);
	PVect.resize(distance(PVect.begin(),it));
	/* Intoducci� del mapa en el vector */
	multimap<pair<double,double>,inMapCell, Classcomp>::iterator itmap = MMPanel.begin();
	for (int k=0; k<PVect.size(); ++k){
		// Fulfills the inVectorDetail structs with a pair composed by a double type with the Vbrx of the group
		// and the corresponding inMapCell struct
		while(itmap->first.first == PVect[k].inVectorSum.Isc){
			PVect[k].inVectorDetails.insert(make_pair(itmap->first.second,itmap->second));
			advance(itmap,1);
			if (itmap == MMPanel.end()) break;
		}
		/* acumulaci� dels detalls en els absoluts del vector */
		// Fulfills the inVectorSum struct
		for(map <double, inMapCell>::iterator itMap = PVect[k].inVectorDetails.begin(); itMap != PVect[k].inVectorDetails.end(); ++itMap){
			PVect[k].inVectorSum.N += itMap->second.inMapSum.N;
			PVect[k].inVectorSum.SVbr += itMap->second.inMapSum.SVbr;
			PVect[k].inVectorSum.SVoc += itMap->second.inMapSum.SVoc;
			//PVect[k].inVectorSum.SVoc += itMap->second.inMapSum.SVocr; /*suma de totes les Voc*/
			PVect[k].inVectorSum.SVocr += itMap->second.inMapSum.SVocr;
		}
	}
	/*c�lcul l�mits dels grups amb mateix corrent (LT) */
	double LTO = genLTO(PVect);
	genLTi(PVect,LTO); // generaci� dels l�mits externs dels grups Isc
	genLRi(PVect); // generaci� dels l�mits interns dels grups Isc
	//MMPanel.~multimap <pair<double,double>, inMapCell, Classcomp>();
}

/*
 * Given the external limits, and numbering every zone in between them (from higher V zones to lower V zones),
 * returns the operational zone that belongs to a certain voltage.
 * LT2 -----zone 2------LT1------zone 1------LT0-----zone 0  where LT2<LT1<LT0
 */
int detectar (double Vin, vector <inVectorCell> &PVect)
{
	int i = 0;
	while(i<PVect.size()){
			if (Vin > PVect[i].inVectorSum.Limit)
				{break;
			}else{
				i = i+1;
			}
	}
	return(i);
}

void calc_upper(int m, vector <inVectorCell> &PVect, vector <double> &vVector)
{
	int iString;
	for (int k = 0; k < m; ++k){
		for (map<double,inMapCell>::iterator itmap = PVect[k].inVectorDetails.begin(); itmap != PVect[k].inVectorDetails.end(); ++itmap){
			for (list<pair<int,grupString>>::iterator itList = itmap->second.inMapDetails.begin(); itList != itmap->second.inMapDetails.end(); ++itList){
				iString = itList->first;
				vVector[iString]+=itList->second.SVbr + itList->second.SVocr; //el segon terme de la suma 29 set
			}
		}
	}
}

void calc_lower(int m, vector <inVectorCell> &PVect, vector <double> &vVector)
{
	int iString;
	for (int k = PVect.size()-1; k > m; --k){
		for (map<double,inMapCell>::iterator itmap = PVect[k].inVectorDetails.begin(); itmap != PVect[k].inVectorDetails.end(); ++itmap){
			for (list<pair<int,grupString>>::iterator itList = itmap->second.inMapDetails.begin(); itList != itmap->second.inMapDetails.end(); ++itList){
				iString = itList->first;
				vVector[iString]+=(itList->second.SVoc + itList->second.SVocr);
			}
		}
	}
}

void calc_middle(int m, double Vpan, vector <inVectorCell> &PVect, vector <double> &vVector)
{
	int iString;
	double Vrel = Vpan-PVect[m].inVectorSum.Limit; // tensi� relativa al l�mit de la regi�
	map<double,inMapCell>::iterator itmap;

	/* Detecci� de la zona d'activitat */
	for (itmap = PVect[m].inVectorDetails.begin(); itmap != PVect[m].inVectorDetails.end(); ++itmap){
		if (Vrel<itmap->second.inMapSum.Limit) break;
	}

	/* Assignaci� de tensions de ruptura dins de l'int�rval d'activitat */

	map<double,inMapCell>::iterator itmap2 = itmap;

	while (itmap2 != PVect[m].inVectorDetails.end()){
		for (list<pair<int,grupString>>::iterator itList = itmap2->second.inMapDetails.begin(); itList != itmap2->second.inMapDetails.end(); ++itList){
				iString = itList->first;
				vVector[iString]+=itList->second.SVbr;
			}
			advance(itmap2,1);
	}

	int N = 0;
	double vLim, vRupt, vS;

	for (map<double,inMapCell>::iterator itmap3 = PVect[m].inVectorDetails.begin(); itmap3 != itmap; ++itmap3){
		N += itmap3->second.inMapSum.N;
	}

	/* fem retrocedir l'iterador itmap per recollir les dades del grup actiu m�xim */

	advance(itmap,-1); // vigilar possible sobreeiximent d'�ndex

	vLim = itmap->second.inMapSum.Limit;
	vRupt = itmap->first;

	vS = (Vrel-vLim)/N + vRupt; // tensi� de cada cel�la activa

	/* c�lcul de les tensions en la zona d'activitat */

	advance(itmap,1);

	for (map<double,inMapCell>::iterator itmap3 = PVect[m].inVectorDetails.begin(); itmap3 != itmap; ++itmap3){
		for (list<pair<int,grupString>>::iterator itList = itmap3->second.inMapDetails.begin(); itList != itmap3->second.inMapDetails.end(); ++itList){
				iString = itList->first;
				N = itList->second.N;
				vVector[iString]+=(N*vS + itList->second.SVocr); //cel�les actives + acompanyant
			}
	}
}

void representar (void)
{
	/*
	Gnuplot plot;


	plot("set datafile separator \",\" \n");
	plot("set term windows");
	plot("set title \" Corbes aproximades \"");
	plot("set grid mxtics mytics");
	plot("set mytics 2");
	plot("set mxtics 5");
	plot("set yrange [-5:5]");
	plot("set grid xtics ytics");
	//plot("plot 'C:\\Users\\Joan Salaet\\Documents\\recerca\\eclipse_work\\string_arma_2\\prova_2x.dat' using 1:2 title 'calculat' \n ");
	//plot("plot \"C:\\Users\\Joan Salaet\\Documents\\recerca\\eclipse_work\\string_arma_2\\prova_2x"
	//		".dat\" using 1:2 title 'calculat' \n ");
	//plot("replot 'C:\\Users\\Joan Salaet\\Documents\\recerca\\eclipse_work\\string_arma_2\\prova_2y.dat' using 1:2 lw 2.000 title 'exacte' with lines \n");
	//system("pause"); //c++ DOS command shell to stop the console
	*/
}

/*
 * Overloading of the operator '=='. Compares the grupString structure by the Isc value.
 */
bool operator == (const grupString &gS1, const grupString &gS2) //operador definit per a les comparacions
{
	return (gS1.Isc == gS2.Isc);
}

/*
 * Compares the grupString structure by the Isc value. Returns true if they're equal.
 */
bool SameCurrent(grupString first, grupString second)
{
	return (first.Isc == second.Isc);
}

/*Generaci� del gr�fic amb els trams de la corba I-V */
/*
 * Generates an output file with info about the vector.
 * Used for testing purposes.
 */
void gencorbiv(vector <inVectorCell> &PVect){
	ofstream curvex;

	curvex.open("prova_2x.dat", ios::out);
	if (!curvex){
		cout << "no hom pot obrir l'arxiu.";
	}

	double Max = genLTO(PVect);
	int indexMin = PVect.size()-1;
	double Min = PVect[indexMin].inVectorSum.Limit;
	int m;

	for (double Vpan = Max; Vpan > -4.5; Vpan -= 0.05){
	//for (double Vpan = Max; Vpan > Min; Vpan -= 0.05){ //hem de comen�ar a l'extrem superior.
		m = detectar(Vpan,PVect); //Altrament sortirem fora dels �ndexs del vector
		curvex << Vpan << "," << PVect[m].inVectorSum.Isc << endl;

	}
	curvex.close();
	//representar();
}

/* Assignaci� de les tensions de String per a una tensi� total determinada, Vpan */
/*
 * Returns the total current that the panel will generate given a certain voltage.
 */
double assignItotal(double Vpan, vector <inVectorCell> &PVect){
	double Isc;
	int m;

	m = detectar(Vpan,PVect); // Detecci� del grups actius
	Isc = PVect[m].inVectorSum.Isc;
	return (Isc);
}

/*
 * Given a total voltage through the panel, assigns the corresponding voltage to every string.
 */
void assignVString(double Vpan, vector <inVectorCell> &PVect, vector <double> &VString){
	int m;

	m = detectar(Vpan,PVect); // Detecci� del grups actius
	/* �ndexs per sobre de la regi� de treball: cel�les en ruptura */
	calc_upper(m, PVect, VString);
	/* �ndexs per sota de la regi� de treball: cel�les en tall */
	calc_lower(m, PVect, VString);
	/* Regi� on les cel�les s�n actives */
	calc_middle(m, Vpan, PVect, VString);
}

/*
 * Generates an output file with the info extracted from the input file.
 * Used for testing purposes.
 */
void genArxcells(solar_string *&panel){
	ofstream arx;

	arx.open("sortida.csv", ios::out);
	if (!arx){
		cout << "no hom pot obrir l'arxiu.";
	}

	for (int k = 0; k < numStrings; ++k){
		arx << "Idiode(" << k << ") = " << panel[k].rm3_c.getIdiode() << " A" << endl;
	}

	arx << endl << endl;
	
	arx << "String" << "," << "Cella" << "," << "Irrad." << "," << "Temper." << "," << "Corr. (A)" << "," << "Tens. (V)" << endl;
	for (int k = 0; k < numStrings; ++k){
		for (int j = 0; j < panel[k].midaCorda; ++j){
			arx << k << "," << panel[k].corda[j].getIndex() << "," << panel[k].corda[j].getG() << "," << panel[k].corda[j].getTc();
			arx << "," << panel[k].corda[j].getIcell() << "," << panel[k].corda[j].getVcell() << endl;
		}
	}
}

/* Programa principal */

/*
 * This version is completely hardcoded. Any change in about diode's or PV cell's parameters, output or input files, etc. is
 * written in the code.
 *
 * This main function uses these pre-established values to create a panel (array of strings). Then uses a multimap structure to
 * retrieve all the information needed in this array. From this multimap, a vector of complex structures is fulfilled,
 * organizing and calculating some parameters.
 * This process makes it easier when it comes to assign voltages to every string given the total voltage in the panel.
 * To do this, the state of every cell (active [working at its Isc], non-active [forced to work with different current than
 * its Isc] or breakdown) and every diode has to be known.
 *
 * The current and voltage of every cell are then approximated, since the voltage and current through the string are known.
 * Finally, an iterative method is applied to every string to find the correct values.
 *
 * If the pipelines to gnuplot are properly configured, it also represents the resulting I-V characteristic of the panel.
 * But no other final output file is generated.
 *
 * During the process some output files are generated for testing purposes only.
 */
int main (void)
{
 
	cout << "Comencem la prova" << endl;

	//system("pause");

	/* Generaci� de les dades base: cel�les i Strings */

	// The basic solar cell is created
	solar_cell sc;

	// An array of  strings (a panel) is created
	// TODO: Num of strings is hardcoded. It should be an input
	solar_string *panel = new solar_string[numStrings];

	// The strings are initialized and filled with the external data
	for (int j=0; j<numStrings; j++){
		panel[j].inici_corda(sc);
	}

	for (int j=0; j<numStrings; j++){
		// TODO: the input file is hardcoded
		panel[j].update_physical(j, "solar_data_v8.csv");
		// Electrical parameters of the strings are updated according to the external data
		panel[j].update_electrical();
		//panel[j].sort_string(); // eliminat el 7 d'octubre
		panel[j].genLlist();
	}

	/* Comprovaci� de l'ordenaci� d'�ndexs dels grups */

	cout << endl << endl;


	// The main vector where all the info will be organized is created
	vector <inVectorCell> PanelVector;
	// The vector is fulfilled and properly organized
	genPanelVector (PanelVector, panel);

	//cout << PanelVector.size() << endl;


	/*
	 * INFO IN CONSOLE BEGINS
	 */
	system ("pause");

	cout << endl << endl;
	
	for (int k=0; k<PanelVector.size(); ++k){
		cout << "Ara" << endl;
		cout << PanelVector[k].inVectorSum.Isc << endl;
		cout << PanelVector[k].inVectorSum.N << '\t' << PanelVector[k].inVectorSum.SVbr << endl;
		for(map <double, inMapCell>::iterator itMap = PanelVector[k].inVectorDetails.begin(); itMap != PanelVector[k].inVectorDetails.end(); ++itMap){
			cout << itMap->first << '\t';
		}
		cout << endl;
	}
	cout << endl << "LTO = " << genLTO(PanelVector) << endl;
	cout << "LTi:" << endl;

	for (int k=0; k<PanelVector.size(); ++k){
		cout << PanelVector[k].inVectorSum.Limit << endl;
	}

	cout << "LRi:" << endl;
	for (int k=0; k<PanelVector.size(); ++k){
		cout << "Vector " << k << ":" << endl;
		for (map <double, inMapCell>::reverse_iterator itMap = PanelVector[k].inVectorDetails.rbegin(); itMap != PanelVector[k].inVectorDetails.rend(); ++itMap){
			cout << itMap->second.inMapSum.Limit << '\t';
		}
		cout << endl << endl;
	}

	system ("pause");
	/*
	 * INFO IN CONSOLE ENDS
	 */

	/*Generaci� del gr�fic amb els trams de la corba I-V */

	// Creation of an output file to check the process until now. Just for testing
	gencorbiv(PanelVector);

	//system ("pause");

	/* assignaci� de tensions de string*/
	/* despr�s ser� una funci� */

	/*
	 * THIS IS ALSO A TEST
	 * Strings' voltage and currents are assigned given a single hardcoded value of the total voltage in the panel
	 * Then the V and I for every cell are approximated and the result is written in an output file.
	 * TODO: Could be a different function (including the real solution) for specific values of Vpan.
	 */
	double Itotal;
	// Hardcoded Vpan value
	double Vpan = -1.0;
	// Gets the total current in the panel
	Itotal = assignItotal(Vpan, PanelVector);
	// Vector that contains the assignment of voltages to every string
	vector <double> voltVector(numStrings, 0.0); // Vector de tensions de String
	// Calculates the vector
	assignVString(Vpan, PanelVector, voltVector);
	/*
	 * INFO IN CONSOLE BEGINS
	 */
	system ("pause");
	for (int n=0; n<numStrings; ++n){
		cout << "VoltVect = " << '\t' << voltVector[n] << endl;
	}

	cout << "Itotal = " << Itotal << endl;

	cout << endl << endl << endl << endl;
	/*
	 * INFO IN CONSOLE ENDS
	 */

	/* Assignaci� de tensions i corrents als grups de cel�les */

	// Approximation of the cells' current and voltage values
	for (int k = 0; k < numStrings; ++k){
		panel[k].findValues(Itotal, voltVector[k]);
	}

	// Creation of an output file to check the process until now. Just for testing
	genArxcells(panel);

	/*
	 * I-V characteristic creation begins.
	 */
	// Commands to create the characteristic representation
	ofstream curvey, curvez;

	curvey.open("prova_2y.dat", ios::out);
	//curvez.open("prova_2z.dat", ios::out);

	if (!curvey){
		cout << "no hom pot obrir l'arxiu.";
		return(0);
	}

	/*if (!curvez){
		cout << "no hom pot obrir l'arxiu.";
		return(0);
	}*/

	// TODO It not used. Itotal (used before!) is used instead.
	double It;

	// Total number of cells + number of strings is stored in dimX
	int dimX = numStrings + 1; // reservem un espai per a It i els corrents de strin

	for (int i=0; i<numStrings; i++){ //espai per a les tensions de cel�la
		dimX = dimX + panel[i].midaCorda; //cada string pot tenir diferent nombre de cel�les
	}

	cout << "dimX = " << dimX << endl;

	system("pause");

	// Counter of points in the characteristic
	int n = 1;

	// TODO define Vini, Vfin and Step as inputs (or possible inputs as an overloaded constructor)
	for (double vc = -1.60; vc <= 34.0; vc += 0.05){

		// Initialization of the (recycled) voltVector
		for (int i = 0; i < voltVector.size(); ++i){
			voltVector[i] = 0.0;
		}
		// Assignment of currents and voltages to every string
		Itotal = assignItotal(vc, PanelVector);
		assignVString(vc, PanelVector, voltVector);
		// Calculation of the initial approximation
		for (int k = 0; k < numStrings; ++k){
			panel[k].findValues(Itotal, voltVector[k]);
		}

		/*for (int k = 0; k < numStrings; ++k){
				cout << " vc = " << vc << endl;
				cout << "panell: " << k << endl;
				cout << "Idiode = " << panel[k].getIdiode() << '\t' << "Icell = " << panel[k].corda[0].getIcell() << endl;
				for (int j = 0; j < panel[k].midaCorda; ++j){
					cout << "V (" << j << ") = " << panel[k].corda[j].getVcell() << endl;
				}
			cout << endl << endl;
		}
		system("pause");*/

		//system ("pause");

		// Iterative method is called to solve every string
		// TODO: Improvement: overload calc_ivm to include a preferred epsilon (acceptable error)
		Itotal = calc_ivm(panel,vc,dimX, numStrings);
		// Results are prepared to be represented
		curvey << vc << "," <<  Itotal << "," << panel[0].getSvcell() << "," << panel[0].corda[0].getIcell()+panel[0].rm3_c.getIdiode() << ","; //panel[1].getSvcell()*panel[1].corda[0].getIcell() << endl;
		curvey << panel[0].getSvcell() << "," << panel[0].corda[0].getIcell()+panel[0].rm3_c.getIdiode() << endl;
		//curvez << panel[1].corda[0].getVcell() << "," << panel[1].corda[0].getIcell() << "," << panel[1].getSvcell() - panel[1].corda[0].getVcell() << ",";
		//curvez << panel[1].corda[0].getIcell()+panel[1].rm3_c.getIdiode() << endl;

		//double SVns = 0.0;
		//for (int k=1; k<panel[0].midaCorda; k++){
		//	SVns += panel[0].corda[k].getVcell();
		//}
		//curvez << SVns << "," << It << "," << panel[0].corda[1].getIcell() << endl;
	
		//curvez.close();

		n+=1;
	}
	curvey.close();

	// Call to gnuplot to show the results
	representar();

	delete[]panel;
	return(0);
}
