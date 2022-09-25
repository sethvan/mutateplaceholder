/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * highlight.cpp: The main.cpp of mutation highlighting
 *
 * - This can be thought of as a self-contained subprogram within the larger mutation program
 * - This manages, organizes, and glues together all the neccecary functionality for mutation highlighting
 * - There are three possible outputs:
 *   1. An interactive HTML file which shows a side-by-side comparison with useful information and line highlighting
 *   2. A textual represention of the source code file with escape sequence coloring and #needed/#wanted fraction beside
 the line number to the left
 *   3. A textual represention of the TSV mutation file with escape sequence coloring and #needed/#wanted fraction
 beside the line number to the left
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

#include "commands/highlight/highlightCommand.hpp"

#include <sstream>

#include "excepts.hpp"

std::string printHighlightHelp(const char *indent) {
    std::ostringstream ss;
    //              "--version                "
    ss << indent
       << "-f, --format             Format of the output file. One of html, srctext, or tsvtext. Defaults to html\n";

    return ss.str();
};

std::string printHighlightHelp(std::string indent) { return printHighlightHelp(indent.c_str()); }

std::string printHighlightHelp(void) { return printHighlightHelp(""); }

void validateHighlightArgs(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    if (opts->hasSeed()) throw InvalidArgumentException("Cannot use the --seed/--read-seed options in highlight mode");
    if (opts->hasMutCount()) throw InvalidArgumentException("Cannot use the --count option in highlight mode");
    if (opts->hasMinMutCount()) throw InvalidArgumentException("Cannot use the --min-count option in highlight mode");
    if (opts->hasMaxMutCount()) throw InvalidArgumentException("Cannot use the --max-count option in highlight mode");
    if (1 < nonpositionals->size())
        throw InvalidArgumentException("highlight mode does not accept extra non-positional arguments");

    if (!opts->hasFormat()) {
        opts->setFormat("html");
    }

    // NOTE: this is the place to do file parsing and file syntax validation
}

void doHighlightAction(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    // TODO: actually do stuff here
    (void)nonpositionals;  // silence unused warnings

    switch (opts->getFormat()) {
        case Format::HTML:
            opts->putResOutput("<!doctype html>\n<html lang=\"en\">\n<body>\n<p>Hello world!</p>\n</body>\n</html>");
            break;
        case Format::SRCTEXT:
            opts->putResOutput(opts->getSrcString());
            break;
        case Format::TSVTEXT:
            opts->putResOutput(opts->getTsvString());
            break;
    }
}

ParseArgvStatusCode execHighlight(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    validateHighlightArgs(opts, nonpositionals);
    doHighlightAction(opts, nonpositionals);
    return ParseArgvStatusCode::SUCCESS;
}
