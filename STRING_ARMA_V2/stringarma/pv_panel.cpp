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

#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include "pv_panel.h"

using namespace std;

namespace stringarma{

	void SolarPanel::setCellVoltageBreakdown(double _Vbreak)
	{
		try
		{
			cell_panel.setVoltageBreakdown(_Vbreak);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Voltage Breakdown parameter." << endl;
		}
	}
	void SolarPanel::setCellBreakdownAlpha(double _alpha)
	{
		try
		{
			cell_panel.setBreakdownAlpha(_alpha);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Breakdown Alpha parameter." << endl;
		}
	}
	void SolarPanel::setCellSoilingFactor(double _SF)
	{
		try
		{
			cell_panel.setSoilingFactor(_SF);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Soiling Factor parameter." << endl;
		}
	}
	void SolarPanel::setCellIdealityFactor(double _n)
	{
		try
		{
			cell_panel.setIdealityFactor(_n);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Ideality Factor parameter." << endl;
		}
	}
	void SolarPanel::setCellResistanceSeries(double _Rs)
	{
		try
		{
			cell_panel.setResistanceSeries(_Rs);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Series Resistance parameter." << endl;
		}
	}
	void SolarPanel::setCellResistanceShunt(double _Rsh)
	{
		try
		{
			cell_panel.setResistanceShunt(_Rsh);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Shunt Resistance parameter." << endl;
		}
	}
	void SolarPanel::setCellTemperatureCoeff(double _a)
	{
		try
		{
			cell_panel.setTemperatureCoeff(_a);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Temperature Coeff parameter." << endl;
		}
	}
	void SolarPanel::setCellVoltageTemperatureCoeff(double _B)
	{
		try
		{
			cell_panel.setVoltageTemperatureCoeff(_B);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Voltage Temperature Coeff parameter." << endl;
		}
	}
	void SolarPanel::setCellBreakdownExponent(double _m)
	{
		try
		{
			cell_panel.setBreakdownExponent(_m);
		}
		catch(...)
		{
			std::cout << "Error when modifying the Breakdown Exponent parameter." << endl;
		}
	}
	void SolarPanel::setVoltageKneeDiode(double Vknee)
	{
		try
		{
			voltage_knee_diode = Vknee;
		}
		catch(...)
		{
			std::cout << "Error when modifying the Diode's Knee Voltage parameter." << endl;
		}
	}
	double SolarPanel::getCellVoltageBreakdown(void)
	{
		return(cell_panel.getVoltageBreakdown());
	}
	double SolarPanel::getCellBreakdownAlpha(void)
	{
		return(cell_panel.getBreakdownAlpha());
	}
	double SolarPanel::getCellSoilingFactor(void)
	{
		return(cell_panel.getSoilingFactor());
	}
	double SolarPanel::getCellIdealityFactor(void)
	{
		return(cell_panel.getIdealityFactor());
	}
	double SolarPanel::getCellResistanceSeries(void)
	{
		return(cell_panel.getResistanceSeries());
	}
	double SolarPanel::getCellResistanceShunt(void)
	{
		return(cell_panel.getResistanceShunt());
	}
	double SolarPanel::getCellTemperatureCoeff(void)
	{
		return(cell_panel.getTemperatureCoeff());
	}
	double SolarPanel::getCellVoltageTemperatureCoeff(void)
	{
		return(cell_panel.getVoltageTemperatureCoeff());
	}
	double SolarPanel::getCellBreakdownExponent(void)
	{
		return(cell_panel.getBreakdownExponent());
	}
	double SolarPanel::getVoltageKneeDiode(void)
	{
		return(voltage_knee_diode);
	}

	vector<pair<bool,vector<pair<double,double>>>>
	SolarPanel :: readInput(std::string filepath)
	{
	  ifstream myfile;
	  myfile.open(filepath, ios::in);

	  vector<pair<bool,vector<pair<double,double>>>> string_info;
	  /*
	   * The file is like
	   * 1;	diode line
	   * 1000;25	cell line
	   * 1000;25
	   */
	  if (myfile.is_open())
	    {
	      bool isDiode;
	      bool ok;
	      char delimiter = ';';
	      char endline = '\n';
	      char c;
	      char *s;
	      char *s2;
	      double T;
	      double G;
	      int nline = 0;
	      int count_string = -1;
	      int count_cell = 0;
	      int pos [2];
	      int i;
	      int intBool;
	      std::string line;
	      vector<pair<double,double>> opValues;
	      // Iterate over the whole file
	      ok = true;
	      while (std::getline(myfile, line))
		{
		  ++nline;
		  pos [0] = line.find (delimiter);
		  if (pos [0] == std::string::npos)
		    {
		      ok = false;
		      break;
		    }
		  isDiode = true;
		  for (i = pos [0] + 1; i < line.size () && isDiode; ++i)
		    {
		      /* Check whether this is a diode line: there are no
			 non-blanks after the delimiter.  */
		      c = line.c_str () [i];
		      if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			{
			  pos [1] = i;
			  isDiode = false;
			}
		    }
		  if (isDiode)
		    {
		      /* Diode line: it must contain zero or more blanks,
			 a "0" or a "1", and zero or more blanks, a ";",
			 and zero or more blanks.  */
		      for (i = 0; i < pos [0]; ++i)
			{
			  /* Skip leading blanks.  */
			  c = line.c_str () [i];
			  if (c != ' ' && c != '\t')
			    {
			      break;
			    }
			}
		      if (i < pos [0])
			{
			  intBool =  std::stoi (line.c_str () + i);
			  if (intBool != 0 && intBool != 1)
			    {
			      /* Neither "0" nor "1": error.  */
			      ok = false;
			      break;
			    }
			  if (!opValues.empty())
			    {
			      string_info.push_back(make_pair(intBool,opValues));
			      opValues.clear();
			    }
			  count_string++;
			  count_cell=0;
			}
		    }
		  else
		    {
		      /* Cell line: it must contain zero or more blanks, a
			 valid floating-point value, zero or more blanks,
			 a ";", zero or more blanks, a valid
			 floating-point value, and zero or more
			 blanks.  */
		      for (i = 0; i < pos [0]; ++i)
			{
			  /* Skip leading blanks.  */
			  c = line.c_str () [i];
			  if (c != ' ' && c != '\t')
			    {
			      break;
			    }
			}
		      if (i < pos [0])
			{
			  G = strtod (line.c_str () + i, &s);
			  if (G == HUGE_VAL
			      || (G == 0.0 && s == line.c_str () + i))
			    {
			      /* Not a valid floating-point value:
				 error.  */
			      ok = false;
			      break;
			    }
			  for (; s && *s; ++s)
			    {
			      /* Check that what is left up to the
				 delimiter consists of blanks.  */
			      if (*s != ' ' && *s != '\t' && *s != '\r'
				  && *s != '\n')
				{
				  break;
				}
			    }
			  if (*s != delimiter)
			    {
			      /* Something spurious has been found:
				 error.  */
			      ok = false;
			      break;
			    }
			  s = (char *) line.c_str () + pos [1];
			  T = strtod (s, &s2);
			  if (T == HUGE_VAL
			      || (T == 0.0 && s2 == s))
			    {
			      ok = false;
			      break;
			    }
			  s = s2;
			  for (; s && *s && ok; ++s)
			    {
			      if (*s != ' ' && *s != '\t' && *s != '\r'
				  && *s != '\n')
				{
				  /* Something spurious has been found:
				     error.  */
				  ok = false;
				}
			    }
			  if (!ok)
			    {
			      break;
			    }
				opValues.push_back(make_pair(G,T));
				count_cell++;
			}
		      else
			{
			  /* First floating-point value not found.  */
			  ok = false;
			  break;
			}
		    }

		}
	      myfile.close ();
			if(!opValues.empty())
			{
				string_info.push_back(make_pair(intBool,opValues));
			}

	  if (!ok)
		{
		  std::string msg ("Error when mapping the input file. "
			      "Check the input file format. (File \'");
		  msg += filepath;
		  msg += "\', line no. ";
		  msg += std::to_string (nline);
		  msg += ".)";
		  throw std::runtime_error(msg);
		}
	      return string_info;
	    }
	  else
	    {
	      std::string msg ("Could not open input file. (File \'");
	      msg += filepath + "\'.)";
	      throw std::runtime_error(msg);
	    }

	  return string_info;
	}

	int SolarPanel :: getPanelSize()
	{
		return panel_size;
	}

	SolarPanel :: SolarPanel()
	{
		panel_size = 0;
		voltage_knee_diode = 0;
	}

	SolarPanel :: SolarPanel(std::string filepath)
	{
		/*
		 * If when calling the solver this value has not change
		 * the reference value is used.
		 */
		voltage_knee_diode = 0;

		// Read the input file and stores the info
		try
		{
			string_info = readInput(filepath);
		}
		catch(std::runtime_error& err)
		{
			std::cout << err.what() << endl;
		}
		catch(...)
		{
			std::cout << "Unspecified error produced "
					"when reading the input file." << endl;
		}

		panel_size = string_info.size();
	}

}
