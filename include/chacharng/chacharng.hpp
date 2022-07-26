/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * chacharng.hpp: Header for the random number generator used in this program
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

#ifndef _INCLUDED_CHACHARNG_HPP
#define _INCLUDED_CHACHARNG_HPP

#include <cstddef>
#include <cstdint>


constexpr std::size_t SEED_SIZE_BYTES = 8 * 4;

class State {
	private:
		std::uint32_t block[16];
		std::uint32_t out[16];
		unsigned pos;

	public:
		using result_type = uint32_t;

		State();

		State(std::uint8_t* inSeed);

		static inline constexpr result_type min() { return 0; }
		static inline constexpr result_type max() { return UINT32_MAX; }

		void seed(std::uint8_t* inSeed);

		std::uint32_t next32();
		std::uint64_t next64();

		result_type operator()();
};

uint32_t nextRNGBetween(const uint32_t min, const uint32_t max, State& generator); 



#endif  //_INCLUDED_CHACHARNG_HPP
