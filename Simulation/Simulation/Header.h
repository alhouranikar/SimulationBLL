

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <set>
#include <memory>
#include <thread>
#include <chrono>
#include <list>
#include <cmath>

using namespace std;

extern vector<vector<vector<vector<long double>>>> frames; // Deklaration hier, damit set_initial keine Fehlermeldung gibt, dass der Vector noch nicht defniert ist
extern vector<vector<bool>> is_boundary; // zeigt, welche Zellen im Mesh boundaries sind
// Alle Variablen, die mit der Zeit zusammenhängen
extern unique_ptr<const double> start;
extern unique_ptr<const double> ende;
extern unique_ptr<double> time_stp;
extern unsigned int frame_anz;
extern unsigned int akt_frame;

// Alle Variablen, die mit dem Mesh zusammenhängen
// Vectoren, die das Mesh wiedergeben, werden in create_mesh erstellt, damit die gewünschte Größe beim initialisieren angeben werden kann
extern size_t size_x;
extern size_t size_y;
extern unsigned int nachbarn; // Anzahl der benachbarten Zellen
extern vector<vector<long double>> temp;
extern vector<long double> temp_nval; // Vektor, welcher die einer Zelle benachbarten Werte speichert

// Alle Variablen, die mit den einzelnen Frames zusammenhängen
extern unsigned int current_posx;
extern unsigned int current_posy;
extern double past_posx;
extern double past_posy;
extern long double umax;


void import_obj(); // Verantwortlich dafür, verschiedene Geometries zu importieren

void create_mesh(); // Mesh erstellen

void set_initial(); // Ursprungszustand setzen, Startbedinugen auf Mesh ansetzen

void next_frame(); // Nächsten Frame aufrufen

void calc_dt(double umax, long double dist); // errechnet den Zeitschritt anhand der CFL-Bedingung

void check_umax(long double speed); // Überprüft, ob die maximale Geschwindigkeit übertroffen wird

//void call_frame(unsigned int posx, unsigned int posy); // Ruft den nachfolgenden Frame auf

void calc_pressure(); // Berechnet den Druck an jedem Ort zu einem Zeitpunkt

void calc_vel(); // Berechnet die Geschwindigkeit anhand des Drucks

vector<long double> get_nval(int w, int v); // NUR FÜR TESTZWECKE, Ausgabe der benachbarten Werte, w steht für verschiedene Werte: 0 = Druck, 1 = Geschw. in x-Richtung, 2 = Geschw. in y-Richtung

long double calc_avg(int i); // Berechnung des Durchschnitts benachbarter Werte

vector<long double> interpolate(); // Funktion, um die der Ursprungsposition der Teilchen nähesten Zelle zu finden

void ausgabe(); // NUR FÜR TESTZWECKE

vector<vector<vector<vector<long double>>>> operator+=(vector<vector<vector<vector<long double>>>>& vec1, vector<vector<long double>>& vec2); // Operatorüberladung, um Divergenz zum Wert zu addieren (NUR FÜR DRUCK, DA IMMER DER INDEX 0 ANGENOMMEN WIRD

/*
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
	void check_cfl(double umax, long double dist); //Ueberpruefung der CFL-Zahl
private:
	const double start = 0;
	const double end = 0;
	const long double time_stp = 0;
};
*/