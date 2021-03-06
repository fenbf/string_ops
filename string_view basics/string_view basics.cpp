// string_view_creation.cpp
// example for "C++17 In Detail"
// by Bartlomiej Filipek
// 2018

#include <iostream>
#include <string>
#include <string_view>

void* operator new(std::size_t n)
{
	std::cout << "new " << n << " bytes\n";
	return malloc(n);
}

void Creation()
{
	const char* cstr = "Hello World";

	// the whole string:
	std::string_view sv1{ cstr };
	std::cout << sv1 << ", len: " << sv1.size() << '\n';

	// slice
	std::string_view sv2{ cstr, 5 }; // not null-terminated!
	std::cout << sv2 << ", len: " << sv2.size() << '\n';

	// from string:
	std::string str = "Hello String";
	std::string_view sv3 = str;
	std::cout << sv3 << ", len: " << sv3.size() << '\n';

	// ""sv literal
	using namespace std::literals;
	std::string_view sv4 = "Hello\0 Super World"sv;
	std::cout << sv4 << ", len: " << sv4.size() << '\n';
	std::cout << sv4.data() << " - till zero\n";
}

std::string StartFromWordStr(const std::string& str, const std::string& word)
{
	return str.substr(str.find(word)); // substr creates a new string
}

std::string_view StartFromWord(std::string_view str, std::string_view word)
{
	return str.substr(str.find(word)); // substr creates only a new view
}

void AvoidingCopies()
{
	// the original string, one allocation:
	std::string str{ "Hello Amazing Programming Environment" };

	std::cout << "string version:\n";

	// one allocation to pass the param "Programming Environment"
	// another allocation in substr inside the function
	// try with "Programming" only or "Environment" only, sso
	auto subStr = StartFromWordStr(str, "Programming Environment");
	std::cout << subStr << "\n";

	std::cout << "string_view version:\n";

	// no allocation for the sub range:
	auto subView = StartFromWord(str, "Programming Environment");
	std::cout << subView << "\n";
}

int main()
{
	std::cout << sizeof(std::string_view) << ", " << sizeof(std::string) << '\n';

	//Creation();
	AvoidingCopies();
}