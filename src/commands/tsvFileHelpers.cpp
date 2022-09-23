/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * tsvFileHelpers.cpp: TSV mutations file processing utilities
 *
 * - Helper functions for parsing and processing the TSV mutations file.
 * - Primarily used by MutationsRetriever class.
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

#include "commands/tsvFileHelpers.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <set>
#include <sstream>

#include "common.hpp"
#include "excepts.hpp"

std::string getPatternOrPermutation(std::string::iterator& it, const std::string::iterator& end, int& lineNumber,
                                    int rowBeginningLine) {
    std::string res = "";
    auto start = it;            // <- to calculate index if error
    int consecutiveQuotes = 0;  // <- to determine when a quoted cell has ended,
                                // i.e. if '\t' appears and this number is odd
    if (*it == '"') {
        ++it;
        while (it != end) {
            if (*it == '\n') {
                ++lineNumber;
                start = it + 1;
            }
            if (*it == '"') {
                ++consecutiveQuotes;
                if ((*(it + 1) == '\t' && (consecutiveQuotes % 2)) || (it + 1) == end) {
                    // end of quoted cell
                    ++it;
                    break;
                }
                else if (*(it + 1) == '"' && *(it + 2) != '\t') {
                    // escaped quote in quoted cell
                    ++it;
                    ++consecutiveQuotes;
                }
                else if (*(it + 1) != '\t' && (consecutiveQuotes % 2) && (it + 1) != end) {
                    // invalid character after end of quoted cell
                    int index = it + 2 - start;
                    throwInvalidCharException(it, end, index, lineNumber,
                                              rowBeginningLine);  // long enough to extract to own method
                }
            }
            else {
                consecutiveQuotes = 0;
            }
            res.push_back(*(it++));
        }
        if (it == end && !(consecutiveQuotes % 2)) {
            // final cell in row is missing terminating quote
            throwTerminatingQuoteException(rowBeginningLine);  // extracting to own method for consistency with above
        }
    }
    else
        while (it != end && *it != '\t') res.push_back(*(it++));
    return res;
}

void verifyHasPermutation(std::string::iterator it, const std::string::iterator& end, int& lineNumber,
                          int rowBeginningLine) {
    if (it == end || noPermutationsInLine(it, end)) {
        std::ostringstream os;
        os << " Error : Permutation cell missing in TSV File.\n"
           << "Notice :\n    Missing permutation cell on line number " << lineNumber
           << "\n    Row that begins with pattern cell on line number " << rowBeginningLine
           << " has no corresponding permutation cell(s)." << std::endl;
        throw TSVParsingException(os.str());
    }
}

// use of isWhiteSpace() ignores white space cells, changing back to accept
// white space cells for now if we add ignoring option, this method will be
// modified back
bool noPermutationsInLine(std::string::iterator it, const std::string::iterator& end) {
    // unsigned int bytes;
    // while (bytes = isWhiteSpace(it, end)) it += bytes;
    while (*it == '\t') ++it;
    return it == end;
}

void checkIndentation(std::string::iterator it, const std::string::iterator& end, int& lineNumber) {
    if (isWhiteSpace(it, end)) {
        std::ostringstream os;
        os << " Error : Indentation detected.\n"
           << "Notice :\n    Cells in TSV format should not be indented.\n"
           << "    Indentation found at row " << lineNumber << " of TSV File." << std::endl;
        throw TSVParsingException(os.str());
    }
}

void throwInvalidCharException(std::string::iterator it, const std::string::iterator& end, int index, int lineNumber,
                               int rowBeginningLine) {
    std::string c = "[ '0' ]";
    c[3] = *(it + 1);

    std::string invalidChar = isWhiteSpace((it + 1), end) ? "['SPACE']" : *(it + 1) == '"' ? "['QUOTATION MARK']" : c;
    std::ostringstream os;

    os << " Error : Invalid syntax found at index " << index << " of line number " << lineNumber << " in TSV\n"
       << "Notice :\n    Currently found in your TSV : ... \"" << invalidChar << "...\n"
       << "    Expected to be found in TSV : ... \"['TAB']...\n"
       << "\nIf index " << index - 1 << " is not intended end of quoted cell, "
       << "check preceding section of the row beginning with pattern cell on "
          "line number "
       << rowBeginningLine << "\nfor any extra or missing "
       << "QUOTATION MARKS and/or TABs as they are likely cause of error." << std::endl;
    throw TSVParsingException(os.str());
}

void throwTerminatingQuoteException(int lineNumber) {
    std::ostringstream os;
    os << " Error : Terminating quote missing.\n"
       << "Notice :\n    Cells beginning with QUOTATION MARK must end with "
       << "QUOTATION MARK.\n"
       << "    Final cell of row beginning on line number " << lineNumber << " missing terminating QUOTATION MARK."
       << std::endl;
    throw TSVParsingException(os.str());
}

void throwEmptyPatternException(int lineNumber) {
    std::ostringstream os;
    os << " Error : Cell content missing in TSV File.\n"
       << "Notice :\n    Missing cell content for pattern cell on line number " << lineNumber << std::endl;
    throw TSVParsingException(os.str());
}

void caseCaret(std::string::iterator patIt, PossibleMutVec::iterator& pmIt) {
    pmIt->data.depth = 2;

    while (((patIt + 1) != pmIt->pattern.end()) && *(++patIt) == '^') ++(pmIt->data.depth);

    if ((*patIt == '^' ? ++patIt : patIt) == pmIt->pattern.end()) { throwEmptyPatternException(pmIt->data.lineNumber); }
    if (*patIt == '@') { caseSynced(patIt, pmIt); }
    else if (pmIt->pattern.find_first_of("+?/!") == static_cast<size_t>(patIt - pmIt->pattern.begin())) {
        caseSpecialChars(patIt, pmIt);
    }
}

void caseSynced(std::string::iterator patIt, PossibleMutVec::iterator& pmIt) {
    pmIt->data.depth = pmIt->data.depth == 0 ? 2 : (pmIt->data.depth + 1);  // depth of non group leaders can never be 1
    pmIt->data.isIndexSynced = true;
    if (++patIt == pmIt->pattern.end()) { throwEmptyPatternException(pmIt->data.lineNumber); }
    if (pmIt->pattern.find_first_of("+?/!") == static_cast<size_t>(patIt - pmIt->pattern.begin())) {
        caseSpecialChars(patIt, pmIt);
    }
}

void caseSpecialChars(std::string::iterator patIt, PossibleMutVec::iterator& pmIt) {
    std::set<char> sChars{'+', '!', '?'};

    while (sChars.find(*patIt) != sChars.end() && (patIt) != pmIt->pattern.end()) {
        sChars.erase(*patIt);
        switch (*patIt) {
            case '+':
                pmIt->data.isNewLined = true;
                break;
            case '?':
                pmIt->data.isOptional = true;
                break;
            case '!':
                pmIt->data.mustPass = true;
                break;
        }
        ++patIt;
    }
    if ((patIt) != pmIt->pattern.end() && *patIt == '/') {
        pmIt->data.isRegex = true;
        ++patIt;
    }
    if (patIt == pmIt->pattern.end()) { throwEmptyPatternException(pmIt->data.lineNumber); }
}
