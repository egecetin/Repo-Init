#pragma once

#include <cstdio>

constexpr size_t constHasher(const char *s) { return *s ? *s + 33 * constHasher(s + 1) : 5381; }

// Create entropy using __FILE__ and __LINE__. Derived from Evan McBroom's
// https://gist.github.com/EvanMcBroom/ad683e394f84b623da63c2b95f6fb547
template <size_t N> constexpr uint64_t constSeeder(const char (&entropy)[N], const uint64_t iv = 0)
{
	auto value{iv};
	for (size_t i{0}; i < N; i++)
	{
		// Xor 1st byte of seed with input byte
		value = (value & (unsigned(~0) << 8)) | ((value & 0xFF) ^ entropy[i]);
		// Rotl 1 byte
		value = value << 8 | value >> ((sizeof(value) * 8) - 8);
	}
	return value ^ iv;
}
