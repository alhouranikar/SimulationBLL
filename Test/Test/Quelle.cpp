#include <vector>
#include <random>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <ctime>

std::vector<long double> test (1000000);
std::vector<long double> test2 (1000000);

void set_vector(size_t x)
{
	test.at(x) = 1.2319821321376;
}

void set_vector()
{
	int y = 0;
	std::cout << test2.size() << "\n";
	while (y < test2.size())
	{
		test2.at(y) = 1.2319821321376;
		++y;
	}
}

int main()
{
	/* //Überprüfung Zeiger
	std::vector<unsigned long long> test(100000);
	std::random_device random;
	std::mt19937 rng(random());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(1, 4000000);
	for (auto& elem : test)
	{
		elem = dist6(rng);
	}
	for (int i = 0; i < test.size(); ++i)
	{
		for (int j = 0; j < test.size()-1; ++j)
		{
			if (test.at(j) > test.at(j + 1))
			{
				auto temp = test.at(j);
				test.at(j) = test.at(j + 1);
				test.at(j + 1) = temp;
			}
		}
	}
	
	std::vector<int> test;
	test.push_back(1);
	test.push_back(2);
	test.push_back(3);
	int* test_ptr = &test.at(1);
	test_ptr = nullptr;
	std::cout << "Wert 1 des Vektors ist: " << test_ptr << "\n";
	test.push_back(4);
	std::cout << "Wert 1 des Vektors ist: " << test_ptr << "\n";
	test_ptr = &test.at(1);
	std::cout << "Wert 1 des Vektors ist: " << *test_ptr << "\n";
	test.insert(test.begin(), 5);
	std::cout << "Wert 1 des Vektors ist: " << *test_ptr << "\n";
	test.clear();
	std::cout << "Wert 1 des Vektors ist: " << *test_ptr << "\n";
	std::unordered_set<std::pair<int, int>, Best> x;
	*/
	/* //Vergleich Laufzeit Erstellung von einer Vector-Container
	std::vector<long double> tes;
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < 1000000; ++i)
	{
		tes.push_back(2.374924);
	}
	tes.insert(74382, 1.43267);
	auto end = std::chrono::steady_clock::now();
	auto duration = end - start;
	std::cout << "Die Laufzeit des Vektors betrug: " << (duration.count())/1e9 << std::endl;
	*/
	auto start = std::chrono::steady_clock::now();
	int y = 0;
	std::cout << test.size() << "\n";
	while (y < test.size())
	{
		set_vector(y);
		++y;
	}
	auto end = std::chrono::steady_clock::now();
	auto temp = ((end - start).count()) / 1e9;
	std::cout << "Die Laufzeit bei der Wiederholung der Funktion betrug: " << temp << "\n";
	start = std::chrono::steady_clock::now();
	set_vector();
	end = std::chrono::steady_clock::now();
	auto temp2 = ((end - start).count()) / 1e9;
	std::cout << "Die Laufzeit bei der Wiederholung in der Funktion: " << ((end - start).count()) / 1e9 << "\n";
	std::cout << "Das ist das " << temp / temp2 << "-fache." << "\n";
	return 0;
}

/*
struct Best
{
	size_t operator()(std::pair<int, int> x)
	{
		return x.first ^ x.second;
	}
};
*/