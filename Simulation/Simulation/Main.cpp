#include "Header.h"

int main()
{
	vector<vector<long double>> coefficient = vector<vector<long double>>(10, vector<long double>(10));
	vector<vector<long double>> b = vector<vector<long double>>(10, vector<long double>(1));
	vector<vector<long double>> initial = vector<vector<long double>>(10, vector<long double>(1));
	double A[10][10] = {
		{4, 1, 0, 0, 1, 0, 0, 0, 0, 0},
		{1, 4, 1, 0, 0, 1, 0, 0, 0, 0},
		{0, 1, 4, 1, 0, 0, 1, 0, 0, 0},
		{0, 0, 1, 4, 1, 0, 0, 1, 0, 0},
		{1, 0, 0, 1, 4, 1, 0, 0, 1, 0},
		{0, 1, 0, 0, 1, 4, 1, 0, 0, 1},
		{0, 0, 1, 0, 0, 1, 4, 1, 0, 0},
		{0, 0, 0, 1, 0, 0, 1, 4, 1, 0},
		{0, 0, 0, 0, 1, 0, 0, 1, 4, 1},
		{0, 0, 0, 0, 0, 1, 0, 0, 1, 4}
	};
	for (int i = 0;i < 10;++i)
	{
		for (int j = 0;j < 10;++j)
		{
			coefficient.at(i).at(j) = A[i][j];
		}
		b.at(i).at(0) = i + 1;
		initial.at(i).at(0) = 1;
	}
	vector<vector<long double>> temp = cgm(coefficient, b, initial);
	for (auto elem : temp)
	{
		for (auto elem2 : elem)
		{
			cout << elem2 << " ";
		}
		cout << endl;
	}
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