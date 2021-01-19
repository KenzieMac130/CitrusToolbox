#pragma once

#include "DynamicArray.hpp"

namespace CT_UTILITIES_NAMESPACE {

class StringUtf8 {
public:
	StringUtf8();
	StringUtf8(const char* input);
//Todo: tomorrow
//codelength (amount of characters, not bytes) 
//bytecount (amount of bytes)
private:
	DynamicArray<char*> _data;
};
}