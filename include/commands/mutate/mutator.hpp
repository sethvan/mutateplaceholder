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
#include "commands/mutate/textReplacer.hpp"

class Mutator {  // Made class for now, may change

   private:
    std::string sourceString;

    std::string& outputString;

    SelectedMutVec selectedMutations;

    CLIOptions* opts;

    TextReplacer replacer;

    void mutate();

    void regexReplace( std::string& subject, const SelectedMutation& sm );

    std::set<std::string> getRegexMatches( const std::string& pattern, const std::string& subject,
                                           const std::string& modifiers );

    std::tuple<std::string, std::string> getPatternAndModifiers( size_t index, const SelectedMutation& sm );

    std::string removeSrcStrComments();

    void checkCountOfMatches( int matches, const SelectedMutation& sm );

   public:
    Mutator( std::string _sourceString, std::string& _outputString, SelectedMutVec _selectedMutations,
             CLIOptions* _opts );

    // long mutatedLineCount{};
};

#endif  // _INCLUDED_MUTATOR_HPP_
