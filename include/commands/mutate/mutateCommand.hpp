/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutateCommand.hpp: Header to be used only by main.cpp to bolt things together
 *
 * - This can be thought of as a self-contained subprogram within the larger
 mutation program
 * - This manages, organizes, and glues together all the neccecary functionality
 to apply random mutations from a tsv upon a source code file
 * - The output is the source code with mutations applied to it
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

#ifndef _INCLUDED_COMMANDS_MUTATE_HPP
#define _INCLUDED_COMMANDS_MUTATE_HPP

#include "commands/cli-options.hpp"
#include "common.hpp"
//#include <cstdint>
//#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

#include "mutationsRetriever.hpp"
#include "mutationsSelector.hpp"
#include "mutator.hpp"

	
std::string printMutateHelp(const char *indent);

std::string printMutateHelp(std::string indent);

std::string printMutateHelp(void);

void validateMutateArgs(CLIOptions *opts, std::vector<std::string> *nonpositionals);

void doMutateAction(CLIOptions *opts, std::vector<std::string> *nonpositionals);

ParseArgvStatusCode execMutate(CLIOptions *opts, std::vector<std::string> *nonpositionals);


#endif  //_INCLUDED_COMMANDS_MUTATE_HPP
