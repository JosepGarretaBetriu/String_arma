#include "solar_string.h"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <iostream>
using namespace std;

//#define Iscr 3.798 // 5.0
// TODO deprecated, all of them
#define Vocr 0.60
#define a 0.0003
#define B -0.0023
// TODO deprecated
#define roc 0.031
//#define roc 0.048
#define Tcr 25.0
#define Gr 1000.0

// Overloading of the operators
bool operator < (const TableStr &T1, const TableStr &T2 )
{
	return ( T2.IscGrup > T1.IscGrup);
}
bool operator == (const TableStr &T1, const TableStr &T2) //operador definit per a les comparacions
{
	return (T1.IscGrup == T2.IscGrup);
}

void solar_string::findOption1 (double &_Iin,double &_Vin)
{
	/*
	 * The idea here is that, if there is no diode, any group can either be active, non-active or in breakdown.
	 * The CellsGr structure is ordered so first come the breakdown ones, then the active and finally the non-active.
	 * First we identify if there's an active one.
	 * Then we assign Voc to the non-active groups and Vbr to the breakdown groups.
	 * Finally we find the voltage corresponding to the active group, if there's one.
	 */
	list <TableStr>::iterator itL; //iterador directe a la llista de grups del String per a Voc
	list <TableStr>::iterator itLac; //iterador per al grup actiu de cada string

	double SumVoc = 0; // acumulador de Voc
	double SumVbr = 0; // acumulador de Vbr
	double SumVact = 0; // tensi� total de cel�les actives

	// Created to introduce the value of Iin in the iterator itL
	TableStr cG;
	cG.IscGrup = _Iin;
	// Moves the iterator to the first group that is not in breakdown (either active or not)
	itL = lower_bound(this->CellsGr.begin(), this->CellsGr.end(), cG);
	//cout << "lower = " << itL->IscGrup << endl;

	// From this point, in reverse direction, the breakdown groups will be evaluated
	list <TableStr>::reverse_iterator ritL(itL); //iterador invers a la llista de grups per a Vbr, resta d'1 posici� (per defecte)

	/*
	 * Looks for an active group of cells (imposing their Isc to the rest of the panel). If it exists, the itLac iterator
	 * is assigned to it (a single group can be active in a string, and since CellsGr is ordered, it will be the first one)
	 */
	if (itL != this->CellsGr.end()) //comprovem que no hi hagi cap problema de sobreeiximent
	{
		if (itL->IscGrup == _Iin) // si hi ha igualtat => grup actiu
		{
			itLac = itL;// hi ha un grup de cel�les actives
			advance(itL,1); //avancem 1 posici� l'iterador per evitar el grup de cel�les actives
		}
		else
		{
			itLac = this->CellsGr.end(); // assignem la posici� end a itLac (sense cel�les actives)
		}
	}
	else
	{
		itLac = this->CellsGr.end(); // assignem la posici� end a itLac si hi ha sobreeiximent
	}

	/*
	 * If there are non-active groups, the approximation is that their voltage will be their Voc
	 * The current will be the one imposed Iin
	 */
	while (itL != this->CellsGr.end()) //assignaci� de Voc a les cel�les tallades
	{
		SumVoc += itL->SVoc; //acumulaci� de Voc
		for (int k = 0; k < itL->index.size(); ++k) // assignaci� de Voc a cada cel�la del grup
		{
			this->corda[itL->index[k]].setVcell(this->corda[itL->index[k]].getVoc());
			this->corda[itL->index[k]].setIcell(_Iin);
		}
		advance(itL,1);
	}
	//cout << "SumVoc = " << SumVoc << endl;

	/*
	 * If there are breakdown groups, the approximation is that their voltage will be their Vbr
	 * The current will be the one imposed Iin
	 */
	// TODO: how does it distinguish between non-active and breakdown groups?
	while (ritL != this->CellsGr.rend()) //assignaci� de Vbr a les cel�les trencades
	{
		SumVbr += ritL->SVbr; //acumulaci� de Vbr
		for (int k = 0; k < ritL->index.size(); ++k) // assignaci� de Vbr a cada cel�la del grup
		{
			this->corda[ritL->index[k]].setVcell(this->corda[ritL->index[k]].getVbreak());
			this->corda[ritL->index[k]].setIcell(_Iin);
		}
		advance(ritL,1);
	}

	/*
	 * Being the total voltage in the string and the voltage in the rest of the groups known, the voltage in the active group
	 * (if present) can be found
	 * The current will match the one imposed Iin
	 */
	if (itLac != this->CellsGr.end())
	{
		SumVact = _Vin - SumVoc - SumVbr;
		for (int k = 0; k < itLac->index.size(); ++k) // assignaci� de Vact a cada cel�la del grup
		{
			this->corda[itLac->index[k]].setVcell(SumVact/itLac->index.size());
			this->corda[itLac->index[k]].setIcell(_Iin);
		}
	}
	// Since there is no diode, there's no current through it
	this->rm3_c.setIdiode(0.0); // d�ode inexistent o b� tallat
}

void solar_string::findOption2 (double &_Iin,double &_Vin)
{
	/*
	 * The idea here is that, if there is a diode, there will be an active group, but can also have non-active or breakdown groups.
	 * The CellsGr structure is ordered so first come the breakdown ones, then the active and finally the non-active.
	 * First we identify the active one.
	 * Then we assign Voc to the non-active groups and Vbr to the breakdown groups.
	 * Finally we find the voltage corresponding to the active group.
	 */
	list <TableStr>::iterator itL; //iterador directe a la llista de grups del String per a Voc
	list <TableStr>::iterator itLac; //iterador per al grup actiu de cada string

	double Iwork;

	/*
	 * Moves the iterator through the breakdown groups (if any) until the active group
	 */
	itL = this->CellsGr.begin();
	while (itL != this->CellsGr.end()){
		if (itL->SVbrx>itL->SVbr) break;
		advance(itL,1);
	}
	// itLac keeps the position of the active group
	itLac = itL;
	// Updates the current in the string to the one of the active group.
	Iwork = itLac->IscGrup; // corrent de les cel�les

	// A reverse iterator will update the breakdown groups from this point
	list <TableStr>::reverse_iterator ritL(itL); //iterador invers a la llista de grups per a Vbr, resta d'1 posici� (per defecte)
	
	// Move one position further to skip the active group
	advance(itL,1); // anirem cap al final assignant-hi tensions Voc a les cel�les

	/*
	 * If there are non-active groups, the approximation is that their voltage will be their Voc
	 * The current will be the one of the active group
	 */
	while (itL != this->CellsGr.end()){ //assignaci� de Voc a les cel�les tallades
		for (int k = 0; k < itL->index.size(); ++k){ // assignaci� de Voc a cada cel�la del grup
			this->corda[itL->index[k]].setVcell(this->corda[itL->index[k]].getVoc());
			this->corda[itL->index[k]].setIcell(Iwork);
		}
		advance(itL,1);
	}

	/*
	 * If there are breakdown groups, the approximation is that their voltage will be their Vbr
	 * The current will be the one of the active group
	 */
	while (ritL != this->CellsGr.rend()){ //assignaci� de Vbr a les cel�les trencades
		for (int k = 0; k < ritL->index.size(); ++k){ // assignaci� de Vbr a cada cel�la del grup
			this->corda[ritL->index[k]].setVcell(this->corda[ritL->index[k]].getVbreak());
			this->corda[ritL->index[k]].setIcell(Iwork);
			}
		advance(ritL,1);
	}

	/*
	 * Being the total voltage in the string and the voltage in the rest of the groups known, the voltage in the active group
	 * can be found
	 * The current will be its Isc
	 */
	if (itLac != this->CellsGr.end()){
		for (int k = 0; k < itLac->index.size(); ++k){ // assignaci� de Vact a cada cel�la del grup
			this->corda[itLac->index[k]].setVcell(itLac->SVbrx/itLac->index.size());
			this->corda[itLac->index[k]].setIcell(Iwork);
			}
	}
	// The diode will be driving the difference between the Isc of the active group and the current in the panel
	this->rm3_c.setIdiode(_Iin-Iwork); // difer�ncia de corrents entre total i cel�les
}

solar_string :: solar_string (void)
{
	// TODO: Hardcoded values.
	ambDiode = true;
	isDiodeOn = true;
	Vdiode = 1.5; //0.95
	midaCorda = 12;
	corda = new solar_cell[midaCorda];
}
solar_string :: ~solar_string (void)
{
	delete [] corda;
}

int solar_string :: getambDiode (void)
{
	return (ambDiode);
}

void solar_string :: setSvoc (void)
{
	double sumVoc = 0;
	for (int k = 0; k < midaCorda; k++){
			sumVoc += corda[k].getVoc();
	}
	Svoc = sumVoc;
}
void solar_string :: setSvbr (void)
{
	double sumVbr = 0;
	for (int k = 0; k < midaCorda; k++){
			sumVbr += corda[k].getVbreak();
	}
	Svbreak = sumVbr;
}
void solar_string :: setVstring (double _Vstring)
{
	this->Vstring = _Vstring;
}
void solar_string :: setSvcell (void)
{
	double sumVcell = 0;
	for (int k = 0; k < midaCorda; k++){
			sumVcell += corda[k].getVcell();
	}
	Svcell = sumVcell;
}
void solar_string :: update_physical (int m, const char *filename)
{
	// TODO: This 81 is pretty hardcoded. Function to recognize the file format should be implemented
	char line[81];
	double Tcx, Gx;

	ifstream myfile;
	myfile.open(filename, ios::in);

	if (myfile.is_open())
		{
			//Loads the entire file
			for (int lineno = 0; lineno < m*(midaCorda+1); lineno++){
				myfile.getline(line,81, '\n');
			}
			myfile.getline(line,81, '\n');
			ambDiode = atoi(line); // TODO: atoi is a deprecated function
			for (int k = 0; k < midaCorda; k++){
				myfile.getline (line,81, ';');
				Gx = atof(line);
				myfile.getline (line,81, '\n');
				Tcx = atof(line);
				corda[k].setG(Gx);
				corda[k].setTc(Tcx);
			}
	}
}
void solar_string :: update_electrical (void)
{
	for (int i = 0; i < midaCorda; i++){
		corda[i].setIndex(i);
		corda[i].setIsc();
		corda[i].setIph();
		corda[i].setVoc();
		corda[i].setIo();
		corda[i].setVbreak(Vbr);//afegit

	}
	setSvoc();
	setSvbr();
}
void solar_string :: inici_corda (solar_cell sc)
{
	// Fills the array with copies of the provided solar_cell object
	std::fill(corda, corda+midaCorda, sc);
	setSvoc();
}

/*Assignaci� de tensions de ruptura dels grups de cel�les segons l'estat dels d�odes de bypass */
/*
 * Calculates the breakdown voltage calculated in the group
 */
void solar_string::setSVbrx (void){//Tensi� de ruptura de tot el grup (calculada sobre el string)
	// If there's no diode Vbrx equals Vbr, since all the cells in the string will eventually suffer breakdown
	if(!this->getambDiode()){
		for(list<TableStr>::iterator itL = this->CellsGr.begin(); itL != this->CellsGr.end(); ++itL){
			itL->SVbrx = itL->SVbr; //si no hi ha d�ode tots dos valors s�n iguals
		}
	/*
	 * If there's a diode, Vbrx can equal Vbr if the cell breaks before the diode conducts or a different value if the diode
	 * conducts first
	 */
	} else { //en cas d'haver-hi d�ode Vbr pot �sser un altre valor
				double SumVbrPrev = 0.0; //acumulador de tensions de ruptura de grups previs
				double SumVocPost = 0.0; //acumulador de tensions de tall de grups posteriors
				double SVbrx = 0.0;
				for(list<TableStr>::iterator itLPost = this->CellsGr.begin(); itLPost != this->CellsGr.end(); ++itLPost){
					SumVocPost += itLPost->SVoc; //acumulaci� de tensions de tall;
				}
				for(list<TableStr>::iterator itL = this->CellsGr.begin(); itL != this->CellsGr.end(); ++itL){
					SumVocPost -= itL->SVoc; // substracci� de la pr�pia SVoc del grup
					SVbrx = -this->getVdiode()-SumVbrPrev-SumVocPost;
					if (SVbrx < itL->SVbr){
						itL->SVbrx = itL->SVbr; //limitaci� del valor m�nim de SVbrx a SVbr
					}else{
						itL->SVbrx = SVbrx;
					}
					SumVbrPrev += itL->SVbr; // suma de la pr�pia SVbr del grup
				}
	}
}

/* Detectar grups de cel�les amb mateix Isc */
bool SameCurrent(TableStr first, TableStr second)
{
	return (first.IscGrup == second.IscGrup);
}
void solar_string :: AccLlist(void)
{
	list<TableStr>::iterator itr = CellsGr.begin();
	list<TableStr>::iterator itrx= CellsGr.begin();
	list<TableStr>::iterator itry = itr;
	// TODO: Simplify loop (for)
	bool stop = false;
	while (!stop) {
		if (itr == CellsGr.end()){
			stop = true;
		} else {
			advance(itry,1);
			itry = find(itry, CellsGr.end(), *itr);
			if (itry != CellsGr.end()){
				itr->index.push_back(itry->index.front());
				itr->SVbr += itry->SVbr;
				itr->SVbrx += itry->SVbrx;
				itr->SVoc += itry->SVoc;
				itrx = itry; 
				}else{
					advance(itrx,1);
					itr = itrx;
					itry = itr;
			}
		}
	}
}
void solar_string :: genLlist(void) 
{
	TableStr Cell;
	// Crea tants elements a la llista CellsGr com celles hi ha
	// Creates an entry in CellsGr for every PV cell in the corda array
	for (int i = 0; i < midaCorda; i++){
		Cell.index.push_back(corda[i].getIndex()); //hi posem l'�ndex de la cel�la segons la seva posici� f�sica
		Cell.IscGrup = corda[i].getIsc();
		Cell.SVbr = corda[i].getVbreak();
		Cell.SVbrx = 0.0;
		Cell.SVoc = corda[i].getVoc();
		CellsGr.push_back(Cell);
		Cell.index.erase(Cell.index.begin());
	}
	// Sorts the list
	this->CellsGr.sort(); // nou 7 octubre
	// Groups the cells with the same Isc under the first identical element
	this->AccLlist();
	//this->CellsGr.erase(unique(this->CellsGr.begin(), this->CellsGr.end(), SameCurrent), this->CellsGr.end());
	// Delete the repeated elements, keeping the first one
	this->CellsGr.unique(SameCurrent);
	// Calculates the SVbrx of the string
	this->setSVbrx();
}	
double solar_string :: getIscmin (void)
{
	return (corda[0].getIsc());
}
double solar_string :: getSvoc (void)
{
	return (Svoc);
}
double solar_string :: getVstring (void)
{
	return (Vstring);
}
double solar_string :: getSvbr (void)
{
	return (Svbreak);
}
double solar_string :: getSvcell (void)
{
	return (Svcell);
}
/*void solar_string :: setIdiode (double Id)
{
	Idiode = Id;
}*/
/*void solar_string :: setIt (double _It)
{
	It = _It;
}*/
/*double solar_string :: getIdiode (void)
{
	return (Idiode);
}*/
double solar_string :: getVdiode (void)
{
	return (Vdiode);
}
void solar_string :: findValues (double &Iin, double &Vin)
{
	// TODO the iterators are not used
	list <TableStr>::iterator itL; //iterador directe a la llista de grups del String per a Voc
	list <TableStr>::iterator itLac; //iterador per al grup actiu de cada string

	//cout << "Iin = " << Iin << endl;
	//cout << "Vin = " << Vin << endl;
	
	// TODO: findoption1 and findoption2 could be easily  joined into a single function
	switch (this->getambDiode()){ // d�ode ok?
		case false: // d�ode no ok
			{
				this->findOption1(Iin,Vin);
				break;
			}
		case true:
			if (Vin > -this->getVdiode()){
				this->findOption1(Iin,Vin); // el d�ode �s tallat
				//cout << "Vdiode = " << this->getVdiode() << endl;
			}else{
				this->findOption2(Iin,Vin);
				//cout << "Vdiode = " << this->getVdiode() << '\t' << "Vin = " << Vin << endl;
			}
			break;
	}
}
double  solar_string :: minim(double *Fa)
{
	double mini = fabs(Fa[0]);
	for (int i=1;i<midaCorda;i++){
		if (fabs(Fa[i])<mini){
			mini = fabs(Fa[i]);
		}
	}
	return(mini);
}
