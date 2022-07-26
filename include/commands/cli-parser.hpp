/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * iohelper.hpp: Parse process argv input header
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

#ifndef _INCLUDED_COMMANDS_CLI_PARSER_HPP
#define _INCLUDED_COMMANDS_CLI_PARSER_HPP

#include "commands/cli-options.hpp"
#include "common.hpp"
#include <vector>
#include <string>


	
extern const char * STDIN_DASH_INIDCATOR;
	
ParseArgvStatusCode parseArgs(CLIOptions * output, std::vector<std::string> * nonposes, int argc, const char **argv);

#endif//_INCLUDED_COMMANDS_CLI_PARSER_HPP
