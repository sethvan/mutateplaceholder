/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * iohelper.cpp: Parse process argv input definitions
 *
 * - Dump parsed values into a iohelper::IOHelper instance
 * - Check iohelper::IOHelper for errors and report them if found
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

#include <getopt.h>
#include <stdint.h>
#include "commands/cli-parser.hpp"
#include <iostream>
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include "excepts.hpp"



enum class MutateOpts : int {
	_PADD_START = 255,
	MIN_COUNT,
	MAX_COUNT
};

static std::string genErrorMessage(const char * arg) {
	std::string s(" (at ");
	s.append( sanitizeOutputMessage(arg) );
	s.append("): ");
	return s;
}

const char * STDIN_DASH_INIDCATOR = "-";
	
// adapted from https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
ParseArgvStatusCode parseArgs(CLIOptions * output, std::vector<std::string> * nonPositionals, int argc, const char **argv) {
	static const char * short_options = "+i:m:o:r:w:s:p:c:f:h:v";

	static struct option long_options[] = {
		{"input",			required_argument,	NULL,	'i'},
		{"mutations",		required_argument,	NULL,	'm'},
		{"output",			required_argument,	NULL,	'o'},
		{"read-seed",		required_argument,	NULL,	'r'},
		{"write-seed",		required_argument,	NULL,	'w'},
		{"seed",			required_argument,	NULL,	's'},
		{"penetration",		required_argument,	NULL,	'p'},
		{"count",			required_argument,	NULL,	'c'},
		{"min-count",		required_argument,	NULL,	(int) MutateOpts::MIN_COUNT},
		{"max-count",		required_argument,	NULL,	(int) MutateOpts::MAX_COUNT},
		{"format",			required_argument,	NULL,	'f'},
		{"help",			no_argument,		NULL,	'h'},
		{"license",			no_argument,		NULL,	'v'},
		{"version",			no_argument,		NULL,	'v'},
		{NULL, 0, NULL, 0}
	};

	int nextStartIndex = 1;
	int previousStartIndex = 1;
	while (nextStartIndex < argc) {
		optind = nextStartIndex; /// skip over 0th argument
		opterr = 1; // print error messages to stderr
		
		previousStartIndex = nextStartIndex;
		while (1) {
			/* getopt_long stores the option index here. */
			int option_index = 0;

			const char * rawArgCur = optind < argc ? argv[optind] : "";

			optarg = nullptr; // set the default if it doesn't get modified
				
			int c = getopt_long(argc, (char* const*) argv, short_options, long_options, &option_index);

			/* Detect the end of the options. */
			if (c < 0) {
				break;
			}
					
			switch(c) {
				case 0:
					/* If this option set a flag, do nothing else now. */
					/*if (long_options[option_index].flag != 0)
						break;
					printf ("option %s", long_options[option_index].name);
					if (optarg)
						printf (" with arg %s", optarg);
					printf ("\n");*/
					break;

				case 'i':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setSrcInput( optarg );
					break;

				case 'm':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setTsvInput( optarg );
					break;

				case 'o':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setResOutput( optarg );
					break;

				case 'r':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setSeedInput( optarg );
					break;

				case 'w':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setSeedOutput( optarg );
					break;

				case 's':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setSeed( optarg );
					break;

				case 'p':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setPenetration( optarg );
					break;

				case 'c':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setMutCount( optarg );
					break;

				case (int) MutateOpts::MIN_COUNT:
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setMinMutCount( optarg );
					break;

				case (int) MutateOpts::MAX_COUNT:
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setMaxMutCount( optarg );
					break;

				case 'f':
					if (optarg == nullptr) throw std::runtime_error( genErrorMessage( rawArgCur ) );
					output->setFormat( optarg );
					break;

				case 'h':
					return ParseArgvStatusCode::SHOWHELP;

				case 'v':
					return ParseArgvStatusCode::SHOWVERSION;

				case '?':
					/* getopt_long already printed an error message. */
					return ParseArgvStatusCode::ERROR;
						
				default:
					if (1) {
						// what happened?
						std::ostringstream ss;
						ss << "FATAL ERR: getopts_long returned invalid option id " << c;
						throw IOErrorException( ss.str() );
					}
			}
		}

		// The following logic below ensures that non-positional arguments can be mixed with options even in libraries with a POSIX compliant getopts_long
		//  The issue is that strict POSIX requires the first non-option argument and all successive arguments to be treated as non-positions, which is no fun.
			
		nextStartIndex = optind;
		
		if (previousStartIndex == nextStartIndex) {
			// prevent infinite loop
			nonPositionals->emplace_back( argv[nextStartIndex] );
			++nextStartIndex;
		}

		if (0 < optind && 0 == strcmp(argv[nextStartIndex - 1], "--")) {
			goto doubleDashEndOptions; // please just ignore this line of code and let's pretend that I didn't use this evil cursed statement.
		}
			
		while (nextStartIndex < argc && argv[nextStartIndex][0] != '-') {
			nonPositionals->emplace_back( argv[nextStartIndex] );
			++nextStartIndex;
			break;
		}

		if (nextStartIndex < argc && 0 == strcmp(argv[nextStartIndex], "--")) {
			++nextStartIndex;
			
			doubleDashEndOptions:
				
			while (nextStartIndex < argc) {
				nonPositionals->emplace_back( argv[nextStartIndex] ); 
				++nextStartIndex;
			}
				
			break;
		}
	}
	return ParseArgvStatusCode::SUCCESS;
}


