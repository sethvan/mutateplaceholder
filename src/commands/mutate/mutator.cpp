/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutator.cpp: This class mutates the source input using the selected mutations from the MutationsSelector class.
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

#include "commands/mutate/mutator.hpp"

#include <algorithm>
#include <cassert>
#include <set>
#include <sstream>
#include <string_view>

#include "commands/mutate/jpcre2.hpp"
#include "common.hpp"
#include "excepts.hpp"

typedef jpcre2::select<char> jp;

Mutator::Mutator( std::string _sourceString, std::string& _outputString, SelectedMutVec _selectedMutations,
                  CLIOptions* _opts )
    : sourceString{ _sourceString },
      outputString{ _outputString },
      selectedMutations{ _selectedMutations },
      opts{ _opts } {
    mutate();
}

void Mutator::mutate() {
    std::string strippedStr = removeSrcStrComments();
    for ( const auto& sm : selectedMutations ) {
        if ( sm.data.isRegex ) {
            regexReplace( strippedStr, sm );
        }
        else {
            int matches = replacer( strippedStr, sm );
            checkCountOfMatches( matches, sm );
        }
    }
    outputString = strippedStr;
}

void Mutator::regexReplace( std::string& subject, const SelectedMutation& sm ) {
    size_t index = sm.pattern.find_last_of( '/' );
    if ( index == std::string::npos ) {
        std::ostringstream os;
        os << "Regex pattern cell in row beginning on line number " << sm.data.lineNumber << " is missing final \'/\'."
           << std::endl;
        throw TSVParsingException( os.str() );
    }

    auto [pattern, modifiers] = getPatternAndModifiers( index, sm );
    std::set<std::string> matches = getRegexMatches( pattern, subject, modifiers );

    for ( const auto& str : matches ) {
        std::string regexMutation = jp::Regex( pattern ).replace( str, sm.mutation, modifiers );
        SelectedMutation regexSm( str, regexMutation, sm.data );
        if ( regexSm.pattern.size() ) {
            int matches = replacer( subject, regexSm );
            checkCountOfMatches( matches, sm );
        }
    }
}

// This is just a temporary stand in method to use until we have better regex patterns
// So that we can continue developing meanwhile
std::string Mutator::removeSrcStrComments() {
    std::string subject = jp::Regex( "\\/\\*.*\\*\\/" ).replace( sourceString, "", "gm" );
    subject = jp::Regex( ";.*?\\/\\/[^\"\n]*\n" ).replace( subject, ";\n", "gm" );
    subject = jp::Regex( "({\\s*?\\/\\/[^\"\n]*\n)" ).replace( subject, "{\n", "gm" );
    subject = jp::Regex( "()\\s*?\\/\\/[^\"\n]*\n)" ).replace( subject, ")\n", "gm" );
    subject = jp::Regex( "\n\\s*?\\/\\/.*\n" ).replace( subject, "\n", "gm" );
    return subject;
}

void Mutator::checkCountOfMatches( int matches, const SelectedMutation& sm ) {
    if ( !matches ) {
        opts->addNoMatchLine( sm.data.lineNumber );
    }
    if ( matches > 1 ) {
        opts->addMultipleMatchLine( sm.data.lineNumber );
    }
}

std::tuple<std::string, std::string> Mutator::getPatternAndModifiers( size_t index, const SelectedMutation& sm ) {
    std::string defaultFlags( "AFgnm" );
    std::string pattern{ sm.pattern, 0, index };
    std::string userModifierInput( sm.pattern.begin() + index + 1, sm.pattern.end() );
    std::string modifiers;
    index = userModifierInput.find_first_of( '-' );
    if ( index != std::string::npos ) {
        std::string additionalModFlags( userModifierInput, 0, index );
        std::string flagsToBeRemoved( userModifierInput.begin() + index + 1, userModifierInput.end() );
        if ( additionalModFlags.size() ) {
            modifiers.append( additionalModFlags );
        }
        for ( const auto& c : defaultFlags ) {
            if ( ( index = flagsToBeRemoved.find_first_of( c ) ) == std::string::npos ) {
                modifiers.push_back( c );
            }
        }
    }
    else {
        modifiers = userModifierInput + defaultFlags;
    }
    return { pattern, modifiers };
}

std::set<std::string> Mutator::getRegexMatches( const std::string& pattern, const std::string& subject,
                                                const std::string& modifiers ) {
    jp::VecNum vec_num;
    jp::Regex re( pattern );
    jp::RegexMatch rr;
    rr.setRegexObject( &re )
        .setSubject( &subject )
        .addModifier( modifiers )
        .setNumberedSubstringVector( &vec_num )
        .match();

    std::set<std::string> strSet;
    for ( auto& vec : vec_num ) {
        for ( auto& n : vec ) {
            strSet.insert( n );
        }
    }

    return strSet;
}
