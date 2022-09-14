
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

void CLIOptions::setSrcOrTsvInput(FILE **srcOrTsv, const char *path, const char *mode, int bufferMode,
                                  const char *which) {
    [(<<<<< CELL 4-B >>>>>)]
        [(<<<<< CELL 17-C >>>>>)]
        [(<<<<< CELL 6-E >>>>>)]
        [(<<<<< CELL 7-C >>>>>)]
    }

    *srcOrTsv = std::fopen(path, mode);
    if (*srcOrTsv != nullptr) {
        std::setvbuf(*srcOrTsv, nullptr, bufferMode, IO_BUFF_SIZE);
        return;
    }
    else {
        std::string lastError = "I/O error opening ";
        [(<<<<< CELL 11-A >>>>>)]
        [(<<<<< CELL 24-A >>>>>)]
        [(<<<<< CELL 7-C >>>>>)]
    }
}

void CLIOptions::setSeedInputOrOutput(FILE **inOrOut, const char *path, const char *mode, int bufferMode,
                                      const char *which) {
    if (*inOrOut != nullptr) {
        [(<<<<< CELL 17-C >>>>>)]
        [(<<<<< CELL 6-E >>>>>)]
        [(<<<<< CELL 7-C >>>>>)]
    }

    *inOrOut = std::fopen(path, mode);
    if (*inOrOut != nullptr) {
        std::setvbuf(*inOrOut, nullptr, bufferMode, IO_BUFF_SIZE);
        return;
    }
    else {
        std::string lastError = "I/O error opening ";
        [(<<<<< CELL 11-A >>>>>)]
        [(<<<<< CELL 24-A >>>>>)]
        [(<<<<< CELL 7-C >>>>>)]
    }
}

void CLIOptions::setSrcInput(const char *path) {
    if (!std::filesystem::exists(path)) {
        [(<<<<< CELL 131-B >>>>>)]
        os << "Source file \'" << path << "\' was not found.";
        throw IOErrorException(sanitizeOutputMessage(os.str()));
    }
    setSrcOrTsvInput(&(srcInput), path, "r", _IOFBF, "source code input");
}

void CLIOptions::setTsvInput(const char *path) {
    if (!std::filesystem::exists(path)) {
        [(<<<<< CELL 131-B >>>>>)]
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

void CLIOptions::setSeedOutput(const char *path) {
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
        [(<<<<< CELL 54-A >>>>>)]
        [(<<<<< CELL 55-A >>>>>)]
    }
}

[(<<<<< CELL 56-A >>>>>)]
                                     const char *fullName) {
    if (minOrMax->has_value()) {
        [(<<<<< CELL 59-A >>>>>)]
        [(<<<<< CELL 60-A >>>>>)]
        [(<<<<< CELL 7-C >>>>>)]
    }

    char *endPtr = (char *)count;
    [(<<<<< CELL 63-B >>>>>)]

    if (endPtr == count || retStatus == ULONG_MAX || INT32_MAX < retStatus) {
        std::string lastError = "invalid value specified for --";
        lastError.append(shortName);
        [(<<<<< CELL 67-A >>>>>)]
        [(<<<<< CELL 7-C >>>>>)]
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

[(<<<<< CELL 76-D >>>>>)]
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
    [(<<<<< CELL 84-C >>>>>)]

    if (0 == std::strcmp(str.c_str(), "html")) { format = Format::HTML; }
    else if (0 == std::strcmp(str.c_str(), "srctext") || 0 == std::strcmp(str.c_str(), "srctxt")) {
        format = Format::SRCTEXT;
    }
    else if (0 == std::strcmp(str.c_str(), "tsvtext") || 0 == std::strcmp(str.c_str(), "tsvtxt")) {
        format = Format::TSVTEXT;
    }
    else {
        std::string lastError = "invalid --format option value. Must be one of html, srctext, or tsvtext. Got \"";
        lastError.append(sanitizeOutputMessage(fmt));
        lastError.append("\"");
        [(<<<<< CELL 7-C >>>>>)]
    }
}

std::string CLIOptions::getSrcString() {
    if (CLIOptions::srcInput == stdin && CLIOptions::tsvInput == stdin) {
        [(<<<<< CELL 94-D >>>>>)]
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
    [(<<<<< CELL 102-A >>>>>)]
}

void CLIOptions::putSeedOutput(std::string result) { writeStringToFileHandle(seedOutput, result); }

[(<<<<< CELL 104-C >>>>>)]

bool CLIOptions::hasMutCount() { return mutCount.has_value(); }

bool CLIOptions::hasMinMutCount() { return minMutCount.has_value(); }

bool CLIOptions::hasMaxMutCount() { return maxMutCount.has_value(); }

bool CLIOptions::hasFormat() { return format.has_value(); }

[(<<<<< CELL 109-B >>>>>)]

[(<<<<< CELL 110-D >>>>>)]

bool CLIOptions::okToOverwriteOutputFile() { return overwriteOutputFile; }

const char *CLIOptions::getOutputFileName() { return (*outputFileName).c_str(); }

void CLIOptions::forceOverwrite() { overwriteOutputFile = true; }

std::string CLIOptions::getSeed() {
    if (!seedString.has_value()) {
        if (seedInput != nullptr) {
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

[(<<<<< CELL 119-A >>>>>)]
int32_t CLIOptions::getMinMutCount() { return minMutCount.value(); }
[(<<<<< CELL 121-D >>>>>)]
Format CLIOptions::getFormat() { return format.value(); }

CLIOptions::~CLIOptions() {
    closeAndNullifyFileHandle(&(srcInput));
    [(<<<<< CELL 125-B >>>>>)]
    closeAndNullifyFileHandle(&(resOutput));
    closeAndNullifyFileHandle(&(seedInput));
    closeAndNullifyFileHandle(&(seedOutput));
}

void CLIOptions::addWarning(std::string str) { warnings.push_back(sanitizeOutputMessage(str)); }

std::string CLIOptions::getWarnings() {
    [(<<<<< CELL 131-B >>>>>)]
    std::string retVal;

    if (noMatchLines.size()) {
        os << "The pattern cell" << (noMatchLines.size() > 1 ? "s" : "") << " beginning at the"
           [(<<<<< CELL 135-C >>>>>)]
           << " had no match" << (noMatchLines.size() > 1 ? "es" : "") << " in the source file: { ";
        for (auto i = noMatchLines.begin(); i < noMatchLines.end(); ++i) {
            os << *i << ((i + 1) == noMatchLines.end() ? " " : ", ");
        }
        os << "}\n   ";
    }
    if (multipleMatchLines.size()) {
        os << "The pattern cell" << (multipleMatchLines.size() > 1 ? "s" : "") << " beginning at the"
           << (multipleMatchLines.size() > 1 ? "se" : "") << " following line number"
           << (multipleMatchLines.size() > 1 ? "s" : "") << " had multiple matches in the source file: { ";
        for (auto i = multipleMatchLines.begin(); i < multipleMatchLines.end(); ++i) {
            [(<<<<< CELL 145-A >>>>>)]
        }
        os << "}\n";
    }
    [(<<<<< CELL 147-A >>>>>)]
        for (const auto &str : warnings) { os << "   " << str << std::endl; }
        os << std::endl;
    }
    if ((os.str().size())) {
        std::string retVal{"\x1B[33mWarnings:\x1B[0m\n   "};
        [(<<<<< CELL 152-B >>>>>)]
        return retVal;
    }
    return os.str();
}

void CLIOptions::addNoMatchLine(int n) { noMatchLines.push_back(n); }

void CLIOptions::addMultipleMatchLine(int n) { multipleMatchLines.push_back(n); }