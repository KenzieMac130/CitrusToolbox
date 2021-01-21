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
	void* Data() const;
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

	/*Case Sensitive Compare*/
	friend bool operator==(ctStringUtf8& a, const char* b);
	friend bool operator==(ctStringUtf8& a, ctStringUtf8& b);
	friend bool operator==(const char* a, ctStringUtf8& b);

	void Printf(size_t max, const char* format, ...);

	int Cmp(const ctStringUtf8& str) const;
	int Cmp(const char* str) const;
	int Cmp(const char* str, const size_t len) const;

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