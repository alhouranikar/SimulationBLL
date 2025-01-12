#include "Header.h"

int main()
{
	set_initial();
	ausgabe();
	for (int o = 0; o < 20; ++o)
	{
		next_frame();
	}
	ausgabe();
	
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