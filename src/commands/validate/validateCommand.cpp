/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * validate.cpp: The main.cpp of validating mutations files
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

#include "commands/validate/validateCommand.hpp"

#include <sstream>

#include "excepts.hpp"

std::string printValidateHelp(const char *indent) {
    std::ostringstream ss;
    //              "--version                "
    ss << indent << "(no special options for validate)\n";

    return ss.str();
};

std::string printValidateHelp(std::string indent) { return printValidateHelp(indent.c_str()); }

std::string printValidateHelp(void) { return printValidateHelp(""); }

void validateValidateArgs(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    if (opts->hasSeed()) throw InvalidArgumentException("Cannot use the --seed/--read-seed options in validate mode");
    if (opts->hasMutCount()) throw InvalidArgumentException("Cannot use the --count option in validate mode");
    if (opts->hasMinMutCount()) throw InvalidArgumentException("Cannot use the --min-count option in validate mode");
    if (opts->hasMaxMutCount()) throw InvalidArgumentException("Cannot use the --max-count option in validate mode");
    if (opts->hasFormat()) throw InvalidArgumentException("Cannot use the --format option in validate mode");
    if (1 < nonpositionals->size())
        throw InvalidArgumentException("validate mode does not accept extra non-positional arguments");

    // NOTE: this is the place to do file parsing and file syntax validation
}

void doValidateAction(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    // TODO: actually do stuff here
    (void)nonpositionals;  // silence unused warnings

    opts->getTsvString();  // ensure we read the file or stdin
    opts->putResOutput(
        "100% of mutations match source code lines...0/0 mutations do match or something...Just make this text look "
        "pretty and functional and colored when implementing this");
}

ParseArgvStatusCode execValidate(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    validateValidateArgs(opts, nonpositionals);
    doValidateAction(opts, nonpositionals);
    return ParseArgvStatusCode::SUCCESS;
}
