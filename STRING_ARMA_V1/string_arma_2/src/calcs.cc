/** \file calcs.cc
 * This file contains calcs, mainly.
 * Meh, that's all actually
 */


#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <armadillo>
#include "solar_string.h"

//#include "solar_cell.h"
//#include "solar_string.h"
//#include "bypass_diode.h"
//#include "gnuplot.h"

//#define Io 2.60  // 5.65
//#define Vo 0.0 // 0.35
//#define numStrings 1

using namespace std;
using namespace arma;

/*void representar (void)
{
	Gnuplot plot;
	plot("set datafile separator \",\" \n");
	plot("set term windows");
	plot("set title \" Corbes aproximades \"");
	plot("set grid mxtics mytics");
	plot("set mytics 2");
	plot("set mxtics 5");
	plot("set grid xtics ytics");
	//plot("plot 'C:\\Documents and Settings\\isabel\\Mis documentos\\Visual Studio 2008\\Projects\\Combi_v2\\Combi_v1\\prova_2z.dat' using 1:2 title 'calculat' \n");
	//plot("replot 'C:\\Documents and Settings\\isabel\\Mis documentos\\Visual Studio 2008\\Projects\\Combi_v1\\Combi_v1\\mostra_2.csv' using 2:4 title 'mesurat' with lines \n");
	//plot("plot 'C:\\Documents and Settings\\isabel\\Mis documentos\\Visual Studio 2008\\Projects\\Combi_v2\\Combi_v1\\prova_2z.dat' using 1:3 lw 2.000 title 'exacte' with lines \n");
	plot("plot 'G:\\Combi_v3_pw\\Combi_v3_pw\\prova_2y.dat' using 1:2 title 'cel�la 0, string 0' with lines \n");
	system("pause"); //c++ DOS command shell to stop the console
}*/
/*double norm(double *Fa, int nCells)
{
	double dist = 0;
	double sum = 0;
	for(int i=0;i<nCells;i++){
		sum += pow(Fa[i],2);
	}
	dist = sqrt(sum);
	return(dist);
}*/

/*double* init (solar_string *st, int n, int nS)
{
	double *initCond = new double[n];
	int relatiu = 0;
	for (int i=0; i<nS; i++){ // inicialitzar tensions
		for (int j=0; j<st[i].midaCorda; j++){
			initCond[relatiu+j] = st[i].corda[j].getVcell();
		}
		relatiu += st[i].midaCorda;
	}
	//cout << relatiu << endl;
	initCond[relatiu] = st[0].corda[0].getIcell() + st[0].rm3_c.getIdiode(); // intensitat total basada en el string 0
	relatiu += 1;
	for (int i=0; i<nS; i++){
		initCond[relatiu+i] = st[i].corda[0].getIcell();
	}
	return (initCond);
	delete [] initCond;
}*/
Col<double> inix(solar_string *st, int n, int nS){
	Col<double> Z = zeros(n);
	int relatiu = 0;
	for (int i=0; i<nS; i++){ // inicialitzar tensions
		for (int j=0; j<st[i].midaCorda; j++){
			Z(relatiu+j) = st[i].corda[j].getVcell();
			}
			relatiu += st[i].midaCorda;
		}
	Z(relatiu) = st[0].corda[0].getIcell() + st[0].rm3_c.getIdiode(); // intensitat total basada en el string 0
	relatiu += 1;
	for (int i=0; i<nS; i++){
		Z(relatiu+i) = st[i].corda[0].getIcell();
		}
	return Z;
}
double* Jsolve(double **a, int *ind, int n)
{
	int i,j,k;
	double *sol = new double[n-1]; // canviat n per n-1
	for (i=0;i<n-1;i++){ // canviat n per n-1
		sol[i]=0.0;
	}

	double c;

	for(k=0;k<ind[0]-1;k++) // new method, ind[0] = n-nS-1
		for(i=ind[0]-1;i<=n-2;i++) // no fem els rengles i columnes ambzeros
		{	
			c= (a[i][k]/a[k][k]) ;
			a[i][k] = 0;
			a[i][ind[k]] -=c*a[k][ind[k]];
			if (abs(a[i][k])<1e-16)
						a[i][k]=0;
			a[i][n-1] -= c*a[k][n-1];
			if (abs(a[i][n-1])<1e-16)
						a[i][n-1]=0;
		}

		for(k=ind[0]-1;k<n-1;k++) // the old method
			for(i=k;i<n-2;i++) // canviat n-1 per n-2
		{	
			c= (a[i+1][k]/a[k][k]) ;     
			for(j=0;j<=n-1;j++){  // canviat n per n-1
				a[i+1][j]-=c*a[k][j];
				if (abs(a[i+1][j])<1e-16)
						a[i+1][j]=0;
			}
		}
     
	//***************** Backward Substitution method****************//

	for(i=n-2;i>=0;i--) // canviat n-1 per n-2
	{
		c=0;
		for(j=i;j<=n-2;j++){// canviat n-1 per n-2
			c=c+a[i][j]*sol[j];
			sol[i]=(a[i][n-1]-c)/a[i][i]; // canviat n per n-1
			//if (a[i][i]==0)
				//cout << "a[" << i << "][" << i << "]" << endl;
		}

			//cout << "sol[" << i << "]" << " = " << sol[i] << endl;
	}

	return (sol);
	delete [] sol;
}



/**
 * \addtogroup nonmembers Non-member functions
 * @{
 */

/**
 * Calculates the exact state of the provided PV panel for a given voltage.
 *
 * The iterative method of Newton-Raphson is used to solve the nonlinear system of equations that define the PV panel. By default, it uses the euclidean norm. There is no maximum number of iterations defined.
 * @param *st Array of solar_string classes that conform the panel.
 * @param Vp Voltage between the terminals of the PV panel [V].
 * @param _dimX Sum of the total number of cells in the panel plus number of strings plus 1.
 * @param nS Number of strings in the panel.
 * @returns A double data type value of the total current through the panel [A].
 */
double calc_ivm (solar_string *st, double Vp, int _dimX, int nS) //Vp tensi� aplicada. nS: nombre de strings
{
	
	
	//int ;
	Col<double> Fv = zeros(_dimX-1);
			//zeros<vec>(_dimX);
	Col<double> Xv = zeros(_dimX); //vector condicions inicials

	Mat<double> Jv = zeros(_dimX-1,_dimX-1);

	Col<double> Gv = zeros(_dimX-1); //vector soluci�


	double It;
	int totalCells = _dimX-nS-1;
	int *indexs = new int[totalCells]; // indexar per a

	Xv = inix(st, _dimX, nS);

	//Xv.print("Xv:");

	//cout << "Xv(12)= " << Xv(12) << endl;

	//system("pause");


	ofstream test_J;
	//double Fmin;
	double nm;
	
	int m=0; // variable per supervisar l'estat de les variables
	
	do
	{
		//m += 1;
		//double J[MAX][MAX+1]={0.0};
		/*if (m==2){

			for (int k = 0; k < nS; ++k){
				cout << "panell: " << k << endl;
				cout << "Idiode = " << st[k].getIdiode() << '\t' << "Icell = " << st[k].corda[0].getIcell() << endl;
				for (int j = 0; j < st[k].midaCorda; ++j){
					cout << "V (" << j << ") = " << st[k].corda[j].getVcell() << endl;
				}
			cout << endl << endl;
			}
			system("pause");
		}*/

		// The jacobian matrix is initialized with the proper size
		double **J = new double *[_dimX-1];
		for (int i=0; i<_dimX-1; i++){
			J[i] = new double [_dimX];
		}

		for (int i=0; i<_dimX-1; i++){
			for (int j=0; j<_dimX; j++){
				J[i][j] = 0;
			}
		}
		
		// The initial estimate is applied to the panel
		int relatiu1 = 0; //utilitzat per adre�ar tensions de cel�la en x
		//It = x[_dimX-nS-1];
		It = Xv(_dimX-nS-1);
		double Id = 0.0; // corrent en el d�ode per a c�lculs no definitius
		for (int i=0; i<nS; i++){
			//st[i].setIt(x[0]);
			for (int j=0; j<st[i].midaCorda; j++){
				st[i].corda[j].setIcell(Xv(_dimX-nS+i)); // els corrents s�n a la part final d'x
				st[i].corda[j].setVcell(Xv(relatiu1+j));//assignem les tensions de cel�la
			}
			st[i].setSvcell();
			//st[i].setIdiode(-st[i].getSvcell()); //corrent en el d�ode bypass
			Id = st[i].rm3_c.calcIdiode(-st[i].getSvcell());
			st[i].rm3_c.setIdiode(Id);
			relatiu1 = relatiu1 + st[i].midaCorda; //avancem una corda
		}
		
		relatiu1 = 0; // posem a 0 l'index per a cada string

		for (int i=0; i<nS; i++){ // com abans
			for (int j=0; j < st[i].midaCorda; j++){
				Fv(relatiu1+j)=st[i].corda[j].calcfcn();
			}
			Fv(_dimX-nS-1+i) = It - st[i].corda[0].getIcell()-st[i].getambDiode()*st[i].rm3_c.getIdiode(); //equaci� d�ode de bypass, afegida condici� de tenir d�ode
			relatiu1 = relatiu1 + st[i].midaCorda; // passem al seg�ent string
		}
		
		//Fv.print("Fv:");

		//system ("pause");


		/* Afegit per tal d'introduir Fv */

		/*Espai per al jacobi� */
		
		relatiu1 = 0;  // com abans
		int relatiu2 = _dimX-nS-1;
		
		for (int i=0; i<nS; i++){
			for (int j=0; j < st[i].midaCorda; j++){
				J[relatiu1+j][relatiu1+j]=st[i].corda[j].calcderv();//derivada func string pel que fa I string
				J[relatiu1+j][relatiu2+i]=st[i].corda[j].calcderi();//derivada func string pel que fa Vcel�les
				indexs[relatiu1+j]=relatiu2+i; // per indexar les columnes deri
			}
			relatiu1 = relatiu1 + st[i].midaCorda;
		}

		relatiu1 = _dimX-nS-1;
		relatiu2 = 0;
		for (int i=0; i<nS; i++){
			for (int j=0; j<st[i].midaCorda; j++){
				J[relatiu1+i][relatiu2+j]=st[i].getambDiode()*st[i].rm3_c.calcderv(-st[i].getSvcell());
			}
			if (i==(nS-1)){
					for(int j=0; j<_dimX-nS-1; j++){ //correccio del darrer d�ode
						J[_dimX-2][j] = J[_dimX-2][j] - J[_dimX-2][_dimX-nS-2];
					}
			}
			J[relatiu1+i][relatiu1-1]=1;
			J[relatiu1+i][relatiu1+i]=-1;
			relatiu2+=st[i].midaCorda;
		}



		/* Bucle de correcci� del rengle de la darrera cel�la */

		for(int i=0; i<_dimX-nS-1; i++){
			J[_dimX-nS-2][i] = J[_dimX-nS-2][i] - J[_dimX-nS-2][_dimX-nS-2];
		}	

		//Fmin = norm(F, _dimX-1);
		nm = norm(Fv,2);

		if (nm <= 0.01){
			break;
		}

		for (int i = 0; i < _dimX-1; i++){ //afegim el vector de termes independents
			J[i][_dimX-1] = -Fv(i); // per aix� J t� una columna m�s que rengles
		}


		/* Bescanviem el rengle de la darrera cel�la pel del darrer d�ode*/

		for (int i=0; i<_dimX-1; i++){
						for (int j=0; j<_dimX-1; j++){
							Jv(i,j)=J[i][j];
						}
				}

		//Jv.print("Jv:");
		//cout << _dimX-nS-2 << endl;

		//cout << "det(Jv) = " << det(Jv) << endl;





		double temp;
		for (int i=0; i<_dimX; i++){
			temp = J[_dimX-2][i]; // darrer rengle
			J[_dimX-2][i] = J[_dimX-nS-2][i];
			J[_dimX-nS-2][i] = temp; // rengle on era la darrera cel�la
		}



		//Jv.print();

		//double *G = Jsolve(J,indexs,_dimX); // m+1 equacions
		Gv = solve(Jv,-Fv);

		//Gv.print("Gv:");



		for (int j=0; j<_dimX-1;j++){//fins a Vcell1
			if (j<_dimX-nS-2){
				//Xv(j)+=G[j];
				Xv(j)+=Gv(j);
			} else {
				//Xv(j+1)+=G[j];
				Xv(j+1)+=Gv(j);
			}
		}
		//system("pause");

		double sumX = 0.0;
		for (int i=0; i<_dimX-nS-2; i++){//sense els corrents
			sumX = sumX + Xv(i);
		}
		
		Xv(_dimX-nS-2) = Vp - sumX;// darrera cel�la
		for (int i=0; i<_dimX-1; i++){
			delete [] J[i];
		}
		delete [] J;

		m += 1;
		
		}while(nm > 0.01);

		

		//delete [] F;
		delete [] indexs;

		return(It);
		//return(x[_dimX-1]);

		//setSvcell();
}

/**
 * @}
 */
