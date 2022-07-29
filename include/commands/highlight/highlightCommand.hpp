/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * highlight.hpp: Header to be used only by main.cpp to bolt things together
 *
 * - This can be thought of as a self-contained subprogram within the larger mutation program
 * - This manages, organizes, and glues together all the neccecary functionality for mutation highlighting
 * - The output is an interactive HTML file which shows a side-by-side comparison with useful information and line
 highlighting
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

#ifndef _INCLUDED_COMMANDS_HIGHLIGHT_HPP
#define _INCLUDED_COMMANDS_HIGHLIGHT_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "commands/cli-options.hpp"
#include "common.hpp"

std::string printHighlightHelp(const char *indent);

std::string printHighlightHelp(std::string indent);

std::string printHighlightHelp(void);

void validateHighlightArgs(CLIOptions *opts, std::vector<std::string> *nonpositionals);

void doHighlightAction(CLIOptions *opts, std::vector<std::string> *nonpositionals);

ParseArgvStatusCode execHighlight(CLIOptions *opts, std::vector<std::string> *nonpositionals);

#endif  //_INCLUDED_COMMANDS_HIGHLIGHT_HPP
