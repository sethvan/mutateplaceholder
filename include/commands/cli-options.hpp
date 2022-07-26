/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * iohelper.hpp: All I/O and input arguments validation header
 *
 * - The CLIOptions class holds the values of the parsed
 input command line arguments
 * - (note that cli-parser.cpp fills up instances of this class with the actual
 values to be used)
 * - This is one of the few files that reads input files/stdin and writes to
 output files/stdout
 * - This file is also tasked with input argument validation
 * - This file MUST NOT validate the syntax/correctness of input file contents.
 That is a job for elsewhere
 * - This file MUST NOT process default value logic unless its purely internal
 (e.x. defaulting to reading from stdin).
 *   \- Instead, users of this class check hasValue() to determine whether to
 getValue() or defaultValue
 * - NOTICE: This file does have quite a broad scope of things it does, and that
 is okay. If decoupling a group of closely intertwined functionalities into
 separate files would significantly increase the complexity and lines of code,
 then its usually fine to lump the group of things together into one file.
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

#ifndef _INCLUDED_COMMANDS_CLI_OPTIONS_HPP
#define _INCLUDED_COMMANDS_CLI_OPTIONS_HPP

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <string>

#include "common.hpp"
#include "chacharng/chacharng.hpp"
#include "chacharng/seedHelper.hpp"


	
	enum class Format : unsigned char { HTML, SRCTEXT, TSVTEXT };

	class CLIOptions  {
		private:
			FILE* srcInput;
			FILE* tsvInput;
			FILE* resOutput;
			FILE* seedInput;
			FILE* seedOutput;

		protected:
			std::optional<std::string> seedString;
			std::optional<std::string> srcString;
			std::optional<std::string> tsvString;
			// std::optional<std::string> resString;
            
			
			std::optional<std::int32_t> mutCount;
			std::optional<std::int32_t> minMutCount;
			std::optional<std::int32_t> maxMutCount;
			std::optional<std::int32_t> penetration;
			std::optional<Format> format;

			std::vector<std::string> warnings;
			std::vector<int> noMatchLines;
			std::vector<int> multipleMatchLines;

			void setSrcOrTsvInput(FILE** srcOrTsv, const char* path, const char* mode,
									int bufferMode, const char* which);
			void setSeedInputOrOutput(FILE** inOrOut, const char* path, const char* mode,
									int bufferMode, const char* which);
			void setMinOrMaxMutCount(std::optional<std::int32_t>* minOrMax,
									const char* count, const char* shortName,
									const char* fullName);

		public:
			CLIOptions();

			// These functions throw exceptions when there is an error
			void setSrcInput(const char* path);
			void setTsvInput(const char* path);
			void setResOutput(const char* path);
			void setSeedInput(const char* path);
			void setSeedOutput(const char* path);
			void setSeed(const char* seed);
			void setMutCount(const char* count);
			void setMinMutCount(const char* count);
			void setMinMutCount(std::int32_t count);
			void setMaxMutCount(const char* count);
			void setMaxMutCount(std::int32_t count);
			void setPenetration(const char* count);
			void setFormat(const char* fmt);
			std::string getSrcString();
			std::string getTsvString();
			void putResOutput(std::string result);
			void putSeedOutput(std::string result);

			// check these before using a getter as getters will throw
			bool hasSeed();
			bool hasMutCount();
			bool hasMinMutCount();
			bool hasMaxMutCount();
			bool hasPenetration();
			bool hasFormat();

			bool seedNeedsExporting();
			

			// These will throw a std::bad_optional_access error if no value was
			// defined/provided, so be sure to check the hasValue() methods first
			std::string getSeed();
			int32_t getMutCount();
			int32_t getMinMutCount();
			int32_t getMaxMutCount();
			int32_t getPenetration();
			Format getFormat();

			void addWarning(std::string str);
			void addNoMatchLine(int n);
			void addMultipleMatchLine(int n);
			std::string getWarnings(); 

			~CLIOptions();
	};


#endif  //_INCLUDED_COMMANDS_CLI_OPTIONS_HPP
