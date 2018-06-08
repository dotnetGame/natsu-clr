//
//
#include <iostream>
#include <vector>
#include <Windows.h>
#include <cassert>

std::vector<uint8_t> load_file(const char* filename)
{
	auto handle = CreateFileA(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, 0);
	assert(handle);
	auto size = GetFileSize(handle, nullptr);
	std::vector<uint8_t> data;
	data.resize(size);
	DWORD read = 0;
	auto hr = ReadFile(handle, data.data(), size, &read, nullptr);
	assert(hr);
	return data;
}

int main()
{
	auto file = load_file(R"(E:\Work\Repository\Tomato.Asr\Tomato.Asr.Web\bin\Any CPU\Debug\netcoreapp2.0\Tomato.Asr.Web.dll)");
	std::cout << "Test" << std::endl;
}
