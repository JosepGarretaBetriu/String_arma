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
 *	This file uses the unmodified Armadillo C++ Linear Algebra Library.
 *	The information below belong to its license:
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

#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include "pv_solver.h"
#include <armadillo>

using namespace std;
using namespace arma;

namespace stringarma{

bool equalIsc (const SameIshortcutGroup &iVC1, const SameIshortcutGroup &iVC2 )
{
	return iVC1.sum_same_i_shortcut_group.current_shortcut == iVC2.sum_same_i_shortcut_group.current_shortcut;
}

Col<double> loadInitialValues(solar_string *st, int n, int nS)
{
	// Fills the column with the voltage of the cells, the total current and the currents in every string
	Col<double> Z = zeros(n);
	int relatiu = 0;
	for (int i=0; i<nS; i++){
		for (int j=0; j<st[i].string_size; j++){
			Z(relatiu+j) = st[i].cells_array[j].getVoltageCell();
			}
			relatiu += st[i].string_size;
		}
	Z(relatiu) = st[0].cells_array[0].getCurrentCell() + st[0].diode_bypass.getCurrentDiode();
	relatiu += 1;
	for (int i=0; i<nS; i++){
		Z(relatiu+i) = st[i].cells_array[0].getCurrentCell();
		}
	return Z;
}

void SolarSolver :: setMaxIterations(int maxIt)
{
	try
	{
		max_iterations = maxIt;
	}
	catch(...)
	{
		std::cout << "Error when modifying the MaxIterations parameter." << endl;
	}
}

void SolarSolver :: setEpsilon(double _epsilon)
{
	try
	{
		epsilon = _epsilon;
	}
	catch(...)
	{
		std::cout << "Error when modifying the epsilon parameter." << endl;
	}
}

int SolarSolver :: getMaxIterations(void)
{
	return(max_iterations);
}

double SolarSolver :: getEpsilon(void)
{
	return(epsilon);
}

void SolarSolver::generatePanelVector (){
	multimap <pair<double,double>, SameIshortcutAndVbreakdownGroup, Classcomp> MMPanel;
	SameIshortcutGroup iVC;
	/*
	 * Fills the MMPanel multimap with the info in the solar_string objects in panel and orders it
	 * by groups of cells with the same breakdown current and voltage
	 */
	retrieveDataFromStringArray(MMPanel);
	// Iterates the multimap to update the Isc and initialize the rest of the values.
	// Every group is a new entry in the vector of inVectorCell
	for (multimap<pair<double,double>,SameIshortcutAndVbreakdownGroup, Classcomp>::iterator
			itmap = MMPanel.begin(); itmap != MMPanel.end(); ++itmap)
	{
		iVC.sum_same_i_shortcut_group.current_shortcut = itmap->first.first;
		iVC.sum_same_i_shortcut_group.group_size = 0;
		iVC.sum_same_i_shortcut_group.sum_voltage_breakdown_in_group = 0;
		iVC.sum_same_i_shortcut_group.sum_voltage_open_circuit_all_cells = 0;
		iVC.sum_same_i_shortcut_group.sum_voltage_open_circuit_non_active_cells = 0;
		panel_vector.push_back(iVC);
	}
	vector <SameIshortcutGroup>::iterator it;
	// If there is any group with the same Isc, all but the first one are removed
	it = unique(panel_vector.begin(),panel_vector.end(), equalIsc);
	panel_vector.resize(distance(panel_vector.begin(),it));
	// The information in the multimap object is migrated to the panel_vector
	multimap<pair<double,double>,SameIshortcutAndVbreakdownGroup, Classcomp>::iterator itmap = MMPanel.begin();
	for (int k=0; k<panel_vector.size(); ++k){
		// Fulfills the inVectorDetail structs with a pair composed by a double type with the Vbrx of the group
		// and the corresponding inMapCell struct
		while(itmap->first.first == panel_vector[k].sum_same_i_shortcut_group.current_shortcut){
			panel_vector[k].detailed_same_i_shortcut_group.insert(make_pair(itmap->first.second,itmap->second));
			advance(itmap,1);
			if (itmap == MMPanel.end()) break;
		}
		// Fulfills the inVectorSum struct
		for(map <double, SameIshortcutAndVbreakdownGroup>::iterator itMap
				= panel_vector[k].detailed_same_i_shortcut_group.begin();
				itMap != panel_vector[k].detailed_same_i_shortcut_group.end(); ++itMap)
		{
			panel_vector[k].sum_same_i_shortcut_group.group_size
				+= itMap->second.sum_same_i_shortcut_and_v_breakdown_group.group_size;

			panel_vector[k].sum_same_i_shortcut_group.sum_voltage_breakdown_in_group
				+= itMap->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_breakdown_in_group;

			panel_vector[k].sum_same_i_shortcut_group.sum_voltage_open_circuit_all_cells
				+= itMap->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_open_circuit_all_cells;

			panel_vector[k].sum_same_i_shortcut_group.sum_voltage_open_circuit_non_active_cells
				+= itMap->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_open_circuit_non_active_cells;
		}
	}
	// Finds the voltage limits where changes in the current or voltage distribution takes place
	// First check the upper limit of voltage where the cells are generating
	double LTO = findMaxVoltageLimit();
	// Then look for changes in the active groups of cells
	findVoltageLimitsForChangesInCurrent(LTO);
	// The diodes can enter conducting state without changing the active group.
	// Here the voltage where that happens is find
	findVoltageLimitsForChangesInVoltage();
}

void SolarSolver::retrieveDataFromStringArray (multimap <pair<double,double>,
		SameIshortcutAndVbreakdownGroup, Classcomp>&MultiMapElDets)
{
	double Iscx, Vbrx;
	CellsGroup gS;
	SameIshortcutAndVbreakdownGroup iMC;
	pair<double,double> clau;
	// Iterates all the strings in the panel.
	// Since the groups are ordered, the ones with lesser Isc will be first (breakdown,actives and non-active)
	for (int j=0; j<number_strings; j++){
		// Iterator of the groups of cells with the same parameters in the string
		list<TotalsOfCellsGroup>::iterator itList = string_array[j].groupsByCurrentShortcut.begin();
		while (itList !=string_array[j].groupsByCurrentShortcut.end()){

			// Stores locally all the values. It takes all the cells in the group as 'active cells'
			Iscx = itList->current_shortcut;
			gS.group_size = itList->index.size();
			gS.sum_voltage_open_circuit_all_cells = itList->sum_voltage_open_circuit;
			gS.sum_voltage_open_circuit_non_active_cells = 0.0;
			gS.sum_voltage_breakdown_in_group = itList->sum_voltage_breakdown_in_group;

			Vbrx = itList->sum_voltage_breakdown_in_group/gS.group_size;
			// The key of the map
			clau = make_pair(Iscx,Vbrx);

			// If the group analyzed has the bypass diode conducting (or all the cells in breakdown) end the while loop.
			if (itList->sum_voltage_breakdown_in_group > itList->sum_voltage_breakdown) break;

			iMC.detailed_same_i_shortcut_and_v_breakdown_group.push_back(make_pair(j,gS));
			iMC.sum_same_i_shortcut_and_v_breakdown_group = gS;
			MultiMapElDets.insert(make_pair(clau,iMC));
			iMC.detailed_same_i_shortcut_and_v_breakdown_group.clear();

			advance(itList,1);
		}
		// Only enters this loop if the iterator is not at the end --> If there's group/s in breakdown(diode on)
		if (itList != string_array[j].groupsByCurrentShortcut.end()){
			advance(itList,1);
			while(itList !=string_array[j].groupsByCurrentShortcut.end()){
				// If there's group/s in breakdown we have to correct the SVocr
				gS.sum_voltage_open_circuit_non_active_cells += itList->sum_voltage_open_circuit;
				advance(itList,1);
			}
			iMC.detailed_same_i_shortcut_and_v_breakdown_group.push_back(make_pair(j,gS));
			iMC.sum_same_i_shortcut_and_v_breakdown_group = gS;

			MultiMapElDets.insert(make_pair(clau,iMC));
			iMC.detailed_same_i_shortcut_and_v_breakdown_group.clear();

		}
	}
	// Unifies elements with the same Isc and Vbr. Across all the multimap. This implies the entire panel.
	// Instantiates a pair of multimaps
	pair<multimap<pair<double,double>,SameIshortcutAndVbreakdownGroup, Classcomp>::iterator,
		multimap<pair<double,double>,SameIshortcutAndVbreakdownGroup, Classcomp>::iterator> ret;
	// Temporal iterator to keep track of the last element
	multimap<pair<double,double>,SameIshortcutAndVbreakdownGroup, Classcomp>::iterator itmap1;
	// Iterator 2 iterates across our filled multimap
	for (multimap<pair<double,double>,SameIshortcutAndVbreakdownGroup, Classcomp>::iterator
			itmap2 = MultiMapElDets.begin(); itmap2 != MultiMapElDets.end(); ++itmap2)
	{
		itmap1 = itmap2;
		ret = MultiMapElDets.equal_range(itmap2->first);
		advance(itmap1,1);
		while (itmap1 != ret.second){
			itmap2->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_breakdown_in_group
				+= itmap1->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_breakdown_in_group;
			itmap2->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_open_circuit_all_cells
				+= itmap1->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_open_circuit_all_cells;
			itmap2->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_open_circuit_non_active_cells
				+= itmap1->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_open_circuit_non_active_cells;
			itmap2->second.sum_same_i_shortcut_and_v_breakdown_group.group_size
				+= itmap1->second.sum_same_i_shortcut_and_v_breakdown_group.group_size;

			itmap2->second.detailed_same_i_shortcut_and_v_breakdown_group.push_back
				(*itmap1->second.detailed_same_i_shortcut_and_v_breakdown_group.begin());
			itmap1 = MultiMapElDets.erase(itmap1);
			}
	}
}
/*
 * Returns the "first upper limit" of the I-V characteristic. That is the sum of Voc of every cell.
 */
double SolarSolver::findMaxVoltageLimit (){
	double LTO = 0;
	for(int k=0; k<panel_vector.size(); ++k){
		LTO += (panel_vector[k].sum_same_i_shortcut_group.sum_voltage_open_circuit_non_active_cells+panel_vector[k].sum_same_i_shortcut_group.sum_voltage_open_circuit_all_cells);
	}
	return LTO;
}
/*
 * Fulfills the Limit parameter of the inVectorSum grupString.
 * These "external" limits are the total voltage in the panel needed to get every cell into breakdown.
 * Notice that it is possible (and panels are designed so) that the bypass diode will enter conduction state before
 * the breakdown.
 *
 * The external limits represent a change in the total current.
 */
void SolarSolver::findVoltageLimitsForChangesInCurrent (const double LTO){
	double LTi = LTO;
	for (int k=0; k<panel_vector.size(); ++k){
		LTi -= panel_vector[k].sum_same_i_shortcut_group.sum_voltage_open_circuit_all_cells;
		LTi += panel_vector[k].sum_same_i_shortcut_group.sum_voltage_breakdown_in_group;
		panel_vector[k].sum_same_i_shortcut_group.limit_voltage = LTi;
	}
}
/*
 * Fulfills the Limit parameter of the inMapSum grupString.
 * These "internal" limits are the total voltage in the panel needed to get every bypass diode in conducting state.
 * In case there's no diode, the limit will match the lower external limit.
 *
 * The internal limits represent a change in the distribution of the total voltage.
 */
void SolarSolver::findVoltageLimitsForChangesInVoltage(){
	int N, Ngr;
	double Voffset;
	map <double, SameIshortcutAndVbreakdownGroup>::reverse_iterator itMap;
	for (int k=0; k<panel_vector.size(); ++k){
		N = panel_vector[k].sum_same_i_shortcut_group.group_size;
		Voffset = -panel_vector[k].sum_same_i_shortcut_group.sum_voltage_breakdown_in_group;
			for (itMap = panel_vector[k].detailed_same_i_shortcut_group.rbegin();
					itMap != panel_vector[k].detailed_same_i_shortcut_group.rend(); ++itMap)
			{
				Ngr = itMap->second.sum_same_i_shortcut_and_v_breakdown_group.group_size;
				itMap->second.sum_same_i_shortcut_and_v_breakdown_group.limit_voltage = N*itMap->first + Voffset;
				N -= Ngr;
				Voffset += itMap->second.sum_same_i_shortcut_and_v_breakdown_group.sum_voltage_breakdown_in_group;
			}
	}
}


int SolarSolver::findWorkingZone (double Vin)
{
	int i = 0;
	while(i<panel_vector.size()){
			if (Vin > panel_vector[i].sum_same_i_shortcut_group.limit_voltage)
				{break;
			}else{
				i = i+1;
			}
	}
	return(i);
}

void SolarSolver::calcUpperZones(int m, vector <double> &vVector)
{
	int iString;
	for (int k = 0; k < m; ++k){
		for (map<double,SameIshortcutAndVbreakdownGroup>::iterator itmap
				= panel_vector[k].detailed_same_i_shortcut_group.begin();
				itmap != panel_vector[k].detailed_same_i_shortcut_group.end(); ++itmap)
		{
			for (list<pair<int,CellsGroup>>::iterator itList
					= itmap->second.detailed_same_i_shortcut_and_v_breakdown_group.begin();
					itList != itmap->second.detailed_same_i_shortcut_and_v_breakdown_group.end(); ++itList)
			{
				iString = itList->first;
				vVector[iString]+=itList->second.sum_voltage_breakdown_in_group
						+ itList->second.sum_voltage_open_circuit_non_active_cells;
			}
		}
	}
}

void SolarSolver::calcLowerZones(int m, vector <double> &vVector)
{
	int iString;
	for (int k = panel_vector.size()-1; k > m; --k){
		for (map<double,SameIshortcutAndVbreakdownGroup>::iterator itmap
				= panel_vector[k].detailed_same_i_shortcut_group.begin();
				itmap != panel_vector[k].detailed_same_i_shortcut_group.end(); ++itmap)
		{
			for (list<pair<int,CellsGroup>>::iterator itList
					= itmap->second.detailed_same_i_shortcut_and_v_breakdown_group.begin();
					itList != itmap->second.detailed_same_i_shortcut_and_v_breakdown_group.end(); ++itList)
			{
				iString = itList->first;
				vVector[iString]+=(itList->second.sum_voltage_open_circuit_all_cells
						+ itList->second.sum_voltage_open_circuit_non_active_cells);
			}
		}
	}
}

void SolarSolver::calcMiddleZones(int m, double Vpan, vector <double> &vVector)
{
	int iString;
	double Vrel = Vpan-panel_vector[m].sum_same_i_shortcut_group.limit_voltage;
	map<double,SameIshortcutAndVbreakdownGroup>::iterator itmap;
	// The working zone is found
	for (itmap = panel_vector[m].detailed_same_i_shortcut_group.begin();
			itmap != panel_vector[m].detailed_same_i_shortcut_group.end(); ++itmap)
	{
		if (Vrel<itmap->second.sum_same_i_shortcut_and_v_breakdown_group.limit_voltage) break;
	}

	// Assignment of the breakdown voltages (or Vocr if the diode conducts first)
	map<double,SameIshortcutAndVbreakdownGroup>::iterator itmap2 = itmap;

	while (itmap2 != panel_vector[m].detailed_same_i_shortcut_group.end()){
		for (list<pair<int,CellsGroup>>::iterator itList
				= itmap2->second.detailed_same_i_shortcut_and_v_breakdown_group.begin();
				itList != itmap2->second.detailed_same_i_shortcut_and_v_breakdown_group.end(); ++itList)
		{
				iString = itList->first;
				vVector[iString]+=itList->second.sum_voltage_breakdown_in_group
						+ itList->second.sum_voltage_open_circuit_non_active_cells;
			}
			advance(itmap2,1);
	}

	int N = 0;
	double vLim, vRupt, vS;

	for (map<double,SameIshortcutAndVbreakdownGroup>::iterator itmap3
			= panel_vector[m].detailed_same_i_shortcut_group.begin(); itmap3 != itmap; ++itmap3)
	{
		N += itmap3->second.sum_same_i_shortcut_and_v_breakdown_group.group_size;
	}

	advance(itmap,-1);

	vLim = itmap->second.sum_same_i_shortcut_and_v_breakdown_group.limit_voltage;
	vRupt = itmap->first;

	// The voltage of every active cell is calculated
	vS = (Vrel-vLim)/N + vRupt;

	advance(itmap,1);

	for (map<double,SameIshortcutAndVbreakdownGroup>::iterator itmap3
			= panel_vector[m].detailed_same_i_shortcut_group.begin(); itmap3 != itmap; ++itmap3)
	{
		for (list<pair<int,CellsGroup>>::iterator itList
				= itmap3->second.detailed_same_i_shortcut_and_v_breakdown_group.begin();
				itList != itmap3->second.detailed_same_i_shortcut_and_v_breakdown_group.end(); ++itList)
		{
				iString = itList->first;
				N = itList->second.group_size;
				vVector[iString]+=(N*vS + itList->second.sum_voltage_open_circuit_non_active_cells);
			}
	}
}

double SolarSolver::findTotalCurrent(double Vpan)
{
	double Isc;
	int m;

	m = findWorkingZone(Vpan);
	Isc = panel_vector[m].sum_same_i_shortcut_group.current_shortcut;
	return (Isc);
}

/*
 * Given a total voltage through the panel, assigns the corresponding voltage to every string.
 */
void SolarSolver::assignStringVoltages(double Vpan, vector <double> &VString)
{
	int m;
	// Finds the working zone
	m = findWorkingZone(Vpan);
	// Assigns the initial estimate of the voltage to the cells in breakdown
	calcUpperZones(m, VString);
	// Assigns the initial estimate of the voltage to the non-active cells
	calcLowerZones(m, VString);
	// Calculates and assigns the initial estimate voltage to the active cells
	calcMiddleZones(m, Vpan, VString);
}


void SolarSolver::calcIVcharacteristic(std::string output_path)
{
	try
	{
		// Vector to store the voltage of every string
		vector <double> voltVector(number_strings, 0.0);

		double Itotal;

		// The variables are the voltage of every cell, the currents of every string and the total current
		int dimX = number_strings + 1;
		for (int i=0; i<number_strings; i++)
		{
			dimX = dimX + string_array[i].string_size;
		}

		// Standard characteristic is composed by 250 points
		double start_v = -2;
		double end_v = findMaxVoltageLimit();
		int numb_points = 250;
		double step = (end_v - start_v)/numb_points;
		step = (int)(step * 100 + .5);
		step = (double)step / 100;


		fstream fout;
		// opens an existing csv file or creates a new file.
		fout.open(output_path, ios::out);

		for (double vc = start_v; vc <= end_v; vc += step)
		{
			// Initialization of the (recycled) voltVector
			for (int i = 0; i < voltVector.size(); ++i)
			{
				voltVector[i] = 0.0;
			}
			// Assignment of currents and voltages to every string
			Itotal = findTotalCurrent(vc);
			assignStringVoltages(vc, voltVector);
			// Calculation of the initial approximation
			for (int k = 0; k < number_strings; ++k)
			{
				string_array[k].findInitialState(Itotal, voltVector[k]);
			}

			// Iterative method is called to solve every string
			try
			{
				Itotal = calcNewtonRaphson(string_array,vc,dimX, number_strings);
			}
			catch(...)
			{
				std::cout << "Error when computing the iterative method for "<< vc << " volts." << endl;
			}

			// Insert the data to file
			fout << vc << ";" << Itotal << "\n";
			std::cout << vc << "; " << Itotal << "\n";
		}
		fout.close();
	}
	catch(...)
	{
		std::cout << "Error when computing the IV characteristic" << endl;
	}
}

void SolarSolver::calcIVcharacteristic(std::string output_path, double start_v, double end_v, int numb_points)
{
	try
	{
		if(start_v > end_v || numb_points < 1)
		{
			throw std::runtime_error("Error in the characteristic parameters.");
		}

		// Vector to store the voltage of every string
		vector <double> voltVector(number_strings, 0.0);

		double Itotal;

		// The variables are the voltage of every cell, the currents of every string and the total current
		int dimX = number_strings + 1;
		for (int i=0; i<number_strings; i++)
		{
			dimX = dimX + string_array[i].string_size;
		}

		// Standard characteristic is composed by 250 points
		double step = (end_v - start_v)/numb_points;
		step = (int)(step * 100 + .5);
		step = (double)step / 100;

		fstream fout;
		// opens an existing csv file or creates a new file.
		fout.open(output_path, ios::out);

		for (double vc = start_v; vc <= end_v; vc += step)
		{
			// Initialization of the (recycled) voltVector
			for (int i = 0; i < voltVector.size(); ++i)
			{
				voltVector[i] = 0.0;
			}
			// Assignment of currents and voltages to every string
			Itotal = findTotalCurrent(vc);
			assignStringVoltages(vc, voltVector);
			// Calculation of the initial approximation
			for (int k = 0; k < number_strings; ++k)
			{
				string_array[k].findInitialState(Itotal, voltVector[k]);
			}

			// Iterative method is called to solve every string
			try
			{
				Itotal = calcNewtonRaphson(string_array,vc,dimX,number_strings);
			}
			catch(...)
			{
				std::cout << "Error when computing the iterative method for "<< vc << " volts." << endl;
			}

			// Insert the data to file
			fout << vc << ";" << Itotal << "\n";
			std::cout << vc << "; " << Itotal << "\n";
		}
		fout.close();
	}
	catch(...)
	{
		std::cout << "Error when computing the IV characteristic" << endl;
	}
}

void SolarSolver::calcState(std::string output_path, double Vpan)
{
	try
	{
		// Vector to store the voltage of every string
		vector <double> voltVector(number_strings, 0.0);

		double Itotal;

		// The variables are the voltage of every cell, the currents of every string and the total current
		int dimX = number_strings + 1;
		for (int i=0; i<number_strings; i++)
		{
			dimX = dimX + string_array[i].string_size;
		}

		// Initialization of the (recycled) voltVector
		for (int i = 0; i < voltVector.size(); ++i)
		{
			voltVector[i] = 0.0;
		}
		// Assignment of currents and voltages to every string
		Itotal = findTotalCurrent(Vpan);
		assignStringVoltages(Vpan, voltVector);
		// Calculation of the initial approximation
		for (int k = 0; k < number_strings; ++k)
		{
			string_array[k].findInitialState(Itotal, voltVector[k]);
		}

		// Iterative method is called to solve every string
		try
		{
			Itotal = calcNewtonRaphson(string_array, Vpan, dimX, number_strings);
		}
		catch(...)
		{
			std::cout << "Error when computing the iterative method." << endl;
		}

		// Starts the printing process
		ofstream arx;

		std::string output_file = output_path;
		arx.open(output_path, ios::out);
		if (!arx){
			cout << "Cannot open the output file.";
		}

		for (int k = 0; k < number_strings; ++k){
			arx << "Idiode(" << k << ") = " << string_array[k].diode_bypass.getCurrentDiode() << " A" << endl;
		}

		arx << endl << endl;

		arx << "String" << "," << "Cell" << "," << "Irrad." << "," << "Temper." << "," << "Curr. (A)" << "," << "Volt. (V)" << endl;
		for (int k = 0; k < number_strings; ++k){
			for (int j = 0; j < string_array[k].string_size; ++j){
				arx << k << "," << string_array[k].cells_array[j].getIndex() << ","
						<< string_array[k].cells_array[j].getIrradiance() << ","
						<< string_array[k].cells_array[j].getTemperatureCell();
				arx << "," << string_array[k].cells_array[j].getCurrentCell() << ","
						<< string_array[k].cells_array[j].getVoltageCell() << endl;
			}
		}
	}
	catch(...)
	{
		std::cout << "Error when computing the state." << endl;
	}
}



double SolarSolver :: calcNewtonRaphson (solar_string *st, double Vp, int _dimX, int nS)
{
	// Functions matrix (column)
	Col<double> Fv = zeros(_dimX-1);
	// Initial state matrix (column)
	Col<double> Xv = zeros(_dimX);
	// Jacobian matrix
	Mat<double> Jv = zeros(_dimX-1,_dimX-1);
	// Solution: new state (column)
	Col<double> Gv = zeros(_dimX-1);


	double It;
	int totalCells = _dimX-nS-1;
	int *indexs = new int[totalCells];

	// Initial values are loaded
	Xv = loadInitialValues(st, _dimX, nS);

	double nm;

	int m=0;

	do
	{
		// Starts building the jacobian matrix
		double **J = new double *[_dimX-1];
		for (int i=0; i<_dimX-1; i++){
			J[i] = new double [_dimX];
		}

		for (int i=0; i<_dimX-1; i++){
			for (int j=0; j<_dimX; j++){
				J[i][j] = 0;
			}
		}

		int relatiu1 = 0;
		It = Xv(_dimX-nS-1);
		double Id = 0.0;

		// Updates the string of arrays with the initial state vector
		for (int i=0; i<nS; i++){
			for (int j=0; j<st[i].string_size; j++){
				st[i].cells_array[j].setCurrentCell(Xv(_dimX-nS+i));
				st[i].cells_array[j].setVoltageCell(Xv(relatiu1+j));
			}
			st[i].setSumVoltageAllCells();
			Id = st[i].diode_bypass.calcFunctionD(-st[i].getSumVoltageAllCells());
			st[i].diode_bypass.setCurrentDiode(Id);
			relatiu1 = relatiu1 + st[i].string_size;
		}

		relatiu1 = 0;

		// Fills the Functions vector
		for (int i=0; i<nS; i++){
			for (int j=0; j < st[i].string_size; j++){
				Fv(relatiu1+j)=st[i].cells_array[j].calcFunctionC();
			}
			Fv(_dimX-nS-1+i) = It - st[i].cells_array[0].getCurrentCell()
					-st[i].getWithDiode()*st[i].diode_bypass.getCurrentDiode();
			// next string
			relatiu1 = relatiu1 + st[i].string_size;
		}

		relatiu1 = 0;
		int relatiu2 = _dimX-nS-1;
		// Fills the jacobian matrix (initialy a 2D array, not the matrix)
		for (int i=0; i<nS; i++){
			for (int j=0; j < st[i].string_size; j++){
				J[relatiu1+j][relatiu1+j]=st[i].cells_array[j].calcFunctionCellDerivativeRespectVoltage();
				J[relatiu1+j][relatiu2+i]=st[i].cells_array[j].calcFunctionCellDerivativeRespectCurrent();
				indexs[relatiu1+j]=relatiu2+i;
			}
			relatiu1 = relatiu1 + st[i].string_size;
		}

		relatiu1 = _dimX-nS-1;
		relatiu2 = 0;
		for (int i=0; i<nS; i++){
			for (int j=0; j<st[i].string_size; j++){
				J[relatiu1+i][relatiu2+j]
							  = st[i].getWithDiode()
							  * st[i].diode_bypass.calcFuntionDiodeDerivativeRespectVoltage(-st[i].getSumVoltageAllCells());
			}
			// Fixes the last diode
			if (i==(nS-1)){
					for(int j=0; j<_dimX-nS-1; j++){
						J[_dimX-2][j] = J[_dimX-2][j] - J[_dimX-2][_dimX-nS-2];
					}
			}
			J[relatiu1+i][relatiu1-1]=1;
			J[relatiu1+i][relatiu1+i]=-1;
			relatiu2+=st[i].string_size;
		}

		// Loop the replace the voltage of the last cell by the difference of the total voltage and the rest of cells
		for(int i=0; i<_dimX-nS-1; i++){
			J[_dimX-nS-2][i] = J[_dimX-nS-2][i] - J[_dimX-nS-2][_dimX-nS-2];
		}

		// Calculates the norm
		nm = norm(Fv,2);

		if (nm <= epsilon){
			break;
		}

		for (int i = 0; i < _dimX-1; i++){
			J[i][_dimX-1] = -Fv(i);
		}

		// Fills the actual armadillo Jacobian matrix
		for (int i=0; i<_dimX-1; i++){
						for (int j=0; j<_dimX-1; j++){
							Jv(i,j)=J[i][j];
						}
				}

		double temp;
		for (int i=0; i<_dimX; i++){
			temp = J[_dimX-2][i];
			J[_dimX-2][i] = J[_dimX-nS-2][i];
			J[_dimX-nS-2][i] = temp;
		}

		// Solve the matrix equation to find the increment
		try
		{
			Gv = solve(Jv,-Fv);
		}
		catch(std::runtime_error& err)
		{
			std::cout << "Error in Armadillo solve. Runtime error: " << err.what() << endl;
		}
		catch(...)
		{
			std::cout << "Error in Armadillo solve" << endl;
		}

		// X_2 = X_1 + Jx(-F)
		for (int j=0; j<_dimX-1;j++){
			if (j<_dimX-nS-2){
				Xv(j)+=Gv(j);
			} else {
				Xv(j+1)+=Gv(j);
			}
		}

		double sumX = 0.0;
		for (int i=0; i<_dimX-nS-2; i++){
			sumX = sumX + Xv(i);
		}

		Xv(_dimX-nS-2) = Vp - sumX;

		for (int i=0; i<_dimX-1; i++){
			delete [] J[i];
		}
		delete [] J;

		m += 1;

	// Condition of convergence
	}while(nm > epsilon or m > max_iterations);

	delete [] indexs;

	return(It);
}


SolarSolver :: SolarSolver(SolarPanel &panel)
{
	try
	{
		epsilon = EPSILON_REF;
		max_iterations = MAX_ITERATIONS_REF;
		number_strings = panel.panel_size;
		string_array = new solar_string[number_strings];

		// The string objects are created from the info in the panel object
		for (int k=0; k<number_strings; k++)
		{
			string_array[k].updateStringsData(panel.string_info[k], panel.cell_panel);
		}

		// If a certain knee voltage for the bypass diodes has been specified, then all the strings are updated
		if(panel.voltage_knee_diode != 0)
		{
			for (int k=0; k<number_strings; k++)
			{
				string_array[k].setVoltageDiode(panel.voltage_knee_diode);
			}
		}
		// The vector is fulfilled and properly organized
		generatePanelVector ();
	}
	catch(...)
	{
		std::cout << "Error when computing the IV characteristic" << endl;
	}

}

}
