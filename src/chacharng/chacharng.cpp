/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * chacharng.cpp: Definition of the random number generator used for this program.
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

#include "chacharng/chacharng.hpp"

#include <cstdint>
#include <cstring>

#define CHACHARNG_ROTL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
#define CHACHARNG_QR(a, b, c, d)                                                                           \
    (a += b, d ^= a, d = CHACHARNG_ROTL(d, 16), c += d, b ^= c, b = CHACHARNG_ROTL(b, 12), a += b, d ^= a, \
     d = CHACHARNG_ROTL(d, 8), c += d, b ^= c, b = CHACHARNG_ROTL(b, 7))
#define CHACHARNG_ROUNDS 20

static void chacha_block(std::uint32_t out[16], std::uint32_t const in[16]) {
    std::uint32_t x[16];

    for (int i = 0; i < 16; ++i) x[i] = in[i];
    // 10 loops × 2 rounds/loop = 20 rounds
    for (int i = 0; i < CHACHARNG_ROUNDS; i += 2) {
        // Odd round
        CHACHARNG_QR(x[0], x[4], x[8], x[12]);   // column 0
        CHACHARNG_QR(x[1], x[5], x[9], x[13]);   // column 1
        CHACHARNG_QR(x[2], x[6], x[10], x[14]);  // column 2
        CHACHARNG_QR(x[3], x[7], x[11], x[15]);  // column 3
        // Even round
        CHACHARNG_QR(x[0], x[5], x[10], x[15]);  // diagonal 1 (main diagonal)
        CHACHARNG_QR(x[1], x[6], x[11], x[12]);  // diagonal 2
        CHACHARNG_QR(x[2], x[7], x[8], x[13]);   // diagonal 3
        CHACHARNG_QR(x[3], x[4], x[9], x[14]);   // diagonal 4
    }
    for (int i = 0; i < 16; ++i) out[i] = x[i] + in[i];
}

State::State() {}

State::State(std::uint8_t *inSeed) { seed(inSeed); }

void State::seed(std::uint8_t *inSeed) {
    std::memcpy((void *)this->block, (const void *)"expand 32-byte k", 16);

    for (int i = 0; i < 8; i++)
        this->block[i + 4] =
            (inSeed[i * 4 + 0] << 24) | (inSeed[i * 4 + 1] << 16) | (inSeed[i * 4 + 2] << 8) | inSeed[i * 4 + 3];

    // counter:
    this->block[16 - 4] = 0;

    // nonce values. Relevant xkcd: https://xkcd.com/221/
    this->block[16 - 3] = 0xfa427c2c;
    this->block[16 - 2] = 0x9422e076;
    this->block[16 - 1] = 0xb0ea2065;

    this->pos = 16;
}

std::uint32_t State::next32() {
    if (16 <= this->pos) {
        ++this->block[12];
        chacha_block(this->out, this->block);
        this->pos = 0;
    }

    std::uint32_t result = this->out[this->pos];

    this->pos += 1;

    return result;
}

std::uint64_t State::next64() {
    if (15 <= this->pos) {
        chacha_block(this->out, this->block);
        this->pos = 0;
    }

    std::uint64_t result = ((std::uint64_t)(this->out[this->pos]) << 32) | (std::uint64_t)(this->out[this->pos + 1]);

    this->pos += 2;

    return result;
}

State::result_type State::operator()() { return next32(); }

uint32_t nextRNGBetween(const uint32_t min, const uint32_t max, State &generator) {
    uint32_t diff = max - min;
    uint32_t rng;

    // keep polling until we get a usable random number
    do {
        rng = generator();

        if (rng < diff) {  // (diff - 1) is worst possible cutoff
            uint32_t cutoff = UINT32_MAX % diff;
            if (rng < cutoff) continue;
        }
    } while (0);

    return (rng % diff) + min;
};
