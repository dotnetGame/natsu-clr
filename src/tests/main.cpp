//
//
#include <iostream>
#include <vector>
#include <Windows.h>
#include <cassert>
#include <loader/AssemblyLoader.hpp>
#include <binder/Binder.hpp>
#include <binder/CorlibBinder.hpp>
#include <vm/Thread.hpp>

using namespace clr;
using namespace clr::loader;
using namespace clr::metadata;
using namespace clr::vm;

std::vector<uint8_t> load_file(const char* filename)
{
	auto handle = CreateFileA(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, 0);
	assert(handle != INVALID_HANDLE_VALUE);
	auto size = GetFileSize(handle, nullptr);
	std::vector<uint8_t> data;
	data.resize(size);
	DWORD read = 0;
	auto hr = ReadFile(handle, data.data(), size, &read, nullptr);
	assert(hr);
	return data;
}

void dummy_deleter(const uint8_t* ptr)
{
}

int main()
{
	auto file = load_file(__FILE__ R"(\..\..\System.Private.CorLib\bin\Debug\netstandard1.3\System.Private.CorLib.dll)");
	auto asmfile = std::make_shared<AssemblyFile>(std::shared_ptr<const uint8_t[]>(file.data(), dummy_deleter), file.size());
	auto loader = std::make_shared<AssemblyLoader>(asmfile);
	loader->Load();

    CorlibBinder::Initialize(loader);
	Thread thread;
	thread.assemblyLoader_ = loader.get();
	thread.Execute(*CorlibBinder::Current().BindMethod("System", "Console", "Test"));

	std::cout << "Test" << std::endl;
}
