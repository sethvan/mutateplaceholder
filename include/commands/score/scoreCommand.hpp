/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * score.hpp: Header to be used only by main.cpp to bolt things together
 *
 * - This can be thought of as a self-contained subprogram within the larger mutation program
 * - This manages, organizes, and glues together all the neccecary functionality to score the quality of the mutations
     TSV file in the context of a source code file
 * - The output is a pretty-printed list of lines in the source code file which have an insufficient number of mutation
     permutations followed by a general pretty-printed one-line summary of useful statistics about the source code file
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

#ifndef _INCLUDED_COMMANDS_SCORE_HPP
#define _INCLUDED_COMMANDS_SCORE_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "commands/cli-options.hpp"
#include "common.hpp"

std::string printScoreHelp(const char *indent);

std::string printScoreHelp(std::string indent);

std::string printScoreHelp(void);

void validateScoreArgs(CLIOptions *opts, std::vector<std::string> *nonpositionals);

void doScoreAction(CLIOptions *opts, std::vector<std::string> *nonpositionals);

ParseArgvStatusCode execScore(CLIOptions *opts, std::vector<std::string> *nonpositionals);

#endif  //_INCLUDED_COMMANDS_SCORE_HPP
