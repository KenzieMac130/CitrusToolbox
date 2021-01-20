#pragma once

#include "Common.h"

class ctStringUtf8 {
public:
	ctStringUtf8();
	ctStringUtf8(ctStringUtf8& str);
	ctStringUtf8(const ctStringUtf8& str);
	ctStringUtf8(const char* input, const size_t count);
	ctStringUtf8(const char* input);

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

	uint32_t xxHash32(const int seed) const;
	uint32_t xxHash32() const;
	uint64_t xxHash64(const int seed) const;
	uint64_t xxHash64() const;

private:
	void* _dataVoid() const;
	void* _dataVoidOffset(size_t offset) const;
	void _removeNullTerminator();
	void _nullTerminate();

	ctDynamicArray<char> _data;
};