/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutationsRetriever.hpp: This class parses and validates the TSV input, capturing the possible mutations.
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

#ifndef _INCLUDED_MUTATIONSRETRIEVER_HPP_
#define _INCLUDED_MUTATIONSRETRIEVER_HPP_

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "commands/mutate/mutateDataStructures.hpp"

struct TSVRow {
    std::string row;
    int lineNumber;
};

class MutationsRetriever {
   private:
    std::istringstream tsvStream;

    PossibleMutVec possibleMutations;

    void capturePossibleMutations();

    void categorizeMutations();

    std::string getPatternOrPermutation(std::string::iterator& it, const std::string::iterator& end, int& lineNumber,
                                        int rowBeginningLine);

    static bool noPermutationsInLine(std::string::iterator it, const std::string::iterator& end);

    static void checkIndentation(std::string::iterator it, const std::string::iterator& end, int& lineNumber);

    static void verifyHasPermutation(std::string::iterator it, const std::string::iterator& end, int& lineNumber,
                                     int rowBeginningLine);

    static void throwInvalidCharException(std::string::iterator it, const std::string::iterator& end, int index,
                                          int lineNumber, int rowBeginningLine);

    static void throwTerminatingQuoteException(int lineNumber);

    static void throwEmptyPatternException(int lineNumber);

    static void caseCaret(std::string::iterator patIt, PossibleMutVec::iterator& pmIt);

    static void caseSynced(std::string::iterator patIt, PossibleMutVec::iterator& pmIt);

    static void caseSpecialChars(std::string::iterator patIt, PossibleMutVec::iterator& pmIt);

    std::vector<TSVRow> getRows();

    void checkNesting();

   public:
    MutationsRetriever(std::string tsvInput);

    PossibleMutVec& getPossibleMutations();
};

#endif  // _INCLUDED_MUTATIONSRETRIEVER_HPP_