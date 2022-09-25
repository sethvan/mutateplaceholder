/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * textReplacer.hpp: This class performs the actual text substitution to the cource code string.
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

#ifndef _INCLUDED_TEXTREPLACER_HPP
#define _INCLUDED_TEXTREPLACER_HPP

#include <string>

#include "commands/mutate/mutateDataStructures.hpp"

class TextReplacer {
    int matches;

    size_t pos;

    size_t lengthToRemove;

    std::string::iterator begin;

    std::string::iterator end;

    std::string::iterator startPos;

    std::string permutationString;

    std::string patternString;

    int indentation;

    bool addIndentation;

    int singleLineReplace( std::string& subject, const SelectedMutation& sm );

    int multilineReplace( std::string& subject, const SelectedMutation& sm );

    bool isMultilineString( const std::string& str ) const;

    bool lineEdgesAreGood( const std::string& str, const std::string& subject );

    bool substringIsMatch( const std::string& subject, std::string::iterator it, const std::string& str ) const;

    void setPermutationIndentation( const SelectedMutation& sm, const std::string& indent );

    bool edgesGoodAndReplacementSuccessful( std::string& subject, const SelectedMutation& sm );

    bool lines3AndOnAreGood( const std::string& subject, std::vector<std::string>::iterator& linesIt,
                             const std::vector<std::string>::iterator& vecEnd );

    bool wholeSublineOfMultilineIsMatch( const std::string& subject, const std::string& str );

    bool line2IsGood( const std::string& subject, std::vector<std::string>::iterator& linesIt );

    std::vector<std::string> separateLinesIntoVector( const std::string& str );

   public:
    TextReplacer() = default;
    int operator()( std::string& subject, const SelectedMutation& sm );
};

#endif  // _INCLUDED_TEXTREPLACER_HPP
