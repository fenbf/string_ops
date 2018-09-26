#include <iostream>
#include <charconv>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>
#include "../searchers/simpleperf.h"

std::vector<std::string> GenRandVecOfNumbers(size_t count, uint32_t minNumLen, uint32_t maxNumLen)
{
	if (minNumLen > maxNumLen)
		return {};

	std::vector<std::string> out;
	out.reserve(count);

	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_int_distribution<std::mt19937::result_type> distLen(minNumLen, maxNumLen);
	std::uniform_int_distribution<std::mt19937::result_type> distDigit('0', '9');
	std::uniform_int_distribution<std::mt19937::result_type> distFirstDigit('1', '9');

	for (size_t i = 0; i < count; ++i)
	{
		auto len = distLen(rng);
		std::string num(len, ' ');
		auto iter = std::begin(num);
		*iter = static_cast<char>(distFirstDigit(rng));
		++iter;
		std::transform(iter, std::end(num), iter,
			[&rng, &distDigit](unsigned char c) { return static_cast<char>(distDigit(rng)); });
		out.emplace_back(num);
	}

	return out;
}

std::vector<int> GenRandVecOfNumbers(size_t count)
{
	std::vector<int> out;
	out.reserve(count);

	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_int_distribution<std::mt19937::result_type> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	for (size_t i = 0; i < count; ++i)
		out.push_back(dist(rng));

	return out;
}

void Benchmark(size_t ITERS, size_t vecSize, uint32_t minNumLen, uint32_t maxNumLen)
{
	const auto numStrVec = GenRandVecOfNumbers(vecSize, minNumLen, maxNumLen);
	std::vector<int> numVec(numStrVec.size());

	RunAndMeasure("from_chars", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				std::from_chars(numStrVec[i].data(), numStrVec[i].data() + numStrVec[i].size(), numVec[i]);
			}
		}
		return numVec.size();
	});

	RunAndMeasure("stoi", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				numVec[i] = std::stoi(numStrVec[i]);
			}
		}
		return numVec.size();
	});

	RunAndMeasure("atoi", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				numVec[i] = atoi(numStrVec[i].c_str());
			}
		}
		return numVec.size();
	});

	RunAndMeasure("stringstream", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				std::istringstream ss(numStrVec[i]);
				ss >> numVec[i];
			}
		}
		return numVec.size();
	});

	// the other way around:
	std::vector<std::string> vecNumBack(numStrVec.size());
	std::string strTemp(9, ' ');

	RunAndMeasure("to_chars", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				const auto res = std::to_chars(strTemp.data(), strTemp.data() + strTemp.size(), numVec[i]);
				vecNumBack[i] = std::string_view(strTemp.data(), res.ptr - strTemp.data());
			}
		}
		return numVec.size();
	});

	RunAndMeasure("to_string", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				vecNumBack[i] = std::to_string(numVec[i]);
			}
		}
		return numVec.size();
	});

	RunAndMeasure("sprintf", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				auto res = snprintf(strTemp.data(), 10, "%d", numVec[i]);
				vecNumBack[i] = std::string_view(strTemp.data(), (strTemp.data()+res)- strTemp.data());
			}
		}
		return numVec.size();
	});

	for (size_t i = 0; i < numStrVec.size(); ++i)
	{
		if (numStrVec[i] != vecNumBack[i])
			std::cout << "error! " << i << " " << numStrVec[i] << " !=  " << vecNumBack[i] << '\n';
	}
}

void CheckVectors(const std::vector<int>& a, const std::vector<int>& b)
{
	if (a.size() != b.size())
	{
		std::cout << "wrong size!\n";
		return;
	}

	for (size_t i = 0; i < a.size(); ++i)
	{
		if (a[i] != b[i])
			std::cout << "error! " << i << " " << a[i] << " !=  " << b[i] << '\n';
	}
}

void Benchmark2(size_t ITERS, size_t vecSize)
{
	const auto numIntVec = GenRandVecOfNumbers(vecSize);
	std::vector<std::string> numStrVec(numIntVec.size());
	std::vector<int> numBackIntVec(numIntVec.size());

	std::string strTemp(15, ' ');

	//
	// from_chars/to_chars
	//

	RunAndMeasure("to_chars", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numIntVec.size(); ++i)
			{
				const auto res = std::to_chars(strTemp.data(), strTemp.data() + strTemp.size(), numIntVec[i]);
				numStrVec[i] = std::string_view(strTemp.data(), res.ptr - strTemp.data());
			}
		}
		return numStrVec.size();
	});

	RunAndMeasure("from_chars", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				std::from_chars(numStrVec[i].data(), numStrVec[i].data() + numStrVec[i].size(), numBackIntVec[i]);
			}
		}
		return numBackIntVec.size();
	});

	CheckVectors(numIntVec, numBackIntVec);

	//
	// to_string / stoi
	//

	RunAndMeasure("to_string", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				numStrVec[i] = std::to_string(numIntVec[i]);
			}
		}
		return numStrVec.size();
	});

	RunAndMeasure("stoi", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				numBackIntVec[i] = std::stoi(numStrVec[i]);
			}
		}
		return numBackIntVec.size();
	});

	CheckVectors(numIntVec, numBackIntVec);

	//
	// sprintf / atoi
	//

	RunAndMeasure("sprintf", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numIntVec.size(); ++i)
			{
				auto res = snprintf(strTemp.data(), 10, "%d", numIntVec[i]);
				numStrVec[i] = std::string_view(strTemp.data(), (strTemp.data() + res) - strTemp.data());
			}
		}
		return numStrVec.size();
	});

	RunAndMeasure("atoi", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				numBackIntVec[i] = atoi(numStrVec[i].c_str());
			}
		}
		return numBackIntVec.size();
	});

	// ostringstream / istringstream

	RunAndMeasure("otringstream", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				std::ostringstream ss;
				ss << numIntVec[i];
				numStrVec[i] = ss.str();
			}
		}
		return numStrVec.size();
	});

	RunAndMeasure("stringstream", [&]() {
		for (size_t iter = 0; iter < ITERS; ++iter)
		{
			for (size_t i = 0; i < numStrVec.size(); ++i)
			{
				std::istringstream ss(numStrVec[i]);
				ss >> numBackIntVec[i];
			}
		}
		return numBackIntVec.size();
	});
}

int main(int argc, const char** argv)
{
#ifdef _DEBUG
	const size_t DEFAULT_ITER = 1;
	const size_t DEFAULT_VECSIZE = 20;
#else
	const size_t DEFAULT_ITER = 1000;
	const size_t DEFAULT_VECSIZE = 10000;
#endif

	const size_t ITERS = argc > 1 ? atoi(argv[1]) : DEFAULT_ITER;
	std::cout << "test iterations: " << ITERS << '\n';
	const size_t VECSIZE = argc > 2 ? atoi(argv[2]) : DEFAULT_VECSIZE;
	std::cout << "vector size: " << VECSIZE << '\n';
	/*const uint32_t MINLEN = argc > 3 ? atoi(argv[3]) : 1;
	std::cout << "min len: " << MINLEN << '\n';	
	const uint32_t MAXLEN = argc > 4 ? atoi(argv[4]) : 9;
	std::cout << "max len: "  << MAXLEN << '\n';

	Benchmark(ITERS, VECSIZE, MINLEN == 0 ? 1 : MINLEN, MAXLEN > 9 ? MAXLEN : 9);*/

	Benchmark2(ITERS, VECSIZE);

	//for (;;)
	//{
	//	std::string str;
	//	std::cin >> str;
	//	if (str == "x" || str == "end" || str == "exit")
	//		break;

	//	double value;
	//	const auto res = std::from_chars(str.data(), str.data() + str.size(), value, std::chars_format::hex);

	//	if (res.ec == std::errc::invalid_argument)
	//	{
	//		std::cout << "invalid argument!, res.p distance: " << res.ptr - str.data() << '\n';
	//	}
	//	else if (res.ec == std::errc::result_out_of_range)
	//	{
	//		std::cout << "out of range! res.p distance: " << res.ptr - str.data() << '\n';
	//	}
	//	else
	//	{
	//		std::cout << "value: " << value << ", distance: " << res.ptr - str.data() << '\n';

	//		std::string strBack = str;
	//		strBack.clear();

	//		if (auto[ptr, ec] = std::to_chars(strBack.data(), strBack.data() + strBack.size(), static_cast<int>(value)); ec == std::errc())
	//		{
	//			std::cout << "converted back to integer: " << std::string_view(strBack.data(), ptr - strBack.data()) << ", " << strBack << '\n'; // not NUL-terminated!
	//		}
	//	}
	//}
}
