#include "Header.h"

vector<vector<vector<vector<long double>>>> frames; // VERBESSERUNG: Aufspaltung der Liste, um Zugriffszeit zu minimieren
vector<vector<bool>> is_boundary;
vector<vector<bool>> is_solid;
vector<vector<bool>> is_inflow;
vector<vector<bool>> is_outflow;
unique_ptr<const double> start;
unique_ptr<const double> ende;
unique_ptr<double> time_stp;
double glb_time;
unsigned int frame_anz; // Anzahl der Frames
int akt_frame; // aktueller Frame
size_t size_x;
size_t size_y;
int current_posx;
int current_posy;
double past_posx_x;
double past_posy_x;
double past_posx_y;
double past_posy_y;
long double umax;
unsigned int nachbarn;
vector<vector<vector<long double>>> temp; // VERBESSERUNG: besser kopieren, Erstellung einer Kopie des Vektorfelds zum Zeitpunkt glb_time
vector<long double> temp_nval;
double dist; 
long double density;
vector<long double> x_vec(2, 0); // Vektor, welcher die Richtung der Geschwindigkeit für die x-Komponente speichert, später lokal definieren
vector<long double> y_vec(2, 0); // Vektor, welcher die Richtung der Geschwindigkeit für die y-Komponente speichert (zweiter Eintrag der Geschwindigkeit in frames

vector<vector<long double>> koeff_x;
vector<vector<long double>> koeff_y;
vector<vector<vector<long double>>> vel_x;
vector<vector<vector<long double>>> vel_y;
vector<vector<vector<long double>>> pressure_x;
vector<vector<vector<long double>>> pressure_y;
vector<vector<vector<long double>>> pressure;
vector<vector<long double>> grav;
vector<vector<long double>> geschw_rest_x; // der Rest der Geschwindigkeit, welcher übrig bleibt bei der zeitlichen Diskretisierung der Geschwindigkeit
vector<vector<long double>> geschw_rest_y; // der Rest der Geschwindigkeit, welcher übrig bleibt bei der zeitlichen Diskretisierung der Geschwindigkeit
vector<vector<long double>> temp_pr;
vector<vector<long double>> temp_guess;
vector<vector<long double>> Z_nx; // nicht diagonaler Teil der Koeffizientenmatrix Z_x
vector<vector<long double>> Z_dx; // invertierter diagonaler Teil der Koeffizientenmatrix Z_x
vector<vector<long double>> Z_ny; // nicht diagonaler Teil der Koeffizientenmatrix Z_y
vector<vector<long double>> Z_dy; // invertierter diagonaler Teil der Koeffizientenmatrix Z_y
vector<vector<long double>> Px;
vector<vector<long double>> Py;
vector<vector<long double>> Ux;
vector<vector<long double>> Uy;

vector<vector<vector<long double>>> temp_vel;

void set_initial()
{
	// Beispiel
	size_x = 10;
	size_y = 10;
	umax = 2.0; // MUSS SCHON VOR DER ERSTELLUNG DES MESHS BEKANNT SEIN, TEIL DER ANFANGSBEDINGUNGEN
	start = make_unique<const double>(0.0);
	ende = make_unique<const double>(1.0);
	glb_time = 0.0;
	akt_frame = 0;
	dist = 0.043; // NUR FÜR TESTZWECKE
	calc_dt(umax, dist); // Berechnung des Zeitschritts
	frame_anz = static_cast<int>(((*ende) - (*start)) / (*time_stp)); // Berechnet die Anzahl der Frames
	ende = make_unique<const double>(frame_anz * (*time_stp)); // Neu Berechnung der Endzeit, damit glatte Zahl an Frames rauskommt // VERBESSERUNG: ohne Runden schaffen
	create_mesh(); // Aufruf, um Mesh entsprechender Größe zu erstellen // VERBESSERUNG: Methode create_mesh verkapseln
	for (int i = 0; i < size_x; ++i)
	{
		// Druck vorgeben!!!
		for (int j = 0; j < size_y; ++j)
		{
			frames.at(0).at(i).at(j).at(0) = 1;
			frames.at(0).at(i).at(j).at(1) = 1;
			frames.at(0).at(i).at(j).at(2) = 1;
			if (j == 5 && i == 5)
			{
				frames.at(0).at(i).at(j).at(1) = 1.8;
			}
		}
		check_umax(frames.at(akt_frame).at(current_posx).at(current_posy)); // Wenn die Geschwindigkeit größer als umax ist, dann neue maximale Geschwindigkeit setzen
	}
	is_inflow = vector<vector<bool>>(size_x, vector<bool>(size_y));
	is_outflow = vector<vector<bool>>(size_x, vector<bool>(size_y));
	is_solid = vector<vector<bool>>(size_x, vector<bool>(size_y));
	is_boundary = vector<vector<bool>>(size_x, vector<bool>(size_y));
	for (int i = 0; i < size_x; ++i)
	{ // size_x und size_y müssen gleich sein
		is_solid.at(0).at(i) = true;
		frames.at(0).at(0).at(i).at(1) = 0;
		frames.at(0).at(0).at(i).at(2) = 0;
		is_boundary.at(0).at(i) = true;
		is_boundary.at(1).at(i) = true;
		is_inflow.at(i).at(0) = true;
		is_boundary.at(i).at(0) = true;
		is_boundary.at(i).at(1) = true;
		is_solid.at(size_x - 1).at(i) = true;
		frames.at(0).at(size_x-1).at(i).at(1) = 0;
		frames.at(0).at(size_x-1).at(i).at(2) = 0;
		is_boundary.at(size_x - 1).at(i) = true;
		is_boundary.at(size_x - 2).at(i) = true;
		is_outflow.at(i).at(size_y-1) = true;
		is_boundary.at(i).at(size_y-1) = true;
		is_boundary.at(i).at(size_y - 2) = true;
	}
	current_posx = 0;
	current_posy = 0;
	for (int i = 0; i < size_x; ++i)
	{
		for (int j = 0; j < size_y; ++j)
		{
			cout << is_boundary.at(i).at(j) << " ";
		}
		cout << endl;
	}
	density = 1.0; // NUR FÜR TESTZWECKE
}

void create_mesh()
{
	// Initialisierung aller Variablen
	frames = vector<vector<vector<vector<long double>>>>(frame_anz + 1, vector<vector<vector<long double>>>(size_x, vector<vector<long double>>(size_y, vector<long double>(3)))); // 3 Argumente beim long double vector, um Druck und Geschwindigkeit (in x- und y-Richtung) zu speichern (1. Eintrag Druck, 2. Eintrag Geschwindigkeit in x-Richtung, 3. Eintrag Geschwindigkeit in y-Richtung), x Zeilen und y Spalten // VERBESSERUNG: mehrere Werte speichern, um mehr Parameter zu haben; +1, damit der urpsrüngliche Frame auch gespeichert werden kann
	koeff_x = vector<vector<long double>>(size_x * size_y, vector<long double>(size_x * size_y));
	koeff_y = vector<vector<long double>>(size_x * size_y, vector<long double>(size_x * size_y));
	vel_x = vector<vector<vector<long double>>>(3, vector<vector<long double>>(size_x * size_y, vector<long double>(1)));
	vel_y = vector<vector<vector<long double>>>(3, vector<vector<long double>>(size_x * size_y, vector<long double>(1)));
	pressure_x = vector<vector<vector<long double>>>(3, vector<vector<long double>>(size_x * size_y, vector<long double>(1)));
	pressure_y = vector<vector<vector<long double>>>(3, vector<vector<long double>>(size_x * size_y, vector<long double>(1)));
	pressure = vector<vector<vector<long double>>>(3, vector<vector<long double>>(size_x, vector<long double>(size_y)));
	grav = vector<vector<long double>>(size_x * size_y, vector<long double>(1));
	geschw_rest_x = vector<vector<long double>>(size_x * size_y, vector<long double>(1)); // der Rest der Geschwindigkeit, welcher übrig bleibt bei der zeitlichen Diskretisierung der Geschwindigkeit
	geschw_rest_y = vector<vector<long double>>(size_x * size_y, vector<long double>(1)); // der Rest der Geschwindigkeit, welcher übrig bleibt bei der zeitlichen Diskretisierung der Geschwindigkeit
	temp_guess = vector<vector<long double>>(size_x * size_y, vector<long double>(1));
	Z_nx = vector<vector<long double>>(size_x * size_y, vector<long double>(size_x * size_y)); // nicht diagonaler Teil der Koeffizientenmatrix Z_x
	Z_dx = vector<vector<long double>>(size_x * size_y, vector<long double>(size_x * size_y)); // invertierter diagonaler Teil der Koeffizientenmatrix Z_x
	Z_ny = vector<vector<long double>>(size_x * size_y, vector<long double>(size_x * size_y)); // nicht diagonaler Teil der Koeffizientenmatrix Z_y
	Z_dy = vector<vector<long double>>(size_x * size_y, vector<long double>(size_x * size_y)); // invertierter diagonaler Teil der Koeffizientenmatrix Z_y
	Px = vector<vector<long double>>(size_x * size_y, vector<long double>(1));
	Py = vector<vector<long double>>(size_x * size_y, vector<long double>(1));
	Ux = vector<vector<long double>>(size_x * size_y, vector<long double>(1));
	Uy = vector<vector<long double>>(size_x * size_y, vector<long double>(1));
}

void next_frame()
{
	++akt_frame;
	current_posx = current_posy = 0;

	// ab akt_frame - 3 Einträge löschen!
	for (int n = 0; n < 101; ++n)
	{
		if (n == 0)
		{
			frames.at(akt_frame) = frames.at(akt_frame - 1); // Mesh zum aktuellen Zeitpunkt = vorheriger Zeitpunkt
			for (int g = 0; g < size_x; ++g)
			{
				for (int h = 0; h < size_y; ++h)
				{ // die anfängliche Abschätzung der Geschwindigkeit ist durch die Geschwindigkeit zum vorherigen Frame gegeben // VERBESSERUNG: erste Abschätzung ist vorheriger Zeitpunkt mit zusätzlicher Diskretisierung
					const size_t z = g * size_y + h;
					vel_x.at(2).at(z).at(0) = frames.at(akt_frame - 1).at(g).at(h).at(1);
					vel_y.at(2).at(z).at(0) = frames.at(akt_frame - 1).at(g).at(h).at(2);
				}
			}
		}
		pressure_x.at(1) = pressure_x.at(2);
		pressure_x.at(0) = pressure_x.at(1);
		pressure_y.at(1) = pressure_y.at(2);
		pressure_y.at(0) = pressure_y.at(1);
		vel_x.at(1) = vel_x.at(2);
		vel_x.at(0) = vel_x.at(1);
		vel_y.at(1) = vel_y.at(2);
		vel_y.at(0) = vel_y.at(1);
		for (int i = 0; i < size_x; ++i)
		{
			for (int j = 0; j < size_y; ++j)
			{
				if (is_boundary.at(i).at(j))
					continue;
				const size_t z = i * size_y + j;
				size_t zz = z - 2 * size_y;
				/*
				if (z < 0)
					z = 0; // nur, wenn auch boundary-punkte überprüft werden
				*/
				koeff_x.at(z).at(z) = 3 / (2 * (*time_stp));
				koeff_y.at(z).at(z) = 3 / (2 * (*time_stp));
				// Bereich für x-Komponente:
				// Geschwindigkeit in y-Richtung für x-Komponente
				koeff_x.at(z).at(z - 2) = vel_y.at(1).at(z - 2 * size_y).at(0) / (12 * dist);
				koeff_x.at(z).at(z - 1) = vel_y.at(1).at(z - size_y).at(0) / (12 * dist);
				koeff_x.at(z).at(z + 1) = 8 * vel_y.at(1).at(z + size_y).at(0) / (12 * dist);
				koeff_x.at(z).at(z + 2) = -vel_y.at(1).at(z + 2 * size_y).at(0) / (12 * dist);
				// Geschwindigkeit in x-Richtung für x-Komponente und für y-Komponente
				if (zz >= 0)
				{
					koeff_x.at(z).at(zz) = vel_x.at(1).at(zz).at(0) / (12 * dist);
					koeff_y.at(z).at(zz) = vel_x.at(1).at(zz).at(0) / (12 * dist);
				}
				zz = z - size_y;
				if (zz >= 0)
				{
					koeff_x.at(z).at(zz) = -8 * vel_x.at(1).at(zz).at(0) / (12 * dist);
					koeff_y.at(z).at(zz) = -8 * vel_x.at(1).at(zz).at(0) / (12 * dist);
				}
				zz = z + size_y;
				if (zz <= size_y * size_x - 1)
				{
					koeff_x.at(z).at(zz) = 8 * vel_x.at(1).at(zz).at(0) / (12 * dist);
					koeff_y.at(z).at(zz) = 8 * vel_x.at(1).at(zz).at(0) / (12 * dist);
				}
				zz = z + 2 * size_y;
				if (zz <= size_y * size_x - 1)
				{
					koeff_x.at(z).at(zz) = -vel_x.at(1).at(zz).at(0) / (12 * dist);
					koeff_y.at(z).at(zz) = -vel_x.at(1).at(zz).at(0) / (12 * dist);
				}
				// Bereich für y-Komponente
				// Geschwindigkeit in y-Richtung für y-Komponente
				koeff_y.at(z).at(z - 2) = vel_y.at(1).at(z - 2).at(0) / (12 * dist);
				koeff_y.at(z).at(z - 1) = -8 * vel_y.at(1).at(z - 1).at(0) / (12 * dist);
				koeff_y.at(z).at(z + 1) = 8 * vel_y.at(1).at(z + 1).at(0) / (12 * dist);
				koeff_y.at(z).at(z + 2) = -vel_y.at(1).at(z + 2).at(0) / (12 * dist);
				// Druckgradienten in x/y-Richtung berechnen:
				pressure_x.at(2).at(z).at(0) = (pressure.at(1).at(i - 2).at(j) - 8 * pressure.at(1).at(i - 1).at(j) + 8 * pressure.at(1).at(i + 1).at(j) - pressure.at(1).at(i + 2).at(j)) / (12 * dist);
				pressure_y.at(2).at(z).at(0) = (pressure.at(1).at(i).at(j - 2) - 8 * pressure.at(1).at(i).at(j - 1) + 8 * pressure.at(1).at(i).at(j + 1) - pressure.at(1).at(i).at(j + 2)) / (12 * dist);
				// Gravitationskraft und andere externe Kräfte werden hier gespeichert
				grav.at(z).at(0) = density * 9.81;
				// zeitliche Entwicklung der Geschwindigkeit
 				if(akt_frame - 2 < 0) // Fall akt_frame - 1 < 0 kann nicht eintreten, da next_frame erst ab dem Frame 1 verwendet wird
				{ // Buffer-overflow, da akt_frame eine positive Zahl sein muss (behoben)
					geschw_rest_x.at(z).at(0) = (-4 / (12 * (*time_stp))) * frames.at(akt_frame - 1).at(i).at(j).at(1) + (1 / (12 * (*time_stp))) * frames.at(akt_frame - 1).at(i).at(j).at(1);
					geschw_rest_y.at(z).at(0) = (-4 / (12 * (*time_stp))) * frames.at(akt_frame - 1).at(i).at(j).at(2) + (1 / (12 * (*time_stp))) * frames.at(akt_frame - 1).at(i).at(j).at(2);
				}
				else
				{
					geschw_rest_x.at(z).at(0) = (-4 / (12 * (*time_stp))) * frames.at(akt_frame - 1).at(i).at(j).at(1) + (1 / (12 * (*time_stp))) * frames.at(akt_frame - 2).at(i).at(j).at(1);
					geschw_rest_y.at(z).at(0) = (-4 / (12 * (*time_stp))) * frames.at(akt_frame - 1).at(i).at(j).at(2) + (1 / (12 * (*time_stp))) * frames.at(akt_frame - 2).at(i).at(j).at(2);
				}
			}
		}

		//Geschwindigkeit in x-Richtung berechnen
		temp_pr = grav + (-1) * pressure_x.at(2) + (-1) * geschw_rest_x; // Gravitation als externe Kraft nur in x-Richtung hinzufügen
		temp_guess = vel_x.at(1); // die erste Abschätzung für conjuage-gradient ist die Geschwindigkeit zur vorherigen Iteration
		vel_x.at(2) = Matrix::cgm(koeff_x, temp_pr, temp_guess);

		temp_pr = (-1) * pressure_y.at(2) + (-1) * geschw_rest_x;

		//Geschwindigkeit in y-Richtung berechnen
		temp_guess = vel_y.at(1);
		vel_y.at(2) = Matrix::cgm(koeff_y, temp_pr, temp_guess);

		// Berechnung des Drucks mithilfe einer Poisson-Gleichung für den Druck und der soeben errrechneten Geschwindigkeit
		for (int i = 0; i < size_x; ++i)
		{
			current_posx = i;
			for (int j = 0; j < size_y; ++j)
			{
				current_posy = j;
				if (is_boundary.at(i).at(j))
					continue;
				const size_t z = i * size_y + j;
				const size_t zz = z - 2 * size_y;
				long double temp_u_x = (-vel_x.at(2).at(z + 2 * size_y).at(0) + 16 * vel_x.at(2).at(z + size_y).at(0) - 30 * vel_x.at(2).at(z).at(0) + 16 * vel_x.at(2).at(z - size_y).at(0) - vel_x.at(2).at(z - 2 * size_y).at(0)) / (12 * dist * dist);
				long double temp_v_y = (-vel_y.at(2).at(z + 2).at(0) + 16 * vel_y.at(2).at(z + 1).at(0) - 30 * vel_y.at(2).at(z ).at(0) + 16 * vel_y.at(2).at(z  - 1).at(0) - vel_y.at(2).at(z  - 2).at(0)) / (12 * dist * dist); // NUR FÜR DELTA X = DELTA Y
				long double temp_uv_xy = (vel_x.at(2).at(z - 2 * size_y - 2).at(0) * vel_y.at(2).at(z - 2 * size_y - 2).at(0) - 8 * vel_x.at(2).at(z - size_y - 2).at(0) * vel_y.at(2).at(z - size_y - 2).at(0) + 8 * vel_x.at(2).at(z + size_y - 2).at(0) * vel_y.at(2).at(z + size_y - 2).at(0) - vel_x.at(2).at(z + 2 * size_y - 2).at(0) * vel_y.at(2).at(z + 2 * size_y - 2).at(0) - 8 * (vel_x.at(2).at(z - 2 * size_y - 1).at(0) * vel_y.at(2).at(z - 2 * size_y - 1).at(0) - 8 * vel_x.at(2).at(z - size_y - 1).at(0) * vel_y.at(2).at(z - size_y - 1).at(0) + 8 * vel_x.at(2).at(z + size_y - 1).at(0) * vel_y.at(2).at(z + size_y - 1).at(0) - vel_x.at(2).at(z + 2 * size_y - 1).at(0) * vel_y.at(2).at(z + 2 * size_y - 1).at(0)) + 8 * (vel_x.at(2).at(z - 2 * size_y + 1).at(0) * vel_y.at(2).at(z - 2 * size_y + 1).at(0) - 8 * vel_x.at(2).at(z - size_y + 1).at(0) * vel_y.at(2).at(z - size_y + 1).at(0) + 8 * vel_x.at(2).at(z + size_y + 1).at(0) * vel_y.at(2).at(z + size_y + 1).at(0) - vel_x.at(2).at(z + 2 * size_y + 1).at(0) * vel_y.at(2).at(z + 2 * size_y + 1).at(0)) - (vel_x.at(2).at(z - 2 * size_y + 2).at(0) * vel_y.at(2).at(z - 2 * size_y + 2).at(0) - 8 * vel_x.at(2).at(z - size_y + 2).at(0) * vel_y.at(2).at(z - size_y + 2).at(0) + 8 * vel_x.at(2).at(z + size_y + 2).at(0) * vel_y.at(2).at(z + size_y + 2).at(0) - vel_x.at(2).at(z + 2 * size_y + 2).at(0) * vel_y.at(2).at(z + 2 * size_y + 2).at(0))) / (12 * dist * dist); // AUFPASSEN MIT UNTERSCHIEDLICHEN MESH-GRÖßEN, DA DELTA X AN VERSCHEIDENEN ORTEN DANN ANDERS SEIN KANN
				pressure.at(2).at(current_posx).at(current_posy) = ((12 * dist * dist) * (-density * (temp_u_x + 2 * temp_uv_xy + temp_v_y)) + pressure.at(1).at(current_posx + 2).at(current_posy) - 16 * pressure.at(1).at(current_posx + 1).at(current_posy) - 16 * pressure.at(1).at(current_posx - 1).at(current_posy) + pressure.at(1).at(current_posx - 2).at(current_posy) + pressure.at(1).at(current_posx).at(current_posy + 2) - 16 * pressure.at(1).at(current_posx).at(current_posy + 1) - 16 * pressure.at(1).at(current_posx).at(current_posy - 1) + pressure.at(1).at(current_posx).at(current_posy - 2)) / (-60);
			}
		}

		// Berechnung der Geschwindigkeit mithilfe des korrigierten Drucks
		for (int x = 0; x < size_x*size_y; ++x)
		{
			for (int y = 0; y < size_x*size_y; ++y)
			{
				if (x == y)
				{
					Z_dx.at(x).at(y) = 1 / koeff_x.at(x).at(y);
					Z_dy.at(x).at(y) = 1 / koeff_y.at(x).at(y);
					continue;
				}
			}
		}
		Z_nx = koeff_x * vel_x.at(2);
		Z_ny = koeff_y * vel_y.at(2);
		Px = (-1) * grav + geschw_rest_x;
		Py = geschw_rest_y;
		for (int i = 0; i < size_x; ++i)
		{
			for (int j = 0; j < size_y; ++j)
			{
				if (is_boundary.at(i).at(j))
					continue;
				const size_t z = i * size_y + j;
				Px.at(z).at(0) -= (pressure.at(2).at(i - 2).at(j) - 8 * pressure.at(2).at(i - 1).at(j) + 8 * pressure.at(2).at(i + 1).at(j) - pressure.at(2).at(i + 2).at(j)) / (12 * dist);
				Py.at(z).at(0) -= (pressure.at(2).at(i).at(j - 2) - 8 * pressure.at(2).at(i).at(j - 1) + 8 * pressure.at(2).at(i).at(j + 1) - pressure.at(2).at(i).at(j + 2)) / (12 * dist);
			}
		}
		Ux = Z_dx * Px + (-1) * Z_dx * Z_nx;
		Uy = Z_dy * Py + (-1) * Z_dy * Z_ny;
		vel_x.at(2) = Ux;
		vel_y.at(2) = Uy;
		// HIER WEITER : IMPLEMENTIERUNG VON GLEICHUNG 4.8

	}
	for (int i = 0; i < size_x; ++i)
	{
		for (int j = 0; j < size_y; ++j)
		{
			const size_t z = i * size_y + j;
			frames.at(akt_frame).at(i).at(j).at(0) = pressure.at(2).at(i).at(j);
			frames.at(akt_frame).at(i).at(j).at(1) = vel_x.at(2).at(z).at(0);
			frames.at(akt_frame).at(i).at(j).at(2) = vel_y.at(2).at(z).at(0);
		}
	}
	/*
	for (int i = 0; i < size_x; ++i)
	{
		current_posx = i;
		for (int j = 0; j < size_y; ++j)
		{
			current_posy = j;
			if (is_boundary.at(current_posx).at(current_posy))
			{
				continue;
			}
			vector<long double> Hni(2);
			Hni.at(0) = calc_Hn(1);
			Hni.at(1) = calc_Hn(2);
			frames.at(akt_frame).at(current_posx).at(current_posy).at(1) = (*time_stp) * (Hni.at(0) - ((frames.at(akt_frame).at(current_posx - 2).at(current_posy).at(0)
				- 8 * frames.at(akt_frame).at(current_posx - 1).at(current_posy).at(0)
				+ 8 * frames.at(akt_frame).at(current_posx + 1).at(current_posy).at(0)
				- frames.at(akt_frame).at(current_posx + 2).at(current_posy).at(0))
				/ (12 * dist)));
			frames.at(akt_frame).at(current_posx).at(current_posy).at(2) = (*time_stp) * (Hni.at(1) - ((frames.at(akt_frame).at(current_posx).at(current_posy - 2).at(0)
				- 8 * frames.at(akt_frame).at(current_posx).at(current_posy - 1).at(0)
				+ 8 * frames.at(akt_frame).at(current_posx).at(current_posy + 1).at(0)
				- frames.at(akt_frame).at(current_posx).at(current_posy + 2).at(0))
				/ (12 * dist)));
		}
	}
	cout << "jfoejafoiepjaoefjafpjeasifjeoisaf" << endl;
	for (int u = 0; u < size_x; ++u)
	{
		current_posx = u;
		for (int v = 0; v < size_y; ++v) {
			current_posy = v;
			int i = 1;
			int j = 1;
			if (is_boundary.at(current_posx).at(current_posy))
			{
				continue;
			}
			long double temp = (frames.at(akt_frame).at(current_posx - 2).at(current_posy).at(i) * frames.at(akt_frame).at(current_posx).at(current_posy).at(j)
				- 8 * frames.at(akt_frame).at(current_posx - 1).at(current_posy).at(i) * frames.at(akt_frame).at(current_posx).at(current_posy).at(j)
				+ 8 * frames.at(akt_frame).at(current_posx + 1).at(current_posy).at(i) * frames.at(akt_frame).at(current_posx).at(current_posy).at(j)
				- frames.at(akt_frame).at(current_posx + 2).at(current_posy).at(i) * frames.at(akt_frame).at(current_posx).at(current_posy).at(j))
				/ (12 * dist);
			j = 2;
			temp += (frames.at(akt_frame).at(current_posx - 2).at(current_posy).at(i) * frames.at(akt_frame).at(current_posx).at(current_posy).at(j)
				- 8 * frames.at(akt_frame).at(current_posx - 1).at(current_posy).at(i) * frames.at(akt_frame).at(current_posx).at(current_posy).at(j)
				+ 8 * frames.at(akt_frame).at(current_posx + 1).at(current_posy).at(i) * frames.at(akt_frame).at(current_posx).at(current_posy).at(j)
				- frames.at(akt_frame).at(current_posx + 2).at(current_posy).at(i) * frames.at(akt_frame).at(current_posx).at(current_posy).at(j))
				/ (12 * dist);
			cout << temp << " ";
		}
		cout << endl;
	}
	//calc_vel();
	// VERBESSERUNG: calc_pressure und calc_vel in eine Schleife zusammenfassen
	//calc_dt(umax, dist); //	VERBESSERUNG: überprüfen, ob dt überhaupt geändert werden muss
	//glb_time += *time_stp;
/*	if (((*ende) - glb_time) / (*time_stp) > frame_anz - akt_frame)
	{ // wenn nach neuem Zeitschritt mehr Frames benötigt werden // VERBESSERUNG: (dringend) überprüfen, ob resize sich lohnt, wenn der neue Zeitschritt größer ist 
		frame_anz = static_cast<int>(((*ende) - (*start)) / (*time_stp));
		frames.resize(frame_anz);
		ende = make_unique<const double>(frame_anz * (*time_stp));
	}
	*/
}

long double calc_Hn(int i)
{
	if (i != 1 && i != 2)
	{
		cerr << "Fehler bei der Hn-Methode!" << endl;
		return 0.0;
	}
		int j = 1;
		long double erg = (-density * (frames.at(akt_frame - 1).at(current_posx - 2).at(current_posy).at(i) * frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(j) - 8 * frames.at(akt_frame - 1).at(current_posx - 1).at(current_posy).at(i) * frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(j) + 8 * frames.at(akt_frame - 1).at(current_posx + 1).at(current_posy).at(i) * frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(j) - frames.at(akt_frame - 1).at(current_posx + 2).at(current_posy).at(i) * frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(j)) / (12 * dist));
		j = 2;
		erg += (-density * (frames.at(akt_frame - 1).at(current_posx - 2).at(current_posy).at(i) * frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(j) - 8 * frames.at(akt_frame - 1).at(current_posx - 1).at(current_posy).at(i) * frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(j) + 8 * frames.at(akt_frame - 1).at(current_posx + 1).at(current_posy).at(i) * frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(j) - frames.at(akt_frame - 1).at(current_posx + 2).at(current_posy).at(i) * frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(j)) / (12 * dist));
		if(i==2)
			erg += density * 9.81;
		return erg;
}

void calc_dt(double umax, long double dist)
{
	time_stp = make_unique<double>(0.9 * dist / umax); // Berechnung von delta t anhand von der CFL-Bedingung //	VERBESSERUNG: Konstante k als Benutzereingabe vor delta t, um Rechenzeit zu verkürzen (hier ist k 0.9)
	return;
}

void check_umax(vector<long double>& speed /* Frames - Vektor zur Zeit t eingeben */ )
{
	if (speed.at(1) > umax)
	{
		umax = speed.at(1);
	}
	if (-speed.at(1) > umax)
	{ // auch negative Geschwindigkeit überprüfen
		umax = -speed.at(1);
	}
	if (speed.at(2) > umax)
	{
		umax = speed.at(2);
	}
	if (-speed.at(2) > umax)
	{
		umax = -speed.at(2);
	}
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
	for (int k = 0; k < 1000; ++k)
	{
		for (int i = 0; i < size_x; ++i)
		{
			current_posx = i;
			for (int j = 0; j < size_y; ++j)
			{
				current_posy = j;
				if (is_boundary.at(current_posx).at(current_posy))
				{
					continue;
				}
				long double temp_u_x = (-frames.at(akt_frame).at(current_posx + 2).at(current_posy).at(1) + 16 * frames.at(akt_frame).at(current_posx + 1).at(current_posy).at(1) - 30 * frames.at(akt_frame).at(current_posx).at(current_posy).at(1) + 16 * frames.at(akt_frame).at(current_posx - 1).at(current_posy).at(1) - frames.at(akt_frame).at(current_posx - 2).at(current_posy).at(1)) / (12 * dist * dist);
				long double temp_v_y = (-frames.at(akt_frame).at(current_posx).at(current_posy + 2).at(2) + 16 * frames.at(akt_frame).at(current_posx).at(current_posy + 1).at(2) - 30 * frames.at(akt_frame).at(current_posx).at(current_posy).at(2) + 16 * frames.at(akt_frame).at(current_posx).at(current_posy - 1).at(2) - frames.at(akt_frame).at(current_posx).at(current_posy - 2).at(2)) / (12 * dist * dist); // NUR FÜR DELTA X = DELTA Y
				long double temp_uv_xy = (frames.at(akt_frame).at(current_posx - 2).at(current_posy - 2).at(1) * frames.at(akt_frame).at(current_posx - 2).at(current_posy - 2).at(2) - 8 * frames.at(akt_frame).at(current_posx - 1).at(current_posy - 2).at(1) * frames.at(akt_frame).at(current_posx - 1).at(current_posy - 2).at(2) + 8 * frames.at(akt_frame).at(current_posx + 1).at(current_posy - 2).at(1) * frames.at(akt_frame).at(current_posx + 1).at(current_posy - 2).at(2) - frames.at(akt_frame).at(current_posx + 2).at(current_posy - 2).at(1) * frames.at(akt_frame).at(current_posx + 2).at(current_posy - 2).at(2) - 8 * (frames.at(akt_frame).at(current_posx - 2).at(current_posy - 1).at(1) * frames.at(akt_frame).at(current_posx - 2).at(current_posy - 1).at(2) - 8 * frames.at(akt_frame).at(current_posx - 1).at(current_posy - 1).at(1) * frames.at(akt_frame).at(current_posx - 1).at(current_posy - 1).at(2) + 8 * frames.at(akt_frame).at(current_posx + 1).at(current_posy - 1).at(1) * frames.at(akt_frame).at(current_posx + 1).at(current_posy - 1).at(2) - frames.at(akt_frame).at(current_posx + 2).at(current_posy - 1).at(1) * frames.at(akt_frame).at(current_posx + 2).at(current_posy - 1).at(2)) + 8 * (frames.at(akt_frame).at(current_posx - 2).at(current_posy + 1).at(1) * frames.at(akt_frame).at(current_posx - 2).at(current_posy + 1).at(2) - 8 * frames.at(akt_frame).at(current_posx - 1).at(current_posy + 1).at(1) * frames.at(akt_frame).at(current_posx - 1).at(current_posy + 1).at(2) + 8 * frames.at(akt_frame).at(current_posx + 1).at(current_posy + 1).at(1) * frames.at(akt_frame).at(current_posx + 1).at(current_posy + 1).at(2) - frames.at(akt_frame).at(current_posx + 2).at(current_posy + 1).at(1) * frames.at(akt_frame).at(current_posx + 2).at(current_posy + 1).at(2)) - (frames.at(akt_frame).at(current_posx - 2).at(current_posy + 2).at(1) * frames.at(akt_frame).at(current_posx - 2).at(current_posy + 2).at(2) - 8 * frames.at(akt_frame).at(current_posx - 1).at(current_posy + 2).at(1) * frames.at(akt_frame).at(current_posx - 1).at(current_posy + 2).at(2) + 8 * frames.at(akt_frame).at(current_posx + 1).at(current_posy + 2).at(1) * frames.at(akt_frame).at(current_posx + 1).at(current_posy + 2).at(2) - frames.at(akt_frame).at(current_posx + 2).at(current_posy + 2).at(1) * frames.at(akt_frame).at(current_posx + 2).at(current_posy + 2).at(2))) / (12 * dist * dist); // AUFPASSEN MIT UNTERSCHIEDLICHEN MESH-GRÖßEN, DA DELTA X AN VERSCHEIDENEN ORTEN DANN ANDERS SEIN KANN
				frames.at(akt_frame).at(current_posx).at(current_posy).at(0) = ((12 * dist * dist) * ( - density * (temp_u_x + 2 * temp_uv_xy + temp_v_y)) + frames.at(akt_frame).at(current_posx + 2).at(current_posy).at(0) - 16 * frames.at(akt_frame).at(current_posx + 1).at(current_posy).at(0) - 16 * frames.at(akt_frame).at(current_posx - 1).at(current_posy).at(0) + frames.at(akt_frame).at(current_posx - 2).at(current_posy).at(0) + frames.at(akt_frame).at(current_posx).at(current_posy + 2).at(0) - 16 * frames.at(akt_frame).at(current_posx).at(current_posy + 1).at(0) - 16 * frames.at(akt_frame).at(current_posx).at(current_posy - 1).at(0) + frames.at(akt_frame).at(current_posx).at(current_posy - 2).at(0)) / (-60);
				if (k == 999)
				{
					cout << temp_u_x << " ";
				}
			}
			if (k == 999)
			{
				cout << "\n";
			}
		}
	}
}

void advect()
{
	for (int i = 0; i < size_x; ++i)
	{
		current_posx = i;
		for (int j = 0; j < size_y; ++j)
		{
			current_posy = j;
			if (is_boundary.at(current_posx).at(current_posy))
			{
				continue;
			}
			past_posx_x = current_posx - frames.at(akt_frame).at(current_posx).at(current_posy).at(1) * (*time_stp); // vorherige Position bei dem Teilchen, welches am Ort der x-Geschwindigkeit ist
			past_posy_x = current_posy + 0.5/*da x- und y-Geschwindigkeit versetzt gespeichert sind*/ - calc_avg(2) * (*time_stp);
			past_posx_y = current_posx + 0.5 - calc_avg(1) * (*time_stp);
			past_posy_y = current_posy - frames.at(akt_frame).at(current_posx).at(current_posy).at(2) * (*time_stp); // hier in y-Richtung
			vector<long double> erg = interpolate(); // interpolierte Geschwindigkeit an der vorherigen Position
			frames.at(akt_frame).at(current_posx).at(current_posy).at(1) = erg.at(0);
			frames.at(akt_frame).at(current_posx).at(current_posy).at(2) = erg.at(1);
		}
	}
}

void ext_force()
{
	for (int i = 0; i < size_x; ++i)
	{
		current_posx = i;
		for (int j = 0; j < size_y; ++j)
		{
			current_posy = j;
			if (is_boundary.at(current_posx).at(current_posy))
			{
				continue;
			}
			frames.at(akt_frame).at(current_posx).at(current_posy).at(2) += (*time_stp) * 9.81; // Einbezug der Gravitation in y-Richtung
		}
	}
}

void calc_vel()
{
	
}

vector<long double> get_nval(int w) // IMMER TEMP AKTUALISIEREN, JE NACHDDEM, OB DRUCK ODER GESCHWINDIGKEIT // zweiter Parameter steht für Value in frames
{
	vector<long double> erg(4);
	nachbarn = 4;
	switch (w)
	{
	case 1: // Geschw. ist gesucht (hier x- und y-Richtung zusammen, um if-else nicht wiederholen zu müssen
		// vorher checken, ob Punkt boundary ist

		// ist möglich, da hier frm = akt_frame ist, siehe temp (Definition)
		if (current_posx - 1 < 0)
		{
			erg.at(0) = 0;
			--nachbarn;
		}
		else if (is_solid.at(current_posx - 1).at(current_posy))
		{
			erg.at(0) = 0;
			--nachbarn;
		}
		else
		{
			erg.at(0) = frames.at(akt_frame).at(current_posx).at(current_posy).at(1);
		}
		if (current_posy + 1 > size_y - 1)
		{
			erg.at(1) = 0;
			--nachbarn;
		}
		else if (is_solid.at(current_posx).at(current_posy + 1))
		{
			erg.at(1) = 0;
			--nachbarn;
		}
		else
		{
			erg.at(1) = frames.at(akt_frame).at(current_posx).at(current_posy + 1).at(2);
		}
		if (current_posx + 1 > size_x - 1)
		{
			erg.at(2) = 0;
			--nachbarn;
		}
		else if (is_solid.at(current_posx + 1).at(current_posy))
		{
			erg.at(2) = 0;
			--nachbarn;
		}
		else
		{
			erg.at(2) = frames.at(akt_frame).at(current_posx + 1).at(current_posy).at(1);
		}
		if ((current_posy - 1) < 0)
		{
			erg.at(3) = 0;
			--nachbarn;
		}
		else if (is_solid.at(current_posx).at(current_posy - 1))
		{
			erg.at(3) = 0;
			--nachbarn;
		}
		else
		{
			erg.at(3) = frames.at(akt_frame).at(current_posx).at(current_posy).at(2);
		}
		/*
		if (current_posx == 0)
		{ // Überprüfung, ob die Simulation an einem Rand des Meshs angelangt ist, wenn ja, einen Nachbarn abziehen, Geschwindigkeit am Rand ist 0
			erg.at(0) = 0;
			--nachbarn;
		}
		else
		{
			erg.at(0) = frames.at(frm).at(current_posx).at(current_posy).at(1);
		}
		if (current_posx == size_x - 1)
		{
			erg.at(2) = 0;
			--nachbarn;
		}
		else
		{
			erg.at(2) = frames.at(frm).at(current_posx + 1).at(current_posy).at(1);
		}
		if (current_posy == size_y - 1)
		{
			erg.at(1) = 0;
			--nachbarn;
		}
		else
		{
			erg.at(1) = frames.at(frm).at(current_posx).at(current_posy + 1).at(2);
		}
		if (current_posy == 0)
		{
			erg.at(3) = 0;
			--nachbarn;
		}
		else
		{
			erg.at(3) = frames.at(frm).at(current_posx).at(current_posy).at(2);
		}
		*/
		break;
	case 0: // VERBESSERUNG: beide Fälle in einem vereinen
		// Wenn pressure gesucht ist, ist nicht der vorherige Frame wichtig, sondern der aktuelle
		erg.at(0) = temp.at(current_posx).at(current_posy).at(1);
		erg.at(1) = temp.at(current_posx).at(current_posy + 1).at(2);
		erg.at(2) = temp.at(current_posx + 1).at(current_posy).at(1);
		erg.at(3) = temp.at(current_posx).at(current_posy).at(2);
		if (is_boundary.at(current_posx - 1).at(current_posy))
		{
			erg.at(0) = 0;
			--nachbarn;
		}
		if (is_boundary.at(current_posx).at(current_posy + 1))
		{
			erg.at(1) = 0;
			--nachbarn;
		}
		if (is_boundary.at(current_posx + 1).at(current_posy))
		{
			erg.at(2) = 0;
			--nachbarn;
		}
		if (is_boundary.at(current_posx).at(current_posy - 1))
		{
			erg.at(3) = 0;
			--nachbarn;
		}
		break;
	}
	return erg;
}

long double calc_avg(int i)
{
	vector<long double> erg(4);
	nachbarn = 4;
	switch (i)
	{
	case 1: // durchschnittlichen x-Geschwindigkeitswerte werden ermittelt
		erg.at(0) = frames.at(akt_frame).at(current_posx).at(current_posy).at(1);
		erg.at(1) = frames.at(akt_frame).at(current_posx + 1).at(current_posy).at(1);
		erg.at(2) = frames.at(akt_frame).at(current_posx + 1).at(current_posy - 1).at(1);
		erg.at(3) = frames.at(akt_frame).at(current_posx).at(current_posy - 1).at(1);
		if (is_solid.at(current_posx).at(current_posy))
		{
			erg.at(0) = 0;
			--nachbarn;
		}
		if (is_solid.at(current_posx + 1).at(current_posy))
		{
			erg.at(1) = 0;
			--nachbarn;
		}
		if (is_solid.at(current_posx + 1).at(current_posy - 1))
		{
			erg.at(2) = 0;
			--nachbarn;
		}
		if (is_solid.at(current_posx).at(current_posy - 1))
		{
			erg.at(3) = 0;
			--nachbarn;
		}
		break;
	case 2: // hier y-Richtung
		erg.at(0) = frames.at(akt_frame).at(current_posx - 1).at(current_posy).at(2);
		erg.at(1) = frames.at(akt_frame).at(current_posx - 1).at(current_posy + 1).at(2);
		erg.at(2) = frames.at(akt_frame).at(current_posx).at(current_posy + 1).at(2);
		erg.at(3) = frames.at(akt_frame).at(current_posx).at(current_posy).at(2);
		if (is_solid.at(current_posx - 1).at(current_posy))
		{
			erg.at(0) = 0;
			--nachbarn;
		}
		if (is_solid.at(current_posx - 1).at(current_posy + 1))
		{
			erg.at(1) = 0;
			--nachbarn;
		}
		if (is_solid.at(current_posx).at(current_posy + 1))
		{
			erg.at(3) = 0;
			--nachbarn;
		}
		if (is_solid.at(current_posx).at(current_posy))
		{
			erg.at(2) = 0;
			--nachbarn;
		}
		break;
	}
	return (erg.at(0) + erg.at(1) + erg.at(2) + erg.at(3)) / nachbarn;
}

vector<long double> interpolate()
{ // sucht die näheste Zelle zum vorherigen Zeitpunkt von dem Partikel, welches sich jetzt an der Stelle current_posx und current_posy befindet

	// TODO: Fehler, da am Rand die x- und y-Positionen negativ sein müssten (Rand definieren, nach CFL-Bedingung kann ein Partikel max eine Zelle pro Zeitschritt vorankommen)
	// für solid boundaries kann code bestehen bleiben, noch zusätzlich inflow / outflow edges
	
	vector<long double> erg(2);
	vector<long double> values(8);
	int untere_grx_x, untere_gry_x, obere_grx_x, obere_gry_x, untere_grx_y, untere_gry_y, obere_grx_y, obere_gry_y;
	untere_grx_x = trunc(past_posx_x);
	untere_gry_x = trunc(past_posy_x - 1) + 0.5; /*-1, damit Abstand der Geschwindigkeit in y-Richtung an der unteren Grenze vom Rand der Zelle berücksichtigt wird (siehe Skizze)*/ /*+0,5, da x- und y-Geschwindigkeit versetzt gespeichert sind*/
	untere_grx_y = trunc(past_posx_y - 1) + 0.5;
	untere_gry_y = trunc(past_posy_y);

	double deriv_x = calc_avg(2) / frames.at(akt_frame).at(current_posx).at(current_posy).at(1);
	double deriv_y = frames.at(akt_frame).at(current_posx).at(current_posy).at(2) / calc_avg(1);

	obere_grx_x = untere_grx_x < 0 ? untere_grx_x - 1 : untere_grx_x + 1; // wenn die Grenze kleienr als 0 ist
	obere_gry_x = untere_gry_x < 0 ? untere_gry_x - 1 : untere_gry_x + 1;
	obere_grx_y = untere_grx_y < 0 ? untere_grx_y - 1 : untere_grx_y + 1;
	obere_gry_y = untere_gry_y < 0 ? untere_gry_y - 1 : untere_gry_y + 1;

	// Wenn etwas von der x-Geschwindigkeit außerhalb der boundary ist
	while (past_posx_x < 0 || is_solid.at(untere_grx_x).at(untere_gry_x) || is_solid.at(untere_grx_x).at(obere_gry_x))
	{
		double temp = (untere_grx_x - past_posx_x);
		past_posx_x += temp + 0.01 * dist; // auf einer Geraden mit dem Richtungsvektor akt_geschwindigkeit_x / akt_geschwindigkeit_y wird so lange der Punkt past_posx_x proportional zu past_posy_x verfolgt, bis die Grenzen innerhalb der Domain sind
		past_posy_x += deriv_x * (temp + 0.01 * dist); // damit Proportionalität beibehalten wird
		untere_grx_x = trunc(past_posx_x);
		obere_grx_x = untere_grx_x <= 0 ? untere_grx_x - 1 : untere_grx_x + 1;
		untere_gry_x = trunc(past_posy_x);
		obere_gry_x = untere_gry_x <= 0 ? untere_gry_x - 1 : untere_gry_x + 1;
	}
	while (past_posx_x > size_x - 1 || is_solid.at(obere_grx_x).at(untere_gry_x) || is_solid.at(obere_grx_x).at(obere_gry_x))
	{
		double temp = (untere_grx_x - past_posx_x);
		past_posx_x += temp - 0.01 * dist; 
		past_posy_x += deriv_x * (temp - 0.01 * dist);
		untere_grx_x = trunc(past_posx_x);
		obere_grx_x = untere_grx_x >= 0 ? untere_grx_x - 1 : untere_grx_x + 1;
		untere_gry_x = trunc(past_posy_x);
		obere_gry_x = untere_gry_x >= 0 ? untere_gry_x - 1 : untere_gry_x + 1;
	}
	while (past_posy_x < 0 || is_solid.at(obere_grx_x).at(untere_gry_x) || is_solid.at(untere_grx_x).at(untere_gry_x))
	{
		double temp = (untere_gry_x - past_posy_x);
		past_posy_x += temp + 0.01 * dist;
		past_posx_x += (1 / deriv_x) * (temp + 0.01 * dist); // hier Kehrwert von deriv_x, damit korrektes dx hier berechnet werden kann (jetzt ist es dx/dy * dy = dx)
		untere_grx_x = trunc(past_posx_x);
		obere_grx_x = untere_grx_x <= 0 ? untere_grx_x - 1 : untere_grx_x + 1;
		untere_gry_x = trunc(past_posy_x);
		obere_gry_x = untere_gry_x <= 0 ? untere_gry_x - 1 : untere_gry_x + 1;
	}
	while (past_posy_x > size_y || is_solid.at(obere_grx_x).at(obere_gry_x) || is_solid.at(untere_grx_x).at(obere_gry_x))
	{
		double temp = (untere_grx_x - past_posx_x);
		past_posx_x += temp - 0.01 * dist;
		past_posy_x += (1 / deriv_x) * (temp - 0.01 * dist);
		untere_grx_x = trunc(past_posx_x);
		obere_grx_x = untere_grx_x >= 0 ? untere_grx_x - 1 : untere_grx_x + 1;
		untere_gry_x = trunc(past_posy_x);
		obere_gry_x = untere_gry_x >= 0 ? untere_gry_x - 1 : untere_gry_x + 1;
	}
	// hier y-Geschwindigkeit:
	while (past_posx_y < 0 || is_solid.at(untere_grx_y).at(untere_gry_y) || is_solid.at(untere_grx_y).at(obere_gry_y))
	{
		double temp = (untere_grx_y - past_posx_y);
		past_posx_y += temp + 0.01 * dist; // auf einer Geraden mit dem Richtungsvektor akt_geschwindigkeit_y / akt_geschwindigkeit_y wird so lange der Punkt past_posx_y proportional zu past_posy_y verfolgt, bis die Grenzen innerhalb der Domain sind
		past_posy_y += deriv_y * (temp + 0.01 * dist); // damit Proportionalität beibehalten wird
		untere_grx_y = trunc(past_posx_y);
		obere_grx_y = untere_grx_y <= 0 ? untere_grx_y - 1 : untere_grx_y + 1;
		untere_gry_y = trunc(past_posy_y);
		obere_gry_y = untere_gry_y <= 0 ? untere_gry_y - 1 : untere_gry_y + 1;
	}
	while (past_posx_y > size_y - 1 || is_solid.at(obere_grx_y).at(untere_gry_y) || is_solid.at(obere_grx_y).at(obere_gry_y))
	{
		double temp = (untere_grx_y - past_posx_y);
		past_posx_y += temp - 0.01 * dist;
		past_posy_y += deriv_y * (temp - 0.01 * dist);
		untere_grx_y = trunc(past_posx_y);
		obere_grx_y = untere_grx_y >= 0 ? untere_grx_y - 1 : untere_grx_y + 1;
		untere_gry_y = trunc(past_posy_y);
		obere_gry_y = untere_gry_y >= 0 ? untere_gry_y - 1 : untere_gry_y + 1;
	}
	while (past_posy_y < 0 || is_solid.at(obere_grx_y).at(untere_gry_y) || is_solid.at(untere_grx_y).at(untere_gry_y))
	{
		double temp = (untere_gry_y - past_posy_y);
		past_posy_y += temp + 0.01 * dist;
		past_posx_y += (1 / deriv_y) * (temp + 0.01 * dist); // hier Kehrwert von deriv_y, damit korrektes dx hier berechnet werden kann (jetzt ist es dx/dy * dy = dx)
		untere_grx_y = trunc(past_posx_y);
		obere_grx_y = untere_grx_y <= 0 ? untere_grx_y - 1 : untere_grx_y + 1;
		untere_gry_y = trunc(past_posy_y);
		obere_gry_y = untere_gry_y <= 0 ? untere_gry_y - 1 : untere_gry_y + 1;
	}
	while (past_posy_y > size_y || is_solid.at(obere_grx_y).at(obere_gry_y) || is_solid.at(untere_grx_y).at(obere_gry_y))
	{
		double temp = (untere_grx_y - past_posx_y);
		past_posx_y += temp - 0.01 * dist;
		past_posy_y += (1 / deriv_y) * (temp - 0.01 * dist);
		untere_grx_y = trunc(past_posx_y);
		obere_grx_y = untere_grx_y >= 0 ? untere_grx_y - 1 : untere_grx_y + 1;
		untere_gry_y = trunc(past_posy_y);
		obere_gry_y = untere_gry_y >= 0 ? untere_gry_y - 1 : untere_gry_y + 1;
	}
	obere_grx_x = untere_grx_x + 1;
	obere_gry_x = untere_gry_x + 1;
	obere_grx_y = untere_grx_y + 1;
	obere_gry_y = untere_gry_y + 1;
	erg.at(0) = (obere_gry_x - past_posy_x) * (past_posx_x - untere_grx_x) * frames.at(akt_frame - 1).at(obere_grx_x).at(untere_gry_x).at(1) + (past_posx_x - untere_grx_x) * (past_posy_x - untere_gry_x) * frames.at(akt_frame - 1).at(obere_grx_x).at(obere_gry_x).at(1) + (obere_grx_x - past_posx_x) * (obere_gry_x - past_posy_x) * frames.at(akt_frame - 1).at(untere_grx_x).at(untere_gry_x).at(1) + (obere_grx_x - past_posx_x) * (past_posy_x - untere_gry_x) * frames.at(akt_frame - 1).at(untere_grx_x).at(obere_gry_x).at(1);
	erg.at(1) = (obere_gry_y - past_posy_y) * (past_posx_y - untere_grx_y) * frames.at(akt_frame - 1).at(obere_grx_y).at(untere_gry_y).at(2) + (past_posx_y - untere_grx_y) * (past_posy_y - untere_gry_y) * frames.at(akt_frame - 1).at(obere_grx_y).at(obere_gry_y).at(2) + (obere_grx_y - past_posx_y) * (obere_gry_y - past_posy_y) * frames.at(akt_frame - 1).at(untere_grx_y).at(untere_gry_y).at(2) + (obere_grx_y - past_posx_y) * (past_posy_y - untere_gry_y) * frames.at(akt_frame - 1).at(untere_grx_y).at(obere_gry_y).at(2); // interpolierte Geschwindigkeit in y-Richtung von y-Geschwindigkeit
	return erg;
}

bool is_outside(int past_x, int past_y, long double u, long double v)
{
	if (is_solid.at(past_x).at(past_y))
	{
		// nichts
	}
	return false;
}

void ausgabe()
{
	std::cout << "Druck zum Zeitpunkt " << akt_frame * (*time_stp) << ":\n";
	for (int x = 0; x < size_x; ++x)
	{
		for (int y = 0; y < size_y; ++y)
		{
			cout << frames.at(akt_frame).at(x).at(y).at(0) << " ";
		}
		cout << "\n";
	}
	std::cout << "Geschwindigkeit in x-Richtung zum Zeitpunkt " << akt_frame * (*time_stp) << ":\n";
	for (int x = 0; x < size_x; ++x)
	{
		for (int y = 0; y < size_y; ++y)
		{
			cout << frames.at(akt_frame).at(x).at(y).at(1) << " ";
		}
		cout << "\n";
	}
	std::cout << "Geschwindigkeit in y-Richtung zum Zeitpunkt " << akt_frame * (*time_stp) << ":\n";
	for (int x = 0; x < size_x; ++x)
	{
		for (int y = 0; y < size_y; ++y)
		{
			cout << frames.at(akt_frame).at(x).at(y).at(2) << " ";
		}
		cout << "\n";
	}
	cout << "Die maximale Geschwindigkeit beträgt " << umax << ".\n";
}

vector<vector<vector<vector<long double>>>>& operator+=(vector<vector<vector<vector<long double>>>>& vec1, vector<vector<long double>>& vec2)
{
	for (int i = 0; i < size_x; ++i)
	{
		for (int j = 0; j < size_y; ++j)
		{
			vec1.at(akt_frame).at(i).at(j).at(0) += vec2.at(i).at(j);
		}
	}
	return vec1;
}

vector<vector<long double>> operator+(const vector<vector<long double>>& summand1, const vector<vector<long double>>& summand2)
{
	if ((summand1.size() != summand2.size()) || (summand1.at(0).size() != summand2.at(0).size()))
	{
		cerr << "Die Vektoren haben nicht die gleiche Größe!";
		terminate();
		return {};
	}
	vector<vector<long double>> erg1 = vector<vector<long double>>(summand1.size(), vector<long double>(summand2.at(0).size()));
	for (int i = 0; i < summand1.size(); ++i)
	{
		for (int j = 0; j < summand1.at(0).size(); ++j)
		{
			erg1.at(i).at(j) = summand1.at(i).at(j) + summand2.at(i).at(j);
		}
	}
	return erg1;
}

vector<vector<long double>> operator*(const long double scalar, const vector<vector<long double>>& vec)
{ // skalare Multiplikation
	vector<vector<long double>> erg_scal_mul = vector<vector<long double>>(vec.size(), vector<long double>(vec.at(0).size()));
	for (int i = 0; i < vec.size(); ++i)
	{
		for (int j = 0; j < vec.at(0).size(); ++j)
		{
			erg_scal_mul.at(i).at(j) = vec.at(i).at(j) * scalar;
		}
	}
	return erg_scal_mul;
}

vector<vector<long double>> operator*(const vector<vector<long double>>& faktor1, vector<vector<long double>>& faktor2)
{
	if (faktor1.at(0).size() != faktor2.size())
	{
		cerr << "Die beiden Matrizen haben nicht die selbe Größe!" << endl;
	}
	vector<vector<long double>> erg = vector<vector<long double>>(faktor1.size(), vector<long double>(faktor2.at(0).size()));
	for (int i = 0; i < faktor1.size(); ++i)
	{
		for (int j = 0; j < faktor2.at(0).size(); ++j)
		{
			erg.at(i).at(j) = faktor1.at(i).at(j) * faktor2.at(i).at(j);
		}
	}
	return erg;
}

// Matrix-Rechnung und CGM-Methode
vector<vector<long double>> Matrix::direction;
vector<vector<long double>> Matrix::erg_scal_mul;
vector<vector<long double>> Matrix::erg;

vector<vector<long double>>& Matrix::operator*(const long double scalar, const vector<vector<long double>>& vec)
{ // skalare Multiplikation
	erg_scal_mul = vector<vector<long double>>(vec.size(), vector<long double>(vec.at(0).size()));
	for (int i = 0; i < vec.size(); ++i)
	{
		for (int j = 0; j < vec.at(0).size(); ++j)
		{
			erg_scal_mul.at(i).at(j) = vec.at(i).at(j) * scalar;
		}
	}
	return erg_scal_mul;
}

template<typename T>
T Matrix::convert(const vector<vector<T>>& vec)
{ // wenn Vektor nur einen Eintrag hat als Skalar zurückgeben
	if (vec.size() == 1 || vec.at(0).size())
	{
		return vec.at(0).at(0);
	}
	cerr << "Fehler bei convert-Methode!" << endl;
	return 0;
}

template<typename T>
vector<vector<T>> Matrix::transpose(const vector<vector<T>>& vec)
{ // zu Matrix/ Vektor transponierten Vektor erstellen
	vector<vector<T>> erg = vector<vector<T>>(vec.at(0).size(), vector<T>(vec.size()));
	for (int i = 0; i < vec.size(); ++i)
	{
		for (int j = 0; j < vec.at(0).size(); ++j)
		{
			erg.at(j).at(i) = vec.at(i).at(j);
		}
	}
	return erg;
}

vector<vector<long double>> Matrix::operator-(const vector<vector<long double>>& minuend, const vector<vector<long double>>& subtrahend)
{
	if ((minuend.size() != subtrahend.size()) || (minuend.at(0).size() != subtrahend.at(0).size()))
	{
		cerr << "Die Vektoren haben nicht die gleiche Größe!";
		return {};
	}
	vector<vector<long double>> erg1 = vector<vector<long double>>(minuend.size(), vector<long double>(minuend.at(0).size()));
	for (int i = 0; i < minuend.size(); ++i)
	{
		for (int j = 0; j < minuend.at(0).size(); ++j)
		{
			erg1.at(i).at(j) = minuend.at(i).at(j) - subtrahend.at(i).at(j);
		}
	}
	return erg1;
}

template<typename T, typename T2>
vector<vector<long double>>& Matrix::operator*(const vector<vector<T>>& mat1, const vector<vector<T2>>& mat2)
{
	erg = vector<vector<long double>>(mat1.size(), vector<long double>(mat2.at(0).size(), 0.0));
	if (mat1.at(0).size() != mat2.size())
	{
		/*
		cout << "jkdflsaöjfioepjfoisjfoesisa: " << mat1.at(0).size() << " " << mat2.size() << endl;
		for (int i = 0;i < mat1.size();++i)
		{
			for (int j = 0;j < mat1.at(0).size();++j)
			{
				cout << mat1.at(i).at(j) << " ";
			}
			cout << endl;
		}
		cout << "ende" << endl;
		*/
		return erg;
	}
	for (int i = 0; i < mat1.size(); ++i)
	{
		for (int j = 0; j < mat2.at(0).size(); ++j)
		{
			for (int k = 0; k < mat2.size(); ++k)
			{
				erg.at(i).at(j) += mat1.at(i).at(k) * mat2.at(k).at(j);
			}
		}
	}
	return erg;
}

vector<vector<long double>> Matrix::cgm(vector<vector<long double>>& koeff, vector<vector<long double>>& b1, vector<vector<long double>>& init)
{
	vector<vector<long double>> rest;
	vector<vector<long double>> next_rest;
	vector<vector<long double>> sol = init;
	long double temp1;
	long double temp2;
	long double temp3;
	long double temp4;
	long double alpha;
	long double beta;
	for (int i = 0; i < sol.size(); ++i) // REMOVE BEFORE FLIGHT
	{
		rest = next_rest;
		if (i == 0)
		{
			direction = b1 - koeff * init;
			rest = direction;
		}
		temp1 = convert(transpose(rest) * rest);
		vector<vector<long double>> temp8 = koeff * direction;
		temp2 = convert(transpose(direction) * temp8); // verketteter Aufruf nicht möglich, da erg globale Variable ist
		alpha = temp1 / temp2;
		sol = sol - (-alpha) * direction; // -, um keinen neuen Operator zu überladen
		next_rest = rest - alpha * (koeff * direction);
		temp3 = convert(transpose(next_rest) * next_rest);
		temp4 = convert(transpose(rest) * rest);
		beta = temp3 / temp4;
		direction = next_rest - (-beta) * direction;
	}
	return sol;
}