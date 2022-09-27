#pragma once

#include <cstdio>

constexpr size_t constHasher(const char *s, size_t index = 0)
{
	return s + index == nullptr || s[index] == '\0' ? 55 : constHasher(s, index + 1) * 33 + (unsigned char)(s[index]);
}