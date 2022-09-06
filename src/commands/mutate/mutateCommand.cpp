/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutate.cpp: The main.cpp of applying mutations
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

#include "commands/mutate/mutateCommand.hpp"

#include <filesystem>
#include <sstream>

#include "excepts.hpp"

std::string printMutateHelp(const char *indent) {
    std::ostringstream ss;
    //              "--version                "
    ss << indent << "-s, --seed=HEXSTRING     Pass seed in as CLI argument. Defaults to generating a new seed\n";
    ss << indent << "-r, --read-seed=FILE     Read PRNG seed from this file. Defaults to generating a new seed\n";
    ss << indent << "-w, --write-seed=FILE    Write PRNG seed out to this file. Defaults to discarding the seed\n";
    ss << indent
       << "-c, --count=NUMBER       Number of mutations to perform. Defaults to a random number of mutations\n";
    ss << indent << "    --min-count=NUMBER   Minimum number of mutations to perform. Defaults to 1\n";
    ss << indent
       << "    --max-count=NUMBER   Maximum number of mutations to perform. Defaults to the available number of "
          "mutations\n";
    ss << '\n';
    ss << indent
       << "-F, --force              Overwrite existing file specified for mutated output. Defaults to aborting if "
          "output file already exists\n";
    ss << '\n';
    ss << indent
       << "NOTE: The options --read-seed and --seed are mutally exclusive. You can't use both at the same time.\n";
    ss << indent
       << "NOTE: The groups --count and --min-count/--max-count are mutally exclusive. You can't specify --count if "
          "you specify --min-count or "
       << "--max-count\n";
    ss << indent
       << "NOTE: If both --input and --mutations are unspecified, then the first line from stdin is swallowed and used "
          "to separate --input and "
       << "--mutations\n";

    return ss.str();
};

std::string printMutateHelp(std::string indent) { return printMutateHelp(indent.c_str()); }

std::string printMutateHelp(void) { return printMutateHelp(""); }

void validateMutateArgs(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    if (opts->hasFormat()) throw InvalidArgumentException("Cannot use the --format option in mutate mode");
    if (1 < nonpositionals->size())
        throw InvalidArgumentException("mutate mode does not accept extra non-positional arguments");

    if (opts->hasOutputFileName()) {
        const char *path = opts->getOutputFileName();
        if (std::filesystem::exists(path) && !opts->okToOverwriteOutputFile()) {
            std::ostringstream os;
            os << "Output file \'" << path << "\' already exists. Use \'-F\' to force overwrite.";
            throw IOErrorException(sanitizeOutputMessage(os.str()));
        }
        opts->setResOutput(path);
    }
    else if (opts->okToOverwriteOutputFile()) {
        throw InvalidArgumentException("Option --force invalid when no output file is specified.");
    }

    // TSV parsing and validation performed by MutationsRetriever class in doAction()
    // mutCount setting and seed hex validation or (if needed) seed generation performed by MutationsSelector class in
    // doAction()
}

void doMutateAction(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    // TODO: actually do stuff here
    (void)nonpositionals;  // silence unused warnings

    MutationsRetriever mRetriever{opts->getTsvString()};
    MutationsSelector mSelector{opts, mRetriever.getPossibleMutations()};
    std::string outputString;
    Mutator mutator{opts->getSrcString(), outputString, mSelector.getSelectedMutations(), opts};

    opts->putResOutput(outputString);

    // std::cout << mutator.mutatedLines.size() << " mutations have been successfully applied across "
    // 		<< mutator.mutatedLineCount << " lines" << std::endl;

    if (opts->seedNeedsExporting()) { opts->putSeedOutput(opts->getSeed()); }
}

ParseArgvStatusCode execMutate(CLIOptions *opts, std::vector<std::string> *nonpositionals) {
    validateMutateArgs(opts, nonpositionals);
    doMutateAction(opts, nonpositionals);
    return ParseArgvStatusCode::SUCCESS;
}
