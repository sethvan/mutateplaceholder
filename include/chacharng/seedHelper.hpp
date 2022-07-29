/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * seedHelper.hpp: Header for the functions used to generate the seed for the chacharng random number generator.
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

#ifndef _INCLUDED_SEEDHELPER_HPP_
#define _INCLUDED_SEEDHELPER_HPP_

#include <array>

#include "chacharng/chacharng.hpp"

using SeedArray = std::array<std::uint8_t, SEED_SIZE_BYTES>;

int hexToInt(char hex);

char intToHex(int num);

bool parseHexString(const char *str, std::uint8_t *output, std::size_t outSize);

bool writeHexString(const char *str, std::uint8_t *output, std::size_t inSize);

void systemRandomFountain(void *output, std::size_t size);

SeedArray generateSeed();

#endif  // _INCLUDED_SEEDHELPER_HPP_