// Copyright (C) 2022 Evan McBroom
// If you are using Visual Studio, you will need to disable the "Edit and Continue" feature.

// Prng based off of Parker Miller's
// "Multiplicative Linear Congruential Generator"
// https://en.wikipedia.org/wiki/Lehmer_random_number_generator

#pragma once

namespace mlcg {
    constexpr uint32_t modulus() {
        return 0x7fffffff;
    }
  
    // Create entropy using __FILE__ and __LINE__
    template<size_t N>
    constexpr uint32_t seed(const char(&entropy)[N], const uint32_t iv = 0) {
        auto value{ iv };
        for (size_t i{ 0 }; i < N; i++) {
            // Xor 1st byte of seed with input byte
            value = (value & (unsigned(~0) << 8)) | ((value & 0xFF) ^ entropy[i]);
            // Rotl 1 byte
            value = value << 8 | value >> ((sizeof(value) * 8) - 8);
        }
        // The seed is required to be less than the modulus and odd
        while (value > modulus()) value = value >> 1;
        return value << 1 | 1;
    }

    constexpr uint32_t prng(const uint32_t input) {
        return (input * 48271) % modulus();
    }
}
