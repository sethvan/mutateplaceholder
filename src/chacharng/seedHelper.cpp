/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * seedHelper.cpp: Defines functions used to generate the seed for the chacharng random number generator. 
 *
 * Copyright (c) 2022 RightEnd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "chacharng/seedHelper.hpp"

#include <sys/time.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>



int hexToInt(char hex) {
	if ('0' <= hex && hex <= '9') {
		return hex - '0';
	} 
	else if ('A' <= hex && hex <= 'F') {
		return 10 + hex - 'A';
	} 
	else if ('a' <= hex && hex <= 'f') {
		return 10 + hex - 'a';
	} 
	else {
		return -1;
	}
}

char intToHex(int num) {
	if (num < 0 || 15 < num) return -1;

	return "0123456789ABCDEF"[num];
}

// returns true when successful
bool parseHexString(const char *str, std::uint8_t *output, std::size_t outSize) {
	for (std::size_t index = 0; index < (outSize * 2); index += 2) {
		if (str[index] == 0) return false;
		if (str[index + 1] == 0) return false;

		int first = hexToInt(str[index]);
		int second = hexToInt(str[index + 1]);
		if (first < 0 || second < 0) return false;

		output[index / 2] = (first << 4) | second;
	}
	return true;
}

bool writeHexString(const char *str, std::uint8_t *output, std::size_t inSize) {
	for (std::size_t index = 0; index < (inSize * 2); index += 2) {
		output[index] = intToHex(((unsigned char)str[index / 2]) >> 4);
		output[index + 1] = intToHex(str[index / 2] & 0xf);
	}
	return true;
}

void systemRandomFountain(void *output, std::size_t size) {
	FILE *urandom = std::fopen("/dev/urandom", "rb");
	std::size_t readBytes = 0;

	if (urandom == nullptr) {
		urandom = std::fopen("/dev/random", "rb");
	}

	std::memset(output, 0, size);

	if (urandom != nullptr) {
		readBytes = std::fread(output, 1, size, urandom);
		std::fclose(urandom);
	}

	if (readBytes < size) {
		// We are left with no other option than time-based seeding :(

		struct timeval current_time;
		gettimeofday(&current_time, NULL);

		char *fakeRng = (char *)&current_time;
		char *charOut = (char *)output;
		for (std::size_t i = 0; i < sizeof(struct timeval) && i < size; ++i) {
			charOut[size - 1 - i] ^= fakeRng[i ^ (sizeof(struct timeval) / 2)];
		}
	}
}

using SeedArray = std::array<std::uint8_t, SEED_SIZE_BYTES>;

SeedArray generateSeed() {
	SeedArray arr;
	systemRandomFountain((void *)arr.data(), SEED_SIZE_BYTES);
	return arr;
}

	

