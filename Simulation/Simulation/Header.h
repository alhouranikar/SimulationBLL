#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <set>

class UI
{ //Import einer obj-Datei, sowie Erstellung der Simulation
public:
	void set_initial();
private:
	void import_obj();
};

class Mesh
{ //Erstellung des Meshs
public:
	Mesh(long sizex, long sizey);
	std::set<std::vector<long double>> values;
private:
	std::vector<std::vector<long long>> mesh;
	long size_x = 0;
	long size_y = 0;
};

class Time
{
public:
	Time(double strt, double end, long double step);
	void next_frame(); //Aufruf des nächsten Frames (akt_zeit += time_stp)
	void check_cfl(double umax, unsigned long double dist); //Ueberpruefung der CFL-Zahl
private:
	const double start = 0;
	const double end = 0;
	const unsigned long double time_stp = 0;
};