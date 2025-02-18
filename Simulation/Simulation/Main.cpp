#include "Header.h"

int main()
{
	set_initial();
	ausgabe();
	next_frame();
	advect();
	next_frame();
	advect();
	next_frame();
	advect();
	ext_force();
	//calc_pressure();
	ausgabe();
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