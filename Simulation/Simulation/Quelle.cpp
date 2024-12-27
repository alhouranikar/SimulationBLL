#include "Header.h"

vector<vector<vector<long double>>> values;
unique_ptr<const double> start;
unique_ptr<const double> ende;
unique_ptr<double> time_stp;
long double glb_time;
size_t size_x;
size_t size_y;
unsigned int current_posx;
unsigned int current_posy;
long double umax;

double dist; //	NUR FÜR TESTZWECKE

void set_initial()
{
	// Beispiel
	size_x = 10;
	size_y = 10;
	umax = 0.0;
	create_mesh(); // Aufruf, um Mesh entsprechender Größe zu erstellen // VERBESSERUNG: Methode create_mesh verkapseln
	for (int i = 0; i < size_x; ++i)
	{
		values.at(i).at(0).at(0) = 1;
		if (i == 5)
		{
			values.at(i).at(0).at(0) = 4;
		}
		values.at(i).at(0).at(1) = 2;
		umax = (values.at(i).at(0).at(1) > umax) ? values.at(i).at(0).at(1) : umax; // Wenn die Geschwindigkeit größer als umax ist, dann neue maximale Geschwindigkeit setzen
	} 
	dist = 0.043; // NUR FÜR TESTZWECKE
	start = make_unique<const double>(0.0);
	ende = make_unique<const double>(1.0);
	glb_time = 0.0;
	current_posx = 0;
	current_posy = 0;
	calc_dt(umax, dist); // Berechnung des Zeitschritts
}

void create_mesh()
{
	values = vector<vector<vector<long double>>>(size_x, vector<vector<long double>>(size_y, vector<long double>(2))); // 2 Argumente beim long double vector, um Druck und Geschwindigkeit zu speichern (1. Eintrag Druck, 2. Eintrag Geschwindigkeit), x Zeilen und y Spalten // VERBESSERUNG: mehrere Werte speichern, um mehr Parameter zu haben
}

void next_frame()
{
	glb_time += *time_stp;
	for (int x = 0; x < size_x; ++x)
	{ // Alle Einträge im Mesh werden zu konstanter Zeit aktualisiert
		current_posx = x;
		for (int y = 0; y < size_y; ++y)
		{
			current_posy = y;
			calc_pressure();
			calc_vel();
		}
	}
	calc_dt(umax, dist);
}

void calc_dt(double umax, long double dist)
{
	time_stp = make_unique<double>(dist / umax); // Berechnung von delta t anhand von der CFL-Bedingung //	VERBESSERUNG: Konstante k als Benutzereingabe vor delta t, um Rechenzeit zu verkürzen
	return;
}

/*void call_frame(unsigned int posx, unsigned int posy)
{
	calc_pressure();
	calc_vel();
}
*/

void calc_pressure()
{
	values.at(current_posx).at(current_posy).at(0) += 2;
	// Schleife über alle Punkte im Mesh, um Laufzeit zu sparen (ca. 1.5x schneller), nicht implementieren, da zwei Schleifen also auch für die Geschwindigkeit durchlaufen werden müssten
}

void calc_vel()
{
	values.at(current_posx).at(current_posy).at(1)++;
	if ((current_posx == size_x - 1 || current_posy == size_y - 1) || (current_posx == 0 || current_posy == 0))
	{
		values.at(current_posx).at(current_posy).at(1) = 0;
	}
	if (values.at(current_posx).at(current_posy).at(1) <= umax)
	{ // ggf. Update von umax
		return;
	}
	umax = values.at(current_posx).at(current_posy).at(1);
}

vector<double> get_nval(string pv) // string für Geschwindigkeit / Druck-wechsel
{
	int i = 0;
	if (pv == "p")
	{
		i = 0;
	}
	else if (pv == "v")
	{
		i = 1;
	}
	else
	{
		cout << "Fehler";
		return {};
	}
	vector<double> erg(4);
	erg.at(0) = values.at(current_posx - 1).at(current_posy).at(i);
	erg.at(1) = values.at(current_posx).at(current_posy + 1).at(i);
	erg.at(2) = values.at(current_posx + 1).at(current_posy).at(i);
	erg.at(3) = values.at(current_posx).at(current_posy - 1).at(i); // Nachbarn werden im Uhrzeigersinn durchlaufen, Beginn im Norden
	return erg;
}

void ausgabe()
{
	std::cout << "Druckfeld zum Zeitpunkt " << glb_time << ":\n";
	for (int x = 0; x < size_x; x++)
	{
		for (int y = 0; y < size_y; ++y)
		{
			cout << values.at(x).at(y).at(0) << " ";
		}
		cout << "\n";
	}
	std::cout << "Geschwindigkeitsfeld zum Zeitpunkt " << glb_time << ":\n";
	for (int x = 0; x < size_x; x++)
	{
		for (int y = 0; y < size_y; ++y)
		{
			cout << values.at(x).at(y).at(1) << " ";
		}
		cout << "\n";
	}
	cout << "Die maximale Geschwindigkeit beträgt " << umax << ".\n";
}