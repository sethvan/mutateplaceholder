/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * main.cpp: Main program entry point
 *
 * - Bootstraps the startup routine and
 * - Should be the *only* file that prints errors to std:cerr
 * - Organizer of all other files
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

// test comment to test cmake addition
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "commands/cli-options.hpp"
#include "commands/cli-parser.hpp"
#include "commands/highlight/highlightCommand.hpp"
#include "commands/mutate/mutateCommand.hpp"
#include "commands/score/scoreCommand.hpp"
#include "commands/validate/validateCommand.hpp"
#include "common.hpp"
#include "excepts.hpp"

const bool verbose = true;  // to get status of process messages in classes

using commandsMap =
    const std::unordered_map<std::string_view, ParseArgvStatusCode (*)(CLIOptions *, std::vector<std::string> *)>;

// returns status code of success, error or showhelp or showversion
static ParseArgvStatusCode parseArgvAndPerformAction(int argc, const char **argv);

// processes final code
static int processFinalStatus(ParseArgvStatusCode status);

// ensures commandsMap does not contain any nullptr
static commandsMap setCommandsMap();

int main(int argc, const char **argv) {
    ParseArgvStatusCode status;

    try {
        status = parseArgvAndPerformAction(argc, argv);
    } catch (const TSVParsingException &ex) {
        std::cerr << PROGRAM_NAME << ": Error parsing TSV file\n" << ex.what() << std::endl;
        status = ParseArgvStatusCode::ERROR;
    } catch (const InvalidSeedException &ex) {
        std::cerr << PROGRAM_NAME << ": Error processing seed\n" << ex.what() << std::endl;
        status = ParseArgvStatusCode::ERROR;
    } catch (const InvalidArgumentException &ex) {
        std::cerr << PROGRAM_NAME << ": Error processing arguments\n" << ex.what() << std::endl;
        status = ParseArgvStatusCode::ERROR;
    } catch (const IOErrorException &ex) {
        std::cerr << PROGRAM_NAME << ": I/O error\n" << ex.what() << std::endl;
        status = ParseArgvStatusCode::ERROR;
    } catch (const std::exception &ex) {
        std::cerr << ": Error " << ex.what() << std::endl;
        status = ParseArgvStatusCode::ERROR;
    }

    return processFinalStatus(status);
}

static commandsMap setCommandsMap() {
    bool containsNullptr = true;

    while (containsNullptr) {
        containsNullptr = false;
        commandsMap temp = {
            {"mutate", &execMutate}, {"highlight", &execHighlight}, {"score", &execScore}, {"validate", &execValidate}};
        for (const auto &n : temp) {
            if (n.second == nullptr) {
                containsNullptr = true;
                break;
            }
        }
        if (!containsNullptr) return temp;
    }
    return commandsMap();  // to turn off  `error: control reaches end of non-void function [-Werror=return-type]`
}

static ParseArgvStatusCode parseArgvAndPerformAction(int argc, const char **argv) {
    if (argc < 2) { throw InvalidArgumentException("Too few arguments"); }

    CLIOptions parsedArgs;
    std::vector<std::string> nonpositionals;
    ParseArgvStatusCode status = parseArgs(&parsedArgs, &nonpositionals, argc, argv);

    if (status == ParseArgvStatusCode::SUCCESS && 1 < argc && 0 == nonpositionals.size()) {
        throw InvalidArgumentException(
            "No command specified (must be one of 'mutate', 'highlight', 'score', or 'validate')\n");
    }

    switch (status) {
        case ParseArgvStatusCode::SUCCESS:
            // continue on with the rest of the program
            break;
        case ParseArgvStatusCode::ERROR:
        case ParseArgvStatusCode::SHOWHELP:
        case ParseArgvStatusCode::SHOWVERSION:
            // exit immediately to show the right message
            return status;
    }

    commandsMap commands = setCommandsMap();
    std::string actionName = nonpositionals[0];

    try {
        ParseArgvStatusCode status = commands.at(actionName)(&parsedArgs, &nonpositionals);
        std::string warnings = parsedArgs.getWarnings();
        if (warnings.size()) { std::cerr << warnings; }
        return status;
    } catch (const std::out_of_range &ex) {
        std::cerr << "Out of range error: " << ex.what() << std::endl;
        return ParseArgvStatusCode::ERROR;
    }
}

static int processFinalStatus(ParseArgvStatusCode status) {
    const char *indent = "  ";

    switch (status) {
        case ParseArgvStatusCode::SUCCESS:
            // nothing to do
            std::cout << std::endl;
            return 0;

        case ParseArgvStatusCode::ERROR:
            std::cerr << "Try '" PROGRAM_NAME " --help' to see available options and information.\n\n";
            return 1;

        case ParseArgvStatusCode::SHOWHELP:
            std::cout << "Usage: " PROGRAM_NAME " <command> [OPTIONS...]\n" << '\n';

            std::cout << "mutate:\n";
            std::cout << printMutateHelp(indent) << '\n';

            std::cout << "highlight:\n";
            std::cout << printHighlightHelp(indent) << '\n';

            std::cout << "score:\n";
            std::cout << printScoreHelp(indent) << '\n';

            std::cout << "validate:\n";
            std::cout << printValidateHelp(indent) << '\n';

            std::cout << "Common options:\n";
            //           "  --version                ";
            std::cout << indent
                      << "-i, --input=FILE         Source code file to apply mutations to. Defaults to stdin\n";
            std::cout << indent
                      << "-m, --mutations=FILE     Mutations TSV file containing mutations. Defaults to stdin\n";
            std::cout << indent
                      << "-o, --output=FILE        Write mutated source code to this file. Defaults to stdout\n";
            std::cout << indent << "-h, --help               Show this help page\n";
            std::cout << indent << "-V, --license            Show license and version information\n";
            std::cout << '\n';
            std::cout << "E.x.: " PROGRAM_NAME " mutate --input code.c --mutations muts.tsv --output output.c\n";

            return 0;

        case ParseArgvStatusCode::SHOWVERSION:
            std::cout << PROGRAM_NAME ", version " PROGRAM_VERSION "\n";
            std::cout << "Copyright (C) " PROGRAM_COPYRIGHT ".\n";
            std::cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n";
            std::cout << '\n';
            std::cout << "This is free software; you are free to change and redistribute it.\n";
            std::cout << "There is NO WARRANTY, to the extent permitted by law.\n";

            return 0;
    }

    std::cerr << "FATAL ERR: Unknown internal status " << (int)status << '\n';
    return 2;
}