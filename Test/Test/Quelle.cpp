#include <vector>
#include <random>
#include <iostream>

int main()
{
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
}