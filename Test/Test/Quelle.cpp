#include <vector>
#include <random>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <ctime>

class Zahl
{
public:
	Zahl(int Zehner, int Einer, double dez) : Zehn{ Zehner }, Eins{ Einer }, dez{ dez };
	Zahl(double dez) : Zahl(0, 0, dez);
private:
	int Zehn, Eins;
	double dez;
};

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
	Zahl nummer1{ 10,1,0.11 };
	ausg(3.3);
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