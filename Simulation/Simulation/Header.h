

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
extern vector<vector<bool>> is_boundary; // Vector, welcher sagt, ob Punkt boundary ist oder nicht, um Rechenzeit zu sparen
extern vector<vector<bool>> is_solid; // zeigt, welche Zellen im Mesh boundaries sind
extern vector<vector<bool>> is_inflow;
extern vector<vector<bool>> is_outflow;
// Alle Variablen, die mit der Zeit zusammenhängen
extern unique_ptr<const double> start;
extern unique_ptr<const double> ende;
extern unique_ptr<double> time_stp;
extern double glb_time;
extern unsigned int frame_anz;
extern int akt_frame;

// Alle Variablen, die mit dem Mesh zusammenhängen
// Vectoren, die das Mesh wiedergeben, werden in create_mesh erstellt, damit die gewünschte Größe beim initialisieren angeben werden kann
extern size_t size_x;
extern size_t size_y;
extern unsigned int nachbarn; // Anzahl der benachbarten Zellen
extern vector<vector<vector<long double>>> temp;
extern vector<long double> temp_nval; // Vektor, welcher die einer Zelle benachbarten Werte speichert

// Alle Variablen, die mit den einzelnen Frames zusammenhängen
extern int current_posx;
extern int current_posy;
extern double past_posx_x; // vorherige x-Position der Geschwindigkeit in x-Richtung
extern double past_posy_x;
extern double past_posx_y; // vroherige x-Position der Geschwindigkeit in y-Richtung
extern double past_posy_y;
extern long double umax;
extern long double density;

extern vector<vector<long double>> erg_scal_mul; // Ergebnis der skalaren Multiplikation

extern vector<vector<long double>> koeff_x;
extern vector<vector<long double>> koeff_y;
extern vector<vector<vector<long double>>> vel_y;
extern vector<vector<vector<long double>>> pressure_x;
extern vector<vector<vector<long double>>> pressure_y;
extern vector<vector<vector<long double>>> pressure;
extern vector<vector<long double>> grav;
extern vector<vector<long double>> geschw_rest_x; // der Rest der Geschwindigkeit, welcher übrig bleibt bei der zeitlichen Diskretisierung der Geschwindigkeit
extern vector<vector<long double>> geschw_rest_y; // der Rest der Geschwindigkeit, welcher übrig bleibt bei der zeitlichen Diskretisierung der Geschwindigkeit
extern vector<vector<long double>> temp_pr;
extern vector<vector<long double>> temp_guess;
extern vector<vector<long double>> Z_nx; // nicht diagonaler Teil der Koeffizientenmatrix Z_x
extern vector<vector<long double>> Z_dx; // invertierter diagonaler Teil der Koeffizientenmatrix Z_x
extern vector<vector<long double>> Z_ny; // nicht diagonaler Teil der Koeffizientenmatrix Z_y
extern vector<vector<long double>> Z_dy; // invertierter diagonaler Teil der Koeffizientenmatrix Z_y
extern vector<vector<long double>> Px;
extern vector<vector<long double>> Py;
extern vector<vector<long double>> Ux;
extern vector<vector<long double>> Uy;


void import_obj(); // Verantwortlich dafür, verschiedene Geometries zu importieren

void create_mesh(); // Mesh erstellen

void set_initial(); // Ursprungszustand setzen, Startbedinugen auf Mesh ansetzen

void next_frame(); // Nächsten Frame aufrufen

void calc_dt(double umax, long double dist); // errechnet den Zeitschritt anhand der CFL-Bedingung

void check_umax(vector<long double>& speed); // Überprüft, ob die maximale Geschwindigkeit übertroffen wird

//void call_frame(unsigned int posx, unsigned int posy); // Ruft den nachfolgenden Frame auf

void calc_pressure(); // Berechnet den Druck an jedem Ort zu einem Zeitpunkt

void calc_vel(); // Berechnet die Geschwindigkeit anhand des Drucks

vector<long double> get_nval(int w); // FÜR TESTZWECKE, Ausgabe der benachbarten Werte, w steht für verschiedene Werte: 0 = Druck, 1 = Geschw. in x-Richtung, 2 = Geschw. in y-Richtung

long double calc_avg(int i); // Berechnung des Durchschnitts benachbarter Werte

vector<long double> interpolate(); // Funktion, um die der Ursprungsposition der Teilchen nähesten Zelle zu finden

void ausgabe(); // NUR FÜR TESTZWECKE

void advect(); // Berechnung der Advektion

void ext_force(); // Einbezug externer Kräfte

vector<vector<vector<vector<long double>>>>& operator+=(vector<vector<vector<vector<long double>>>>& vec1, vector<vector<long double>>& vec2); // Operatorüberladung, um Divergenz zum Wert zu addieren (NUR FÜR DRUCK, DA IMMER DER INDEX 0 ANGENOMMEN WIRD

vector<vector<long double>> operator*(const long double scalar, const vector<vector<long double>>& vec);

vector<vector<long double>> operator+(const vector<vector<long double>>& summand1, const vector<vector<long double>>& summand2);

vector<vector<long double>> operator*(const vector<vector<long double>>& faktor1, vector<vector<long double>>& faktor2);

vector<vector<long double>> operator/(const long double dividend, vector<vector<long double>>& divisor);

//vector<vector<long double>> cgm(vector<vector<long double>>& koeff, vector<vector<long double>>& b1, vector<vector<long double>>& init); // Conjugate-Gradient-Methode

long double calc_Hn(int i);

// Alles für Matrizenrechnung

namespace Matrix
{
	// CGM-Methode:
	extern vector<vector<long double>> initial;
	extern vector<vector<long double>> field;
	extern vector<vector<long double>> coefficient;
	extern vector<vector<long double>> b;

	extern vector<vector<long double>> direction;
	extern vector<vector<long double>> erg_scal_mul; // Speicherung des Ergebnisses von scal_mul
	extern vector<vector<long double>> erg;

	vector<vector<long double>>& operator*(const long double scalar, const vector<vector<long double>>& vec);
	template<typename T>
	T convert(const vector<vector<T>>& vec);
	template<typename T>
	vector<vector<T>> transpose(const vector<vector<T>>& vec);
	vector<vector<long double>> operator-(const vector<vector<long double>>& minuend, const vector<vector<long double>>& subtrahend);
	template<typename T, typename T2>
	vector<vector<long double>>& operator*(const vector<vector<T>>& mat1, const vector<vector<T2>>& mat2);
	vector<vector<long double>> cgm(vector<vector<long double>>& koeff, vector<vector<long double>>& b1, vector<vector<long double>>& init);
}

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