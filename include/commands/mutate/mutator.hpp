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
#include <set>
#include <string>
#include <tuple>
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

    bool isMultilineStringView(std::string_view sv) const;

    bool lineEdgesAreGood(std::string::iterator& begin, std::string::iterator& end, const std::string& str,
                          const std::string& subject);

    bool substringIsMatch(const std::string& subject, std::string::iterator it, const std::string& str);

    void checkPermutation(const SelectedMutation& sm, bool addIndentation, std::string& permutationString,
                          const std::string& indent);

    bool edgesGoodAndReplacementSuccessful(std::string::iterator& begin, std::string::iterator& end,
                                           const std::string& patternString, std::string& subject,
                                           const SelectedMutation& sm, std::string& permutationString, size_t& pos,
                                           size_t lengthToRemove, int& matches);

    void checkCountOfMatches(int matches, const SelectedMutation& sm);

    bool lines3AndOnAreGood(bool addIndentation, int indentation, const std::string& subject,
                            std::string::iterator& begin, std::string::iterator& end,
                            std::vector<std::string>::iterator& linesIt,
                            const std::vector<std::string>::iterator& vecEnd);

    bool wholeSublineOfMultilineIsMatch(bool addIndentation, int indentation, const std::string& subject,
                                        std::string::iterator& begin, std::string::iterator& end,
                                        const std::string& str);

    bool line2IsGood(const std::string& subject, std::string::iterator& end,
                     std::vector<std::string>::iterator& linesIt, int indentation, bool& addIndentation,
                     std::string::iterator& begin);

    std::tuple<std::string, std::string> getPatternAndModifiers(size_t index, const SelectedMutation& sm);

    std::set<std::string> getMatches(const std::string& pattern, const std::string& subject,
                                     const std::string& modifiers);

    std::vector<std::string> separateLinesIntoVector(std::string_view sv);

    std::string removeSrcStrComments();

   public:
    Mutator(std::string _sourceString, std::string& _outputString, SelectedMutVec _selectedMutations,
            CLIOptions* _opts);

    MutatedLinesMap mutatedLines;

    long mutatedLineCount{};
};

#endif  // _INCLUDED_MUTATOR_HPP_