/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * iohelper.hpp: A mini I/O utility header tightly coupled with commands/cli-options.cpp
 *
 * - The iohelper::IOHelper class holds the values of the parsed input command line arguments
 * - (note that cli-parser.cpp fills up this class with the actual values to be used)
 * - This should be the one and only file that reads input files/stdin and writes to output files/stdout
 *
 * Copyright (c) 2023 RightEnd
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


#ifndef _INCLUDED_IOHELPERS_HPP
#define _INCLUDED_IOHELPERS_HPP

#include "common.hpp"
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string>
#include <optional>

#ifndef EOK
// just in case
#  define EOK 0
#endif

constexpr size_t IO_BUFF_SIZE = 16384;

std::string readWholeFileIntoString(std::FILE * handle, const char * errMsg);
	
void initializeSrcTsvTogetherFromStdin(std::optional<std::string> * srcString, std::optional<std::string> * tsvString);

void writeStringToFileHandle(std::FILE * handle, std::string text);
	
void readSeedFileIntoString(std::FILE * seedInput, std::optional<std::string> * output);
	
void closeAndNullifyFileHandle(std::FILE ** handle);


#endif//_INCLUDED_IOHELPERS_HPP
