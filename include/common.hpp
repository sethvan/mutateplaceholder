/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * common.cpp: Common utilities header
 *
 * - Basically any generic routine useful in multiple files but not fitting into the context of other files
 * - In other words, this file is a miscellaneous trash dump of whatever for whereever
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

#ifndef _INCLUDED_COMMON_HPP
#define _INCLUDED_COMMON_HPP

#include <cstddef>
#include <string>

#define PROGRAM_NAME "mutateplaceholder"
#define PROGRAM_VERSION "0.1"
#define PROGRAM_COPYRIGHT "RightEnd"

#define RNG_SEED_LENGTH 64
#define PRIM_MACRO_QUOTE(NAME) NAME

#if defined(NDEBUG)
// no-op the break point macro just in case
#define DEBUG_GDB_BREAK_POINT() \
    do {                        \
    } while (0)
#elif defined(X86)
#define DEBUG_GDB_BREAK_POINT() __asm__ __volatile__("int 3")
#elif defined(ARM)
#define DEBUG_GDB_BREAK_POINT() __asm__ __volatile__("bkpt")
#else
#define DEBUG_GDB_BREAK_POINT() \
    do {                        \
    } while (0)
#endif

enum class ParseArgvStatusCode : unsigned char { SUCCESS, ERROR, SHOWHELP, SHOWVERSION };

extern bool verbose;  // for printing status of process messages in classes

// remove special characters from a string so it can be safely shown in the console without risk of introducing security
// vulnerabilities
std::string sanitizeOutputMessage(const char* input);

std::string sanitizeOutputMessage(std::string input);

// Check for unicode white spaces as well as ascii , if not white space returns 0, if white space returns the amount of
// bytes it takes up Parameter end is .end() of std::string
unsigned int isWhiteSpace(const std::string::iterator& it, const std::string::iterator& end);

// Returns position from starting position `begin` to last non white character in the section of std::string being
// passed in Uses isWhiteSpace() function above to check for unicode white spaces as well as ascii Parameter `end`
// should either be one position passed the last position you want checked or .end() of the std::string If passed
// string/sub-string contains only white spaces, function returns std::string::npos NOTE: THIS FUNCTION ONLY WORKS IF
// PASSED IN STRING DOES NOT CUTOFF ANY PORTION OF MULTI-BYTE CHARACTERS BUT SINCE THE
//      USE CASES IN THIS PROJECT DO NUT RUN THAT RISK IT IS OK FOR OUR PURPOSES.
size_t lastNonWhiteSpace(std::string::iterator begin, std::string::iterator end);

#endif  //_INCLUDED_COMMON_HPP
