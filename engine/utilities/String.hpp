#pragma once

#include "Common.h"

class ctStringUtf8 {
public:
	ctStringUtf8();
	ctStringUtf8(ctStringUtf8& str);
	ctStringUtf8(const char* input, const size_t count);
	ctStringUtf8(const char* input);

	ctResults MapMemory(char* dest, size_t capacity);

	const char* CStr() const;
	void* Data();
	size_t CodeLength() const;
	size_t ByteLength() const;
	size_t Capacity() const;
	ctResults Reserve(const size_t amount);
	void Clear();

	/* ASCII Char Concat */
	ctStringUtf8& operator+=(const char c);
	/* Unicode Char Concat */
	ctStringUtf8& operator+=(const int32_t c);
	/* C String Concat */
	ctStringUtf8& operator+=(const char* str);
	/* StringUtf8 Concat */
	ctStringUtf8& operator+=(const ctStringUtf8& str);

	void Printf(size_t max, const char* format, ...);

	ctStringUtf8& ToUpper();
	ctStringUtf8& ToLower();

private:
	void* _dataVoid() const;
	void* _dataVoidOffset(size_t offset) const;
	void _removeNullTerminator();
	void _nullTerminate();

	size_t pStaticCapacity;
	void* pStaticData;
	ctDynamicArray<char> _data;
};