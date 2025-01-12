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
unsigned int akt_frame; // aktueller Frame
size_t size_x;
size_t size_y;
unsigned int current_posx;
unsigned int current_posy;
double past_posx;
double past_posy;
long double umax;
unsigned int nachbarn;
vector<vector<vector<long double>>> temp; // VERBESSERUNG: besser kopieren, Erstellung einer Kopie des Vektorfelds zum Zeitpunkt glb_time
vector<long double> temp_nval;
double dist; 

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
			frames.at(0).at(i).at(j).at(1) = 1;
			frames.at(0).at(i).at(j).at(2) = 1;
		}
		check_umax(frames.at(akt_frame).at(current_posx).at(current_posy)); // Wenn die Geschwindigkeit größer als umax ist, dann neue maximale Geschwindigkeit setzen
	}
	is_inflow = vector<vector<bool>>(size_x, vector<bool>(size_y));
	is_outflow = vector<vector<bool>>(size_x, vector<bool>(size_y));
	is_solid = vector<vector<bool>>(size_x, vector<bool>(size_y));
	is_boundary = vector<vector<bool>>(size_x, vector<bool>(size_y));
	for (int i = 0; i < size_x; ++i)
	{ // size_x und size_y müssen gleich sein
		is_inflow.at(i).at(0) = true;
		is_boundary.at(i).at(0) = true;
		is_inflow.at(0).at(i) = true;
		is_boundary.at(0).at(i) = true;
		is_inflow.at(i).at(size_y - 1) = true;
		is_boundary.at(i).at(size_y - 1) = true;
		is_inflow.at(size_x - 1).at(i) = true;
		is_boundary.at(size_x - 1).at(i) = true;
	}
	current_posx = 0;
	current_posy = 0;
}

void create_mesh()
{
	frames = vector<vector<vector<vector<long double>>>>(frame_anz + 1, vector<vector<vector<long double>>>(size_x, vector<vector<long double>>(size_y, vector<long double>(3)))); // 3 Argumente beim long double vector, um Druck und Geschwindigkeit (in x- und y-Richtung) zu speichern (1. Eintrag Druck, 2. Eintrag Geschwindigkeit in x-Richtung, 3. Eintrag Geschwindigkeit in y-Richtung), x Zeilen und y Spalten // VERBESSERUNG: mehrere Werte speichern, um mehr Parameter zu haben; +1, damit der urpsrüngliche Frame auch gespeichert werden kann
}

void next_frame()
{
	++akt_frame;
	temp = frames.at(akt_frame);
	current_posx = current_posy = 0;
	calc_pressure();
	current_posx = current_posy = 0;
	calc_vel();
	// VERBESSERUNG: calc_pressure und calc_vel in eine Schleife zusammenfassen
	calc_dt(umax, dist); //	VERBESSERUNG: überprüfen, ob dt überhaupt geändert werden muss
	glb_time += *time_stp;
	if (((*ende) - glb_time) / (*time_stp) > frame_anz - akt_frame)
	{ // wenn nach neuem Zeitschritt mehr Frames benötigt werden // VERBESSERUNG: (dringend) überprüfen, ob resize sich lohnt, wenn der neue Zeitschritt größer ist 
		frame_anz = static_cast<int>(((*ende) - (*start)) / (*time_stp));
		frames.resize(frame_anz);
		ende = make_unique<const double>(frame_anz * (*time_stp));
	}
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
	if (speed.at(2) > umax)
	{
		umax = speed.at(2);
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
	/*
	for (int x = 0; x < size_x; ++x)
	{
		current_posx = x;
		for (int y = 0; y < size_y; ++y)
		{
			current_posy = y;
			frames.at(akt_frame).at(current_posx).at(current_posy).at(0) = frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(0);
			vector<long double> nval = get_nval(0); // Fehler, akt_frame - 1 muss als temp gesetzt werden
			temp.at(current_posx).at(current_posy) = frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(0) * nachbarn; // Pressure-solver nach https://cg.informatik.uni-freiburg.de/intern/seminar/gridFluids_fluid-EulerParticle.pdf, Frame vor dem aktuellen als Ursprungszustand
			for (int h = 0; h < nval.size(); ++h)
			{ // Schleife über alle Nachbarn der Zelle, um den Druckgradienten der aktuellen Zelle zu berechnen
				temp.at(current_posx).at(current_posy) -= nval.at(h);
			}
			// Schleife über alle Punkte im Mesh, um Laufzeit zu sparen (ca. 1.5x schneller), nicht implementieren, da zwei Schleifen also auch für die Geschwindigkeit durchlaufen werden müssten
		}
	}
	frames += temp; // berechnete Änderung der Werte aus temp als Divergenz zu den Werten in values hinzuaddieren, nur für pressure
	*/
	// für Pressure-solver noch boundary-check einführen
}

void calc_vel()
{
	for (int x = 0; x < size_x; ++x)
	{
		current_posx = x;
		for (int y = 0; y < size_y; ++y)
		{
			current_posy = y;
			if (is_boundary.at(current_posx).at(current_posy))
			{
				temp.at(current_posx).at(current_posy).at(1) = frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(1);
				temp.at(current_posx).at(current_posy).at(2) = frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(2); // VERBESSERUNG: zeitlich veränderbare Anfgangsbedingungen, dann muss hier jeweils die neue Anfangsbedingungen zum Frame abgerufen werden
				continue;
			}
			temp.at(current_posx).at(current_posy).at(1) = frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(1);
			temp.at(current_posx).at(current_posy).at(2) = frames.at(akt_frame - 1).at(current_posx).at(current_posy).at(2);
			temp.at(current_posx).at(current_posy).at(2) += 9.81 * (*time_stp); // Änderung der Geschwindigkeit aufgrund der Gravitation
			check_umax(frames.at(akt_frame).at(current_posx).at(current_posy));
		}
	}
	for (int n = 0; n < 501; ++n)
	{ // VERBESSSERUNG: statt Gauß-Seidel conjugate gradient verwenden
		for (int x = 0; x < size_x; ++x)
		{
			current_posx = x;
			for (int y = 0; y < size_y; ++y)
			{ // Änderung der Geschwindigkeit, damit Divergenz für jede Zelle null ist
				current_posy = y;
				if (is_boundary.at(current_posx).at(current_posy))
				{
					continue;
				}
				temp_nval = get_nval(1); // Alle benachbarten Werte der Geschw. speichern
				double div = temp_nval.at(2) + temp_nval.at(1) - temp_nval.at(0) - temp_nval.at(3);
				div *= 1.99;
				// wenn der Eintrag eine Wand ist, ändert sich die Geschwindigkeit nicht
				temp.at(current_posx).at(current_posy).at(1) += div / nachbarn; // + Divergenz, damit die Divergenz null ist
				temp.at(current_posx).at(current_posy + 1).at(2) -= div / nachbarn;
				temp.at(current_posx + 1).at(current_posy).at(1) -= div / nachbarn;
				temp.at(current_posx).at(current_posy).at(2) += div / nachbarn;
				check_umax(frames.at(akt_frame).at(current_posx).at(current_posy));
			}
		}
	}

	// TODO: check_umax und calc_dt so implementieren, dass Simulation nicht außerhalb des Rands sein kann
	
	for (int x = 0; x < size_x; ++x)
	{ // Berechnung der Advektion
		current_posx = x;
		for (int y = 0; y < size_y; ++y)
		{
			current_posy = y;
			if (is_boundary.at(current_posx).at(current_posy))
			{
				continue;
			}
			past_posx = current_posx - temp.at(current_posx).at(current_posy).at(1) * (*time_stp) - calc_avg(2) * (*time_stp); // Berechnung der vorherigen Position
			past_posy = current_posy - temp.at(current_posx).at(current_posy).at(2) * (*time_stp) - calc_avg(1) * (*time_stp);
			vector<long double> geschw = interpolate();
			temp.at(current_posx).at(current_posy).at(1) = geschw.at(0);
			temp.at(current_posx).at(current_posy).at(2) = geschw.at(1);
			check_umax(temp.at(current_posx).at(current_posy));
			// nach cg.informatik, inakkurat // VERBESSERUNG: Runge-Kutta oder besser upwind
		}
	}
	frames.at(akt_frame) = temp;
}

vector<long double> get_nval(int w) // IMMER TEMP AKTUALISIEREN, JE NACHDDEM, OB DRUCK ODER GESCHWINDIGKEIT // zweiter Parameter steht für Value in frames
{
	vector<long double> erg(4);
	nachbarn = 4;
	switch (w)
	{
	case 1: // Geschw. ist gesucht (hier x- und y-Richtung zusammen, um if-else nicht wiederholen zu müssen

		// TODO: Umgang mit solid boundaaries
		// vorher checken, ob Punkt boundary ist

		// ist möglich, da hier frm = akt_frame ist, siehe temp (Definition)
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
		erg.at(0) = temp.at(current_posx).at(current_posy).at(1);
		erg.at(1) = temp.at(current_posx + 1).at(current_posy).at(1);
		erg.at(2) = temp.at(current_posx).at(current_posy - 1).at(1);
		erg.at(3) = temp.at(current_posx + 1).at(current_posy - 1).at(1);
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
	case 2: // hier y-Richtung
		for (int o = 0; o < size_x; ++o)
		{
			for (int k = 0; k < size_y; ++k)
			{
				cout << temp.at(o).at(k).at(2) << " ";
			}
			cout << endl;
		}
		erg.at(0) = temp.at(current_posx - 1).at(current_posy).at(2);
		erg.at(1) = temp.at(current_posx - 1).at(current_posy + 1).at(2);
		erg.at(2) = temp.at(current_posx).at(current_posy).at(2);
		erg.at(3) = temp.at(current_posx).at(current_posy + 1).at(2);
		if (is_boundary.at(current_posx - 1).at(current_posy))
		{
			erg.at(0) = 0;
			--nachbarn;
		}
		if (is_boundary.at(current_posx - 1).at(current_posy + 1))
		{
			erg.at(1) = 0;
			--nachbarn;
		}
		if (is_boundary.at(current_posx).at(current_posy))
		{
			erg.at(2) = 0;
			--nachbarn;
		}
		if (is_boundary.at(current_posx).at(current_posy + 1))
		{
			erg.at(3) = 0;
			--nachbarn;
		}
		break;
	}
	cout << (erg.at(0) + erg.at(1) + erg.at(2) + erg.at(3)) / nachbarn << endl;
	return (erg.at(0) + erg.at(1) + erg.at(2) + erg.at(3)) / nachbarn;
}

vector<long double> interpolate()
{ // sucht die näheste Zelle zum vorherigen Zeitpunkt von dem Partikel, welches sich jetzt an der Stelle current_posx und current_posy befindet

	// TODO: Fehler, da am Rand die x- und y-Positionen negativ sein müssten (Rand definieren, nach CFL-Bedingung kann ein Partikel max eine Zelle pro Zeitschritt vorankommen)
	// für solid boundaries kann code bestehen bleiben, noch zusätzlich inflow / outflow edges
	
	vector<long double> erg(2);
	size_t untere_grx;
	size_t obere_grx;
	size_t untere_gry;
	size_t obere_gry;
	untere_grx = floor(past_posx); // Änderung aus Informatik, bitte testen
	obere_grx = untere_grx + 1;
	untere_gry = floor(past_posy); // Änderung aus Informatik, bitte testen
	obere_gry = untere_gry + 1;
	cout << untere_grx << endl;
	erg.at(0) = (1 - (past_posx - untere_grx) / dist) * frames.at(akt_frame).at(untere_grx).at(untere_gry).at(1) + ((past_posx - untere_grx) / dist) * frames.at(akt_frame).at(obere_grx).at(untere_gry).at(1); // interpolierte Geschwindigkeit in x-Richtung
	erg.at(1) = (1 - (past_posy - untere_gry) / dist) * frames.at(akt_frame).at(untere_grx).at(untere_gry).at(2) + ((past_posy - untere_gry) / dist) * frames.at(akt_frame).at(untere_grx).at(obere_gry).at(2); // interpolierte Geschwindigkeit in y-Richtung
	return erg;
}

void ausgabe()
{
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