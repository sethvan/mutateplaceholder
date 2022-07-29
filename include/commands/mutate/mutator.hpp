/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutator.hpp: This class mutates the source input using the selected mutations from the MutationsSelector class.
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

#ifndef _INCLUDED_MUTATOR_HPP_
#define _INCLUDED_MUTATOR_HPP_

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <unordered_map>

#include "../cli-options.hpp"
#include "commands/mutate/mutateDataStructures.hpp"
//#include "commands/mutate/jpcre2.hpp"

struct MutatedLine {
    int lineNumber;
    size_t lineIndex;  // where in line the pattern starts
    std::string_view mutation;

    MutatedLine(int _lineNumber, size_t _lineIndex, std::string_view _mutation)
        : lineNumber{_lineNumber}, lineIndex{_lineIndex}, mutation{_mutation} {}
};
using MutatedLinesMap = std::unordered_map<std::string_view, std::vector<MutatedLine>>;

class Mutator {  // Made class for now, may change

   private:
    std::string sourceString;

    std::string& outputString;

    SelectedMutVec selectedMutations;

    CLIOptions* opts;

    void mutate();

    void replaceStringInPlace(std::string& subject, const SelectedMutation& sm);

    void multilineReplace(std::string& subject, const SelectedMutation& sm);

    void regexReplace(std::string& subject, const SelectedMutation& sm);

    bool isMultilineString(std::string_view str) const;

    bool lineEdgesAreGood(std::string::iterator& begin, std::string::iterator& end, const std::string& str);

    bool substringIsMatch(const std::string& subject, std::string::iterator it, const std::string& str);

    bool wholeLineDoesMatch();

    std::vector<std::string> separateLinesIntoVector(std::string_view);

    std::string removeSrcStrComments();

   public:
    Mutator(std::string _sourceString, std::string& _outputString, SelectedMutVec _selectedMutations,
            CLIOptions* _opts);

    MutatedLinesMap mutatedLines;

    long mutatedLineCount{};
};

#endif  // _INCLUDED_MUTATOR_HPP_