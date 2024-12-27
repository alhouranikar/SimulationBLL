#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>

int main()
{
	std::cout << "Hello World";
	std::map<int, int, std::vector<long double>> test;
	test.insert < 1, 1, { 1,2,3 };
	return EXIT_SUCCESS;
}