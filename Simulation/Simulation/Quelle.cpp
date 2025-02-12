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
			frames.at(0).at(i).at(j).at(1) = 0;
			frames.at(0).at(i).at(j).at(2) = 1;
			if (j == 0)
			{
				frames.at(0).at(i).at(j).at(2) = 1;
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

	density = 1.0; // NUR FÜR TESTZWECKE
}

void create_mesh()
{
	frames = vector<vector<vector<vector<long double>>>>(frame_anz + 1, vector<vector<vector<long double>>>(size_x, vector<vector<long double>>(size_y, vector<long double>(3)))); // 3 Argumente beim long double vector, um Druck und Geschwindigkeit (in x- und y-Richtung) zu speichern (1. Eintrag Druck, 2. Eintrag Geschwindigkeit in x-Richtung, 3. Eintrag Geschwindigkeit in y-Richtung), x Zeilen und y Spalten // VERBESSERUNG: mehrere Werte speichern, um mehr Parameter zu haben; +1, damit der urpsrüngliche Frame auch gespeichert werden kann
}

void next_frame()
{
	++akt_frame;
	frames.at(akt_frame) = frames.at(akt_frame - 1); // Mesh zum aktuellen Zeitpunkt = vorheriger Zeitpunkt
	current_posx = current_posy = 0;
	//calc_pressure();
	current_posx = current_posy = 0;
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
	for (int x = 0; x < size_x; ++x)
	{
		current_posx = x;
		for (int y = 0; y < size_y; ++y)
		{
			current_posy = y;
			if (is_solid.at(current_posx).at(current_posy))
			{ // Geschwindigkeit an solid edge ist immer 0
				continue;
			}
			frames.at(akt_frame).at(current_posx).at(current_posy).at(2) += 9.81 * (*time_stp); // Erhöhung d. Geschwindigkeit durch Gravitation
		}
	}	
	for (int n=0;n<1001;++n)
	{
		for (int x = 0; x < size_x; ++x)
		{
			current_posx = x;
			for (int y = 0; y < size_y - 1; ++y)
			{
				current_posy = y;
				if (is_boundary.at(current_posx).at(current_posy))
				{
					continue;
				}
				temp_nval = get_nval(1);
				long double div = temp_nval.at(2) + temp_nval.at(1) - temp_nval.at(0) - temp_nval.at(3); // Berechnung der Divergenz
				div *= 1.99; // overrelaxation
				if (!is_boundary.at(current_posx).at(current_posy))
				{ // solid und inflow-boundaries sind unveränderlich
					frames.at(akt_frame).at(current_posx).at(current_posy).at(1) += div / nachbarn;
					frames.at(akt_frame).at(current_posx).at(current_posy).at(2) += div / nachbarn;
				}
				if (!is_boundary.at(current_posx + 1).at(current_posy))
				{
					frames.at(akt_frame).at(current_posx + 1).at(current_posy).at(1) -= div / nachbarn;
				}
				if (!is_boundary.at(current_posx).at(current_posy + 1))
				{
					frames.at(akt_frame).at(current_posx).at(current_posy + 1).at(2) -= div / nachbarn;
				}
			}
		}
	}
	ausgabe();
	for (int x = 0; x < size_x; ++x)
	{ 
		current_posx = x;
		for (int y = 0; y < size_y; ++y)
		{
			current_posy = y;
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
		cout << endl;
	}

	// TODO: jetzigen div = 0 -solver durch \nabla \cdot p \cdot \Delta t / \rho ersetzen

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
	untere_grx_x = floor(past_posx_x);
	untere_gry_x = floor(past_posy_x - 1) + 0.5; /*-1, damit Abstand der Geschwindigkeit in y-Richtung an der unteren Grenze vom Rand der Zelle berücksichtigt wird (siehe Skizze)*/ /*+0,5, da x- und y-Geschwindigkeit versetzt gespeichert sind*/
	untere_grx_y = floor(past_posx_y - 1) + 0.5; // HIER NOCH GUCKEN
	untere_gry_y = floor(past_posy_y);

	// Wenn etwas von der x-Geschwindigkeit außerhalb der boundary ist
	if (untere_grx_x < 0)
	{
		untere_grx_x = 0;
	}
	else if (untere_grx_x + 1 > size_x - 1)
	{
		untere_grx_x = size_x - 2;
	}
	if (untere_gry_x < 0)
	{
		untere_gry_x = 0.5;
	}
	else if (untere_gry_x + 1 > size_y - 1)
	{
		untere_gry_x = size_y - 1.5;
	}
	// hier y-Geschwindigkeit:
	if (untere_grx_y < 0)
	{ // HIER NOCH GUCKEN
		untere_grx_y = 0;
	}
	else if (untere_grx_y + 1 > size_x - 1)
	{
		untere_grx_y = size_x - 2;
	}
	if (untere_gry_y < 0)
	{
		untere_gry_y = 0;
	}
	else if (untere_gry_y + 1 > size_y - 1)
	{
		untere_gry_y = size_y - 2;
	}
	obere_grx_x = untere_grx_x + 1;
	obere_gry_x = untere_gry_x + 1;
	obere_grx_y = untere_grx_y + 1;
	obere_gry_y = untere_gry_y + 1;
	if (is_solid.at(obere_grx_x).at(untere_gry_x))
	{
		
	}
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
	vector<vector<long double>> erg1 = vector<vector<long double>>(minuend.size(), vector<long double>(minuend.at(0).size()));
	if ((minuend.size() != subtrahend.size()) || (minuend.at(0).size() != subtrahend.at(0).size()))
	{
		cerr << "Die Vektoren haben nicht die gleiche Größe!";
		return {};
	}
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