/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutate.cpp: The main.cpp of scoring mutations
 *
 * - This can be thought of as a self-contained subprogram within the larger mutation program
 * - This manages, organizes, and glues together all the neccecary functionality to score the quality of the mutations TSV file in the context of a source code file
 * - The output is a list of lines which have insufficient number of mutation permutations followed by a general pretty-printed one-line summary of useful statistics
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

#include "commands/score/scoreCommand.hpp"
#include <sstream>
#include "excepts.hpp"

	
std::string printScoreHelp(const char *indent) {
	std::ostringstream ss;
	//              "--version                "
	ss << indent << "(no special options for score)\n";

	return ss.str();
};

std::string printScoreHelp(std::string indent) {
	return printScoreHelp( indent.c_str() );
}

std::string printScoreHelp(void) {
	return printScoreHelp("");
}

void validateScoreArgs(CLIOptions * opts, std::vector<std::string> * nonpositionals) {
	if (opts->hasSeed()) throw InvalidArgumentException("Cannot use the --seed/--read-seed options in score mode");
	if (opts->hasMutCount()) throw InvalidArgumentException("Cannot use the --count option in score mode");
	if (opts->hasMinMutCount()) throw InvalidArgumentException("Cannot use the --min-count option in score mode");
	if (opts->hasMaxMutCount()) throw InvalidArgumentException("Cannot use the --max-count option in score mode");
	if (opts->hasPenetration()) throw InvalidArgumentException("Cannot use the --penetration option in score mode");
	if (opts->hasFormat()) throw InvalidArgumentException("Cannot use the --format option in score mode");
	if (1 < nonpositionals->size()) throw InvalidArgumentException("score mode does not accept extra non-positional arguments");

	// NOTE: this is the place to do file parsing and file syntax validation
		
}

void doScoreAction(CLIOptions * opts, std::vector<std::string> * nonpositionals) {
	// TODO: actually do stuff here
	(void) nonpositionals; // silence unused warnings
		
	opts->getSrcString(); // ensure we read the file or stdin
	opts->putResOutput( "100% of code is mutated properly...0/0 mutations fulfilled or something...Just make this text look pretty and functional and colored when implementing this" );
	
}

ParseArgvStatusCode execScore(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
	validateScoreArgs(opts, nonpositionals);
	doScoreAction(opts, nonpositionals);
	return ParseArgvStatusCode::SUCCESS;
}




