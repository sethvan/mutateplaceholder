/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * iohelper.cpp: Mini I/O utility defintions tightly coupled with commands/cli-options.cpp
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

#include "iohelpers.hpp"
#include <errno.h>
#include <optional>
#include <cstring>
#include <cstdio>
#include <excepts.hpp>


// helper method for reading a whole file into a std::string
std::string readWholeFileIntoString(std::FILE * handle, const char * errMsg) {
	char buffer[IO_BUFF_SIZE];
	std::string fileContents;
		
	while ( ! feof(handle)) {
		size_t readN = fread((void *) buffer, 1, IO_BUFF_SIZE, handle);
		int err = ferror( handle );
		if ((readN == 0 && ! feof( handle )) || (err != EOK && err != EAGAIN)) {
			throw IOErrorException( errMsg );
			return std::string("", 0);
		}
			
		fileContents.append( buffer, readN );
	}

	return fileContents;
}

// helper method for reading stdin up to a deliminator line or EOF
void readStdinLinesIntoOptionalString(char * deliminator, std::optional<std::string> * output) {
	char buffTmp[IO_BUFF_SIZE + 16];// = {0};
	int err = 0;
		
	while ( ! feof( stdin )) {
		if (nullptr == std::fgets(buffTmp, IO_BUFF_SIZE + 1, stdin)) {
			err = ferror( stdin );
			if (err == EAGAIN) continue;
			if (feof( stdin )) break;
			throw IOErrorException( "I/O error reading from stdin" );
		}
			
		if (0 == std::strcmp(deliminator, buffTmp)) {
			break;
		}
			
		if ( ! output->has_value()) {
			output->emplace( buffTmp );
		} 
		else {
			output->value().append(buffTmp);
		}
	}

	err = ferror( stdin );
	if (err != EOK && err != EAGAIN) {
		throw IOErrorException( "I/O error reading from stdin" );
	}
		
	if ( ! output->has_value()) output->emplace( "", 0 );

}
	
void initializeSrcTsvTogetherFromStdin(std::optional<std::string> * srcString, std::optional<std::string> * tsvString) {
	if (srcString->has_value() && tsvString->has_value()) return;

	char deliminator[IO_BUFF_SIZE + 16];// = {0};
		if (nullptr == std::fgets(deliminator, IO_BUFF_SIZE + 1, stdin)) {
		if ( ! feof( stdin )) {
			throw IOErrorException( "I/O error reading from stdin" );
		}
	}

	readStdinLinesIntoOptionalString(deliminator, srcString);
		
	if (feof( stdin )) {
		throw IOErrorException( "Encountered EOF in stdin before encountering the second deliminator (first line of stdin) separating the mutation file and the source code file" );
	}
		
	readStdinLinesIntoOptionalString(deliminator, tsvString);
}
	
void writeStringToFileHandle(std::FILE * handle, std::string textData) {
	const char * str = textData.c_str();
	const std::size_t len = textData.size();
		
	for (size_t pos=0; pos < len; ) {
		size_t written = std::fwrite((const void *)(str + pos), 1, len - pos, handle);
			
		int err = ferror( handle );
		if ((written == 0 && ! feof( handle )) || (err != EOK && err != EAGAIN)) {
			throw IOErrorException( "I/O error writing to output file" );
		}			
		pos += written;
	}
}

void readSeedFileIntoString(std::FILE * seedInput, std::optional<std::string> * output) {
	// "+ 1" is needed in order to hold the trailing NULL
	char readCharBuff[IO_BUFF_SIZE + 16] = {0};

	char * aliasPtr = std::fgets(readCharBuff, IO_BUFF_SIZE + 1, seedInput);
		
	if (aliasPtr == nullptr) {
		throw IOErrorException( "I/O error reading from seed file" );
	} 
	else {
		// First, trim trailing new line
		char * newLine = strchr( readCharBuff, '\n' );
		if (newLine != nullptr) *newLine = 0;
			
		output->emplace( readCharBuff );
	}

	if ( output->value().size() != RNG_SEED_LENGTH ) {
		throw InvalidSeedException(" Error : Invalid input seed. Expected 64 hexadecimal digits");			
	}
}

void closeAndNullifyFileHandle(std::FILE ** handle) {
	if (*handle != nullptr) {
		if (*handle != stdin && *handle != stdout) std::fclose( *handle );
			
		*handle = nullptr;
	}
}


