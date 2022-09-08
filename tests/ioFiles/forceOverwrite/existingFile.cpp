
/*
 * iohelper.cpp: All I/O and input arguments validation definitions
 *
 * - The CLIOptions class holds the values of the parsed
     input command line arguments
 * - (note that cli-parser.cpp fills up instances of this class with the actual
     values to be used)
 * - This is one of the few files that reads input files/stdin and writes to
     output files/stdout
 * - This file is also tasked with input argument validation
 * - This file MUST NOT validate the syntax/correctness of input file contents.
     That is a job for elsewhere
 * - This file MUST NOT process default value logic unless its purely internal
 (e.x. defaulting to reading from stdin).
 *   \- Instead, users of this class check hasValue() to determine whether to
     getValue() or defaultValue
 * - NOTICE: This file does have quite a broad scope of things it does, and that
     is okay. If decoupling a group of closely intertwined functionalities into
     separate files would signifigantly increase the complexity and lines of code,
 then its usually fine to lump the group of things together into one file.
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

#include "commands/cli-options.hpp"

#include <errno.h>

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

#include "chacharng/chacharng.hpp"
#include "chacharng/seedHelper.hpp"
#include "common.hpp"
#include "excepts.hpp"
#include "iohelpers.hpp"

CLIOptions::CLIOptions()
    : srcInput(stdin), tsvInput(stdin), resOutput(stdout), seedInput(nullptr), seedOutput(nullptr) {}

[(<<<<< CELL 2-A >>>>>)]
                                  [(<<<<< CELL 15-A >>>>>)]
    if (*srcOrTsv != stdin && *srcOrTsv != stdout) {
        [(<<<<< CELL 17-A >>>>>)]
        lastError.append(" file can only be specified once");
        [(<<<<< CELL 13-B >>>>>)]
    }

    *srcOrTsv = std::fopen(path, mode);
    if (*srcOrTsv != nullptr) {
        std::setvbuf(*srcOrTsv, nullptr, bufferMode, IO_BUFF_SIZE);
        return;
    }
    else {
        [(<<<<< CELL 22-B >>>>>)]
        lastError.append(which);
        [(<<<<< CELL 12-A >>>>>)]
        [(<<<<< CELL 13-B >>>>>)]
    }
}

void CLIOptions::setSeedInputOrOutput(FILE **inOrOut, const char *path, const char *mode, int bufferMode,
                                      [(<<<<< CELL 15-A >>>>>)]
    [(<<<<< CELL 16-A >>>>>)]
        [(<<<<< CELL 17-A >>>>>)]
        lastError.append(" file can only be specified once");
        [(<<<<< CELL 13-B >>>>>)]
    }

    *inOrOut = std::fopen(path, mode);
    [(<<<<< CELL 16-A >>>>>)]
        [(<<<<< CELL 21-A >>>>>)]
        return;
    }
    else {
        [(<<<<< CELL 22-B >>>>>)]
        lastError.append(which);
        [(<<<<< CELL 12-A >>>>>)]
        [(<<<<< CELL 13-B >>>>>)]
    }
}

void CLIOptions::setSrcInput(const char *path) {
    if (!std::filesystem::exists(path)) {
        [(<<<<< CELL 28-A >>>>>)]
        [(<<<<< CELL 29-B >>>>>)]
        throw IOErrorException(sanitizeOutputMessage(os.str()));
    }
    setSrcOrTsvInput(&(srcInput), path, "r", _IOFBF, "source code input");
}

void CLIOptions::setTsvInput(const char *path) {
    if (!std::filesystem::exists(path)) {
        [(<<<<< CELL 28-A >>>>>)]
        os << "TSV file \'" << path << "\' was not found.";
        throw IOErrorException(sanitizeOutputMessage(os.str()));
    }
    setSrcOrTsvInput(&(tsvInput), path, "r", _IOFBF, "TSV mutations input");
}

void CLIOptions::setOutputFileName(const char *path) { outputFileName = std::string(path); }

void CLIOptions::setResOutput(const char *path) {
    setSrcOrTsvInput(&(resOutput), path, "w", _IONBF, "resulting output");
}

void CLIOptions::setSeedInput(const char *path) {
    if (seedString.has_value()) {
        throw InvalidArgumentException("options --seed and --read-seed are mutually exclusive. Please choose one");
    }
    setSeedInputOrOutput(&(seedInput), path, "r", _IONBF, "seed input");
}

[(<<<<< CELL 45-B >>>>>)]
    setSeedInputOrOutput(&(seedOutput), path, "w", _IONBF, "seed output");
}

void CLIOptions::setSeed(const char *seed) {
    if (seedString.has_value()) { throw InvalidArgumentException("seed string can only be specified once"); }

    if (CLIOptions::seedInput != nullptr) {
        throw InvalidArgumentException("options --seed and --read-seed are mutually exclusive. Please choose one");
    }

    CLIOptions::seedString.emplace(seed);

    if (CLIOptions::seedString.value().size() != RNG_SEED_LENGTH) {
        char err[60];
        [(<<<<< CELL 54-C >>>>>)]
        throw InvalidSeedException(err);
    }
}

void CLIOptions::setMinOrMaxMutCount(std::optional<std::int32_t> *minOrMax, const char *count, const char *shortName,
                                     [(<<<<< CELL 57-D >>>>>)]
    [(<<<<< CELL 58-B >>>>>)]
        [(<<<<< CELL 59-B >>>>>)]
        [(<<<<< CELL 60-B >>>>>)]
        [(<<<<< CELL 13-B >>>>>)]
    }

    char *endPtr = (char *)count;
    unsigned long retStatus = strtoul(count, &endPtr, 0);

    if (endPtr == count || retStatus == ULONG_MAX || INT32_MAX < retStatus) {
        std::string lastError = "invalid value specified for --";
        lastError.append(shortName);
        lastError.append("count. Expected a number");
        [(<<<<< CELL 13-B >>>>>)]
    }

    *minOrMax = (std::int32_t)retStatus;

    if (mutCount.has_value() && (minMutCount.has_value() || maxMutCount.has_value())) {
        throw InvalidArgumentException(
            "option groups --count and --max-count/--min-count are mutually exclusive. Please choose one group");
    }
}

void CLIOptions::setMutCount(const char *count) { setMinOrMaxMutCount(&(mutCount), count, "", "mutation count"); }

void CLIOptions::setMinMutCount(const char *count) {
    setMinOrMaxMutCount(&(minMutCount), count, "min-", "minimum mutation count");
}

void CLIOptions::setMinMutCount(std::int32_t count) { minMutCount = count; }

void CLIOptions::setMaxMutCount(const char *count) {
    setMinOrMaxMutCount(&(maxMutCount), count, "max-", "maximum mutation count");
}

void CLIOptions::setMaxMutCount(std::int32_t count) { maxMutCount = count; }
static char asciitolower_for_format(char in) {
    if (in <= 'Z' && in >= 'A') return in - ('Z' - 'z');
    return in;
}

void CLIOptions::setFormat(const char *fmt) {
    if (CLIOptions::format.has_value()) { throw InvalidArgumentException("format can only be specified once"); }

    std::string str(fmt);
    [(<<<<< CELL 84-B >>>>>)]

    if (0 == std::strcmp(str.c_str(), "html")) { format = Format::HTML; }
    else if (0 == std::strcmp(str.c_str(), "srctext") || 0 == std::strcmp(str.c_str(), "srctxt")) {
        format = Format::SRCTEXT;
    }
    else if (0 == std::strcmp(str.c_str(), "tsvtext") || 0 == std::strcmp(str.c_str(), "tsvtxt")) {
        format = Format::TSVTEXT;
    }
    else {
        [(<<<<< CELL 88-D >>>>>)]
        lastError.append(sanitizeOutputMessage(fmt));
        lastError.append("\"");
        [(<<<<< CELL 13-B >>>>>)]
    }
}

std::string CLIOptions::getSrcString() {
    if (CLIOptions::srcInput == stdin && CLIOptions::tsvInput == stdin) {
        initializeSrcTsvTogetherFromStdin(&(CLIOptions::srcString), &(CLIOptions::tsvString));
    }
    if (!CLIOptions::srcString.has_value()) {
        srcString = readWholeFileIntoString(CLIOptions::srcInput, "I/O error reading source code file");
    }
    return srcString.value();
}

std::string CLIOptions::getTsvString() {
    if (srcInput == stdin && tsvInput == stdin) { initializeSrcTsvTogetherFromStdin(&(srcString), &(tsvString)); }
    if (!tsvString.has_value()) {
        tsvString = readWholeFileIntoString(tsvInput, "I/O error reading TSV mutations file");
    }
    return tsvString.value();
}

void CLIOptions::putResOutput(std::string result) {
    writeStringToFileHandle(resOutput, result);
}

void CLIOptions::putSeedOutput(std::string result) { writeStringToFileHandle(seedOutput, result); }

bool CLIOptions::hasSeed() { return seedString.has_value() || seedInput != nullptr; }

bool CLIOptions::hasMutCount() { return mutCount.has_value(); }

bool CLIOptions::hasMinMutCount() { return minMutCount.has_value(); }

bool CLIOptions::hasMaxMutCount() { return maxMutCount.has_value(); }

bool CLIOptions::hasFormat() { return format.has_value(); }

[(<<<<< CELL 109-B >>>>>)]

[(<<<<< CELL 110-D >>>>>)]

[(<<<<< CELL 111-A >>>>>)]

const char *CLIOptions::getOutputFileName() { return (*outputFileName).c_str(); }

void CLIOptions::forceOverwrite() { overwriteOutputFile = true; }

std::string CLIOptions::getSeed() {
    if (!seedString.has_value()) {
        [(<<<<< CELL 116-C >>>>>)]
            readSeedFileIntoString(seedInput, &(seedString));
        }
        else {
            return std::string("");
        }

        if (CLIOptions::seedOutput != nullptr) {
        }
    }
    return seedString.value();
}

int32_t CLIOptions::getMutCount() { return mutCount.value(); }
int32_t CLIOptions::getMinMutCount() { return minMutCount.value(); }
[(<<<<< CELL 121-D >>>>>)]
Format CLIOptions::getFormat() { return format.value(); }

CLIOptions::~CLIOptions() {
    closeAndNullifyFileHandle(&(srcInput));
    closeAndNullifyFileHandle(&(tsvInput));
    [(<<<<< CELL 126-A >>>>>)]
    closeAndNullifyFileHandle(&(seedInput));
    [(<<<<< CELL 128-A >>>>>)]
}

void CLIOptions::addWarning(std::string str) { warnings.push_back(sanitizeOutputMessage(str)); }

[(<<<<< CELL 130-C >>>>>)]
    [(<<<<< CELL 28-A >>>>>)]
    [(<<<<< CELL 132-A >>>>>)]

    if (noMatchLines.size()) {
        os << "The pattern cell" << (noMatchLines.size() > 1 ? "s" : "") << " beginning at the"
           << (noMatchLines.size() > 1 ? "se" : "") << " following line number" << (noMatchLines.size() > 1 ? "s" : "")
           << " had no match" << (noMatchLines.size() > 1 ? "es" : "") << " in the source file: { ";
        for (auto i = noMatchLines.begin(); i < noMatchLines.end(); ++i) {
            [(<<<<< CELL 138-A >>>>>)]
        }
        os << "}\n   ";
    }
    if (multipleMatchLines.size()) {
        os << "The pattern cell" << (multipleMatchLines.size() > 1 ? "s" : "") << " beginning at the"
           [(<<<<< CELL 142-B >>>>>)]
           << (multipleMatchLines.size() > 1 ? "s" : "") << " had multiple matches in the source file: { ";
        for (auto i = multipleMatchLines.begin(); i < multipleMatchLines.end(); ++i) {
            os << *i << ((i + 1) == multipleMatchLines.end() ? " " : ", ");
        }
        os << "}\n";
    }
    if (warnings.size()) {
        for (const auto &str : warnings) { os << "   " << str << std::endl; }
        os << std::endl;
    }
    if ((os.str().size())) {
        std::string retVal{"\x1B[33mWarnings:\x1B[0m\n   "};
        retVal += os.str();
        return retVal;
    }
    return os.str();
}

void CLIOptions::addNoMatchLine(int n) { noMatchLines.push_back(n); }

void CLIOptions::addMultipleMatchLine(int n) { multipleMatchLines.push_back(n); }