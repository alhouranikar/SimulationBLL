#include "Header.h"

int main()
{
	auto begin = chrono::steady_clock::now();
	set_initial();
	next_frame();
	next_frame();
	next_frame();
	next_frame();
	next_frame();
	next_frame();
	auto end = chrono::steady_clock::now();
	ausgabe();
	cout << "Die Berechnung dauerte " << (chrono::steady_clock::duration(end - begin)).count() * 10e9 << " Sekunden.";
	//calc_pressure();
	/*
	set_initial();
	ausgabe();
	next_frame();
	ausgabe();
	*/
	
	/*
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	next_frame();
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	next_frame();
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	next_frame();
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	next_frame();
	*/
	return 0;
}