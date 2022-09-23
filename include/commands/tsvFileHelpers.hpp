/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * tsvFileHelpers.hpp: TSV mutations file processing utilities
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

#ifndef _DEFINED_TSV_FILE_FUNCTIONS
#define _DEFINED_TSV_FILE_FUNCTIONS

#include <string>

#include "commands/mutate/mutateDataStructures.hpp"

std::string getPatternOrPermutation(std::string::iterator& it, const std::string::iterator& end, int& lineNumber,
                                    int rowBeginningLine);

bool noPermutationsInLine(std::string::iterator it, const std::string::iterator& end);

void checkIndentation(std::string::iterator it, const std::string::iterator& end, int& lineNumber);

void verifyHasPermutation(std::string::iterator it, const std::string::iterator& end, int& lineNumber,
                          int rowBeginningLine);

void throwInvalidCharException(std::string::iterator it, const std::string::iterator& end, int index, int lineNumber,
                               int rowBeginningLine);

void throwTerminatingQuoteException(int lineNumber);

void throwEmptyPatternException(int lineNumber);

void caseCaret(std::string::iterator patIt, PossibleMutVec::iterator& pmIt);

void caseSynced(std::string::iterator patIt, PossibleMutVec::iterator& pmIt);

void caseSpecialChars(std::string::iterator patIt, PossibleMutVec::iterator& pmIt);

#endif  // _DEFINED_TSV_FILE_FUNCTIONS
