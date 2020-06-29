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

#include "pv_string.h"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace stringarma{

// Knee voltage of the bypass diode [V]
#define VOLTAGE_KNEE_DIODE_REF 1.5

using namespace std;

// Overloading of the operators
bool operator < (const TotalsOfCellsGroup &T1, const TotalsOfCellsGroup &T2 )
{
	return ( T2.current_shortcut > T1.current_shortcut);
}
bool operator == (const TotalsOfCellsGroup &T1, const TotalsOfCellsGroup &T2)
{
	return (T1.current_shortcut == T2.current_shortcut);
}

void solar_string::findInitialStateWithoutDiode (double &_Iin,double &_Vin)
{
	/*
	 * The CellsGr structure is ordered so first come the breakdown ones, then the active and finally the non-active.
	 * First we identify if there's an active one.
	 * Then we assign Voc to the non-active groups and Vbr to the breakdown groups.
	 * Finally, since the voltage between the terminals of the string is known, we find the voltage corresponding
	 * to the active group, if there's one.
	 */
	list <TotalsOfCellsGroup>::iterator itL;
	list <TotalsOfCellsGroup>::iterator itLac;

	double SumVoc = 0;
	double SumVbr = 0;
	double SumVact = 0; // total voltage of active cells

	// Created to introduce the value of Iin in the iterator itL
	TotalsOfCellsGroup cG;
	cG.current_shortcut = _Iin;
	// Moves the iterator to the first group that is not in breakdown (either active or not)
	itL = lower_bound(this->groupsByCurrentShortcut.begin(), this->groupsByCurrentShortcut.end(), cG);

	// From this point, in reverse direction, the breakdown groups will be evaluated
	list <TotalsOfCellsGroup>::reverse_iterator ritL(itL);

	/*
	 * Looks for an active group of cells (imposing their Isc to the rest of the panel). If it exists, the itLac iterator
	 * is assigned to it (a single group can be active in a string, and since CellsGr is ordered, it will be the first one)
	 */
	if (itL != this->groupsByCurrentShortcut.end())
	{
		// Checks if its an active group
		if (itL->current_shortcut == _Iin)
		{
			itLac = itL;
			// To avoid the active group
			advance(itL,1);
		}
		else
		{
			itLac = this->groupsByCurrentShortcut.end();
		}
	}
	else
	{
		itLac = this->groupsByCurrentShortcut.end();
	}

	/*
	 * If there are non-active groups, the approximation is that their voltage will be their Voc
	 * The current will be the one imposed Iin
	 */
	while (itL != this->groupsByCurrentShortcut.end())
	{
		SumVoc += itL->sum_voltage_open_circuit;
		for (int k = 0; k < itL->index.size(); ++k)
		{
			this->cells_array[itL->index[k]].setVoltageCell(this->cells_array[itL->index[k]].getVoltageOpenCircuit());
			this->cells_array[itL->index[k]].setCurrentCell(_Iin);
		}
		advance(itL,1);
	}

	/*
	 * If there are breakdown groups, the approximation is that their voltage will be their Vbr
	 * The current will be the one imposed Iin
	 */
	while (ritL != this->groupsByCurrentShortcut.rend())
	{
		SumVbr += ritL->sum_voltage_breakdown;
		for (int k = 0; k < ritL->index.size(); ++k)
		{
			this->cells_array[ritL->index[k]].setVoltageCell(this->cells_array[ritL->index[k]].getVoltageBreakdown());
			this->cells_array[ritL->index[k]].setCurrentCell(_Iin);
		}
		advance(ritL,1);
	}

	/*
	 * Being the total voltage in the string and the voltage in the rest of the groups known, the voltage in the active group
	 * (if present) can be found
	 * The current will match the one imposed Iin
	 */
	if (itLac != this->groupsByCurrentShortcut.end())
	{
		SumVact = _Vin - SumVoc - SumVbr;
		for (int k = 0; k < itLac->index.size(); ++k)
		{
			this->cells_array[itLac->index[k]].setVoltageCell(SumVact/itLac->index.size());
			this->cells_array[itLac->index[k]].setCurrentCell(_Iin);
		}
	}
	// Since there is no diode, there's no current through it
	this->diode_bypass.setCurrentDiode(0.0);
}

void solar_string::findInitialStateWithDiode (double &_Iin)
{
	/*
	 * The CellsGr structure is ordered so first come the breakdown ones, then the active and finally the non-active.
	 * First we identify the active one.
	 * Then we assign Voc to the non-active groups and Vbr to the breakdown groups.
	 * Finally, since the voltage between the terminals of the string equals the diode's voltage,
	 * we find the voltage corresponding to the active group.
	 */
	list <TotalsOfCellsGroup>::iterator itL;
	list <TotalsOfCellsGroup>::iterator itLac;

	double Iwork;

	/*
	 * Moves the iterator through the breakdown groups (if any) until the active group (if any)
	 */
	itL = this->groupsByCurrentShortcut.begin();
	while (itL != this->groupsByCurrentShortcut.end()){
		if (itL->sum_voltage_breakdown_in_group>itL->sum_voltage_breakdown) break;
		advance(itL,1);
	}
	// itLac keeps the position of the active group
	itLac = itL;
	// Updates the current in the string to the one of the active group.
	Iwork = itLac->current_shortcut;

	// A reverse iterator will update the breakdown groups from this point
	list <TotalsOfCellsGroup>::reverse_iterator ritL(itL);

	// Move one position further to skip the active group
	advance(itL,1);

	/*
	 * If there are non-active groups, the approximation is that their voltage will be their Voc
	 * The current will be the one of the active group
	 */
	while (itL != this->groupsByCurrentShortcut.end()){
		for (int k = 0; k < itL->index.size(); ++k){
			this->cells_array[itL->index[k]].setVoltageCell(this->cells_array[itL->index[k]].getVoltageOpenCircuit());
			this->cells_array[itL->index[k]].setCurrentCell(Iwork);
		}
		advance(itL,1);
	}

	/*
	 * If there are breakdown groups, the approximation is that their voltage will be their Vbr
	 * The current will be the one of the active group
	 */
	while (ritL != this->groupsByCurrentShortcut.rend()){
		for (int k = 0; k < ritL->index.size(); ++k){
			this->cells_array[ritL->index[k]].setVoltageCell(this->cells_array[ritL->index[k]].getVoltageBreakdown());
			this->cells_array[ritL->index[k]].setCurrentCell(Iwork);
			}
		advance(ritL,1);
	}

	/*
	 * Being the total voltage in the string and the voltage in the rest of the groups known, the voltage in the active group
	 * can be found
	 * The current will be its Isc
	 */
	if (itLac != this->groupsByCurrentShortcut.end()){
		for (int k = 0; k < itLac->index.size(); ++k){
			this->cells_array[itLac->index[k]].setVoltageCell(itLac->sum_voltage_breakdown_in_group/itLac->index.size());
			this->cells_array[itLac->index[k]].setCurrentCell(Iwork);
			}
	}
	// The diode will be driving the difference between the Isc of the active group and the current in the panel
	this->diode_bypass.setCurrentDiode(_Iin-Iwork);
}

solar_string :: solar_string (void)
{
	with_diode = true;
	voltage_knee_diode = VOLTAGE_KNEE_DIODE_REF;
	string_size = 0;
}
solar_string :: ~solar_string (void)
{
	delete [] cells_array;
}

int solar_string :: getWithDiode (void)
{
	return (with_diode);
}

void solar_string :: setSumVoltageOpenCircuit (void)
{
	double sumVoc = 0;
	for (int k = 0; k < string_size; k++){
			sumVoc += cells_array[k].getVoltageOpenCircuit();
	}
	sum_voltage_open_circuit = sumVoc;
}

void solar_string :: setSumVoltageBreakdown (void)
{
	double sumVbr = 0;
	for (int k = 0; k < string_size; k++){
			sumVbr += cells_array[k].getVoltageBreakdown();
	}
	sum_voltage_breakdown = sumVbr;
}

void solar_string :: setVoltageString (double _Vstring)
{
	this->voltage_string = _Vstring;
}

void solar_string :: setSumVoltageAllCells (void)
{
	double sumVcell = 0;
	for (int k = 0; k < string_size; k++){
			sumVcell += cells_array[k].getVoltageCell();
	}
	sum_voltage_all_cells = sumVcell;
}

void solar_string :: setVoltageDiode (double _Vdiode)
{
	this->voltage_knee_diode = _Vdiode;
}

void solar_string :: updateStringsData (std::pair<bool,std::vector<std::pair<double,double>>> &string_input, SolarCell &sc)
{
	// Creates SolarString objects according to the info provided
	string_size=string_input.second.size();
	cells_array = new SolarCell[string_size];
	std::fill(cells_array, cells_array+string_size, sc);
	setSumVoltageOpenCircuit();

	with_diode = string_input.first;

	for(int k = 0; k < string_size; k++)
	{
		cells_array[k].setIrradiance(string_input.second[k].first);
		cells_array[k].setTemperatureCell(string_input.second[k].second);
	}

	// Updates the electrical parameters of the cells according to the temperature and irradiance
	updateElectricalParameters();
	// Sorts and groups the cells by their shortcut current
	updateGroupsByShortcutCurrent();
}

void solar_string :: updateElectricalParameters (void)
{
	for (int i = 0; i < string_size; i++){

		cells_array[i].setIndex(i);
		cells_array[i].setCurrentShortcut();
		cells_array[i].setCurrentPhotogenerated();
		cells_array[i].setVoltageOpenCircuit();
		cells_array[i].setCurrentReverseSaturation();
		cells_array[i].setVoltageBreakdown(VOLTAGE_BREAKDOWN_REF);//afegit

	}
	setSumVoltageOpenCircuit();
	setSumVoltageBreakdown();
}

/*
 * Calculates the breakdown voltage calculated in the group
 */
void solar_string::setSumVolageBreakdownInGroup (void){
	// If there's no diode Vbrx equals Vbr, since all the cells in the string will eventually suffer breakdown
	if(!this->getWithDiode()){
		for(list<TotalsOfCellsGroup>::iterator itL = this->groupsByCurrentShortcut.begin(); itL != this->groupsByCurrentShortcut.end(); ++itL){
			itL->sum_voltage_breakdown_in_group = itL->sum_voltage_breakdown;
		}
	/*
	 * If there's a diode, Vbrx can equal Vbr if the cell breaks before the diode conducts or a different value if the diode
	 * conducts first
	 */
	} else {
				double SumVbrPrev = 0.0;
				double SumVocPost = 0.0;
				double SVbrx = 0.0;
				for(list<TotalsOfCellsGroup>::iterator itLPost = this->groupsByCurrentShortcut.begin(); itLPost != this->groupsByCurrentShortcut.end(); ++itLPost){
					SumVocPost += itLPost->sum_voltage_open_circuit;
				}
				for(list<TotalsOfCellsGroup>::iterator itL = this->groupsByCurrentShortcut.begin(); itL != this->groupsByCurrentShortcut.end(); ++itL){
					SumVocPost -= itL->sum_voltage_open_circuit;
					SVbrx = -this->getVoltageDiode()-SumVbrPrev-SumVocPost;
					if (SVbrx < itL->sum_voltage_breakdown){
						itL->sum_voltage_breakdown_in_group = itL->sum_voltage_breakdown;
					}else{
						itL->sum_voltage_breakdown_in_group = SVbrx;
					}
					SumVbrPrev += itL->sum_voltage_breakdown;
				}
	}
}

bool SameCurrent(TotalsOfCellsGroup first, TotalsOfCellsGroup second)
{
	return (first.current_shortcut == second.current_shortcut);
}
void solar_string :: sortGroupsByShortcutCurrent(void)
{
	list<TotalsOfCellsGroup>::iterator itr = groupsByCurrentShortcut.begin();
	list<TotalsOfCellsGroup>::iterator itrx= groupsByCurrentShortcut.begin();
	list<TotalsOfCellsGroup>::iterator itry = itr;
	bool stop = false;
	while (!stop) {
		if (itr == groupsByCurrentShortcut.end()){
			stop = true;
		} else {
			advance(itry,1);
			itry = find(itry, groupsByCurrentShortcut.end(), *itr);
			if (itry != groupsByCurrentShortcut.end()){
				itr->index.push_back(itry->index.front());
				itr->sum_voltage_breakdown += itry->sum_voltage_breakdown;
				itr->sum_voltage_breakdown_in_group += itry->sum_voltage_breakdown_in_group;
				itr->sum_voltage_open_circuit += itry->sum_voltage_open_circuit;
				itrx = itry;
				}else{
					advance(itrx,1);
					itr = itrx;
					itry = itr;
			}
		}
	}
}
void solar_string :: updateGroupsByShortcutCurrent(void)
{
	TotalsOfCellsGroup Cell;
	// Creates an entry for every PV cell in the corda array
	for (int i = 0; i < string_size; i++){
		Cell.index.push_back(cells_array[i].getIndex());
		Cell.current_shortcut = cells_array[i].getCurrentShortcut();
		Cell.sum_voltage_breakdown = cells_array[i].getVoltageBreakdown();
		Cell.sum_voltage_breakdown_in_group = 0.0;
		Cell.sum_voltage_open_circuit = cells_array[i].getVoltageOpenCircuit();
		groupsByCurrentShortcut.push_back(Cell);
		Cell.index.erase(Cell.index.begin());
	}
	// Sorts the list
	this->groupsByCurrentShortcut.sort();
	// Groups the cells with the same Isc under the first identical element
	this->sortGroupsByShortcutCurrent();
	// Delete the repeated elements, keeping the first one
	this->groupsByCurrentShortcut.unique(SameCurrent);
	// Calculates the SVbrx of the string
	this->setSumVolageBreakdownInGroup();
}
double solar_string :: getMinimCurrentShortcut (void)
{
	return (cells_array[0].getCurrentShortcut());
}
double solar_string :: getSumVoltageOpenCircuit (void)
{
	return (sum_voltage_open_circuit);
}
double solar_string :: getVoltageString (void)
{
	return (voltage_string);
}
double solar_string :: getSumVoltageBreakdown (void)
{
	return (sum_voltage_breakdown);
}
double solar_string :: getSumVoltageAllCells (void)
{
	return (sum_voltage_all_cells);
}
double solar_string :: getVoltageDiode (void)
{
	return (voltage_knee_diode);
}
void solar_string :: findInitialState (double &Iin, double &Vin)
{
	switch (this->getWithDiode()){
		case false:
			{
				this->findInitialStateWithoutDiode(Iin,Vin);
				break;
			}
		case true:
			if (Vin > -this->getVoltageDiode()){
				this->findInitialStateWithoutDiode(Iin,Vin);
			}else{
				this->findInitialStateWithDiode(Iin);
			}
			break;
	}
}
double  solar_string :: minimumInArray(double *Fa)
{
	double mini = fabs(Fa[0]);
	for (int i=1;i<string_size;i++){
		if (fabs(Fa[i])<mini){
			mini = fabs(Fa[i]);
		}
	}
	return(mini);
}

}
