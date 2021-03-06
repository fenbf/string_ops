#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <chrono>
#include <fstream>
#include <string_view>
#include <sstream>
#include <numeric>
#include "simpleperf.h"

using namespace std::literals;

const std::string_view LoremIpsumStrv{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
"sed do eiusmod tempor incididuntsuperlongwordsuper ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
"quis nostrud exercitation ullamco laboris nisi ut aliquipsuperlongword ex ea commodo consequat. Duis aute "
"irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
"Excepteur sint occaecat cupidatatsuperlongword non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." };

std::string GetNeedleString(int argc, const char** argv, std::string &testString)
{
	const auto testStringLen = testString.length();
	if (argc > 3)
	{
		const size_t tempLen = atoi(argv[3]);
		if (tempLen == 0) // some word?
		{
			std::cout << "needle is a string...\n";
			return argv[3];
		}
		else
		{
			const size_t PATTERN_LEN = tempLen > testStringLen ? testStringLen : tempLen;
			const int pos = argc > 4 ? atoi(argv[4]) : 0;

			if (pos == 0)
			{
				std::cout << "needle from the start...\n";
				return testString.substr(0, PATTERN_LEN);
				
			}
			else if (pos == 1)
			{
				std::cout << "needle from the center...\n";
				return testString.substr(testStringLen / 2 - PATTERN_LEN / 2 - 1, PATTERN_LEN);
			}
			else
			{
				std::cout << "needle from the end\n";				
				return testString.substr(testStringLen - PATTERN_LEN - 1, PATTERN_LEN);
			}
		}
	}
	
	// just take the 1/4 of the input string from the end...
	return testString.substr(testStringLen - testStringLen/4 - 1, testStringLen/4);
}

// https://stackoverflow.com/questions/39242932/how-to-encode-char-in-2-bits
// A 65 01000 | 00 | 1  0
// C 67 01000 | 01 | 1  1
// G 71 01000 | 11 | 1  3
// T 84 01010 | 10 | 0  2
enum class Nucleotide : uint8_t {
	A = 0,
	C = 1,
	G = 3,
	T = 2
};

char ToChar(Nucleotide t)
{
	switch (t)
	{
	case Nucleotide::A: return 'A';
	case Nucleotide::C: return 'C';
	case Nucleotide::G: return 'G';
	case Nucleotide::T: return 'T';
	}
	return 0;
}

Nucleotide FromChar(char ch)
{
	return static_cast<Nucleotide>((ch >> 1) & 0x03);
}

std::vector<Nucleotide> FromString(const std::string& s)
{
	std::vector<Nucleotide> out;
	out.reserve(s.length());
	for (auto& ch : s)
		out.push_back(FromChar(ch));

	return out;
}

std::string ToString(const std::vector<Nucleotide>& vec)
{
	std::string s;
	for (auto& n : vec)
		s += ToChar(n);

	return s;
}

void DNADemo()
{
	std::vector<Nucleotide> dna = FromString("CTGATGTTAAGTCAACGCTGC");
	std::cout << ToString(dna) << '\n';
	std::vector<Nucleotide> s = FromString("GCTGC");
	std::cout << ToString(s) << '\n';

	auto it = std::search(dna.begin(), dna.end(),
		std::boyer_moore_horspool_searcher(
			s.begin(), s.end()));
	if (it == dna.end())
		std::cout << "The pattern " << ToString(s) << " not found\n";
}

int main(int argc, const char** argv)
{
	//DNADemo();

	std::string testString{ LoremIpsumStrv };

	if (argc == 1)
	{
		std::cout << "searchers.exe filename iterations pattern_len pos\n";
		return 0;
	}

	if (argc > 1 && "nofile"s != argv[1])
	{
		std::ifstream inFile(argv[1]);

		std::stringstream strStream;
		strStream << inFile.rdbuf();
		testString = strStream.str();
	}

	std::cout << "string length: " << testString.length() << '\n';

	const size_t ITERS = argc > 2 ? atoi(argv[2]) : 1000;
	std::cout << "test iterations: " << ITERS << '\n';

	const auto needle = GetNeedleString(argc, argv, testString);
	std::cout << "pattern length: " << needle.length() << '\n';

	RunAndMeasure("string::find", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			std::size_t found = testString.find(needle);
			if (found == std::string::npos)
				std::cout << "The string " << needle << " not found\n";
		}
		return 0;
	});

	RunAndMeasure("default searcher", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testString.begin(), testString.end(),
				std::default_searcher(
					needle.begin(), needle.end()));
			if (it == testString.end())
				std::cout << "The string " << needle << " not found\n";
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_searcher init only", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			std::boyer_moore_searcher b(needle.begin(), needle.end());
			DoNotOptimizeAway(&b);
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_searcher", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testString.begin(), testString.end(),
				std::boyer_moore_searcher(
					needle.begin(), needle.end()));
			if (it == testString.end())
				std::cout << "The string " << needle << " not found\n";
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_horspool_searcher init only", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			std::boyer_moore_horspool_searcher b(needle.begin(), needle.end());
			DoNotOptimizeAway(&b);
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_horspool_searcher", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testString.begin(), testString.end(),
				std::boyer_moore_horspool_searcher(
					needle.begin(), needle.end()));
			if (it == testString.end())
				std::cout << "The string " << needle << " not found\n";
		}
		return 0;
	});

	std::vector<int> testVector(1000000);
	std::iota(testVector.begin(), testVector.end(), 0);
	std::vector vecNeedle(testVector.end() - 1000, testVector.end());

	RunAndMeasure("vector of ints default", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testVector.begin(), testVector.end(),
				std::default_searcher(
					vecNeedle.begin(), vecNeedle.end()));
			if (it == testVector.end())
				std::cout << "The pattern " << needle << " not found\n";
		}
		return 0;
	});

	RunAndMeasure("boyer_moore_horspool_searcher vec init only", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			std::boyer_moore_horspool_searcher b(vecNeedle.begin(), vecNeedle.end());
			DoNotOptimizeAway(&b);
		}
		return 0;
	});

	RunAndMeasure("vector of ints", [&]() {
		for (size_t i = 0; i < ITERS; ++i)
		{
			auto it = std::search(testVector.begin(), testVector.end(),
				std::boyer_moore_horspool_searcher(
					vecNeedle.begin(), vecNeedle.end()));
			if (it == testVector.end())
				std::cout << "The pattern " << needle << " not found\n";
		}
		return 0;
	});
}
