#include "Header.h"

int main()
{
	set_initial();
	next_frame();
	ausgabe();
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