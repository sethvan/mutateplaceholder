/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * validate.hpp: Header to be used only by main.cpp to bolt things together
 *
 * - This can be thought of as a self-contained subprogram within the larger mutation program
 * - This manages, organizes, and glues together all the neccecary functionality to find "dead" mutation (mutations that
     don't match any source code lines)
 * - The output is a pretty-printed list of lines in the mutation file which do not match any source code lines followed
     by a general pretty-printed one-line summary of useful statistics about the mutation file
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

#ifndef _INCLUDED_COMMANDS_VALIDATE_HPP
#define _INCLUDED_COMMANDS_VALIDATE_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "commands/cli-options.hpp"
#include "common.hpp"

std::string printValidateHelp(const char *indent);

std::string printValidateHelp(std::string indent);

std::string printValidateHelp(void);

void validateValidateArgs(CLIOptions *opts, std::vector<std::string> *nonpositionals);

void doValidateAction(CLIOptions *opts, std::vector<std::string> *nonpositionals);

ParseArgvStatusCode execValidate(CLIOptions *opts, std::vector<std::string> *nonpositionals);

#endif  //_INCLUDED_COMMANDS_VALIDATE_HPP
