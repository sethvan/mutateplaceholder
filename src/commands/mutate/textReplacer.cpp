/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * textReplacer.cpp: This class performs the actual text substitution to the source code string
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

#include "commands/mutate/textReplacer.hpp"

#include <sstream>

#include "commands/mutate/mutateDataStructures.hpp"
#include "common.hpp"
#include "excepts.hpp"

int TextReplacer::operator()( std::string& subject, const SelectedMutation& sm ) {
    if ( isMultilineString( sm.pattern ) ) {
        return multilineReplace( subject, sm );
    }
    else {
        return singleLineReplace( subject, sm );
    }
}

// adapted from https://stackoverflow.com/questions/4643512/replace-substring-with-another-substring-c/14678946#14678946
int TextReplacer::singleLineReplace( std::string& subject, const SelectedMutation& sm ) {
    matches = 0;
    pos = 0;

    while ( ( pos = subject.find( sm.pattern, pos ) ) != std::string::npos ) {
        begin = subject.begin() + pos;
        while ( *( begin - 1 ) != '\n' ) {
            --begin;
        }

        end = subject.begin() + pos;
        patternString = sm.pattern;
        lengthToRemove = sm.pattern.length();
        if ( !edgesGoodAndReplacementSuccessful( subject, sm ) ) {
            ++pos;
            continue;
        }
    }
    return matches;
}

// Does not consider consecutive newlines '\n' to mean multi line if they are at the beginning or end
bool TextReplacer::isMultilineString( const std::string& str ) const {
    auto it = str.begin();
    if ( ( it + 1 ) != str.end() ) {
        while ( ( it + 2 ) != str.end() ) {
            ++it;
            if ( ( *it == '\n' || *it == '\r' ) && ( *( it - 1 ) != '\n' ) && ( *( it - 1 ) != '\r' ) &&
                 ( *( it + 1 ) != '\n' ) && ( *( it + 1 ) != '\r' ) ) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::string> TextReplacer::separateLinesIntoVector( const std::string& str ) {
    std::istringstream is{ str };
    std::string line;
    std::vector<std::string> vec;

    while ( std::getline( is, line ) ) {
        vec.push_back( line + "\n" );
    }
    vec.back().pop_back();
    return vec;
}

int TextReplacer::multilineReplace( std::string& subject, const SelectedMutation& sm ) {
    matches = 0;
    pos = 0;
    std::vector<std::string> lines = separateLinesIntoVector( sm.pattern );

    while ( ( pos = subject.find( lines[0], pos ) ) != std::string::npos ) {
        begin = subject.begin() + pos;
        indentation = 0;
        while ( *( begin - 1 ) != '\n' ) {
            --begin;
            ++indentation;
        }

        lengthToRemove = sm.pattern.length();
        patternString = sm.pattern;
        end = subject.begin() + pos;
        startPos = subject.begin() + pos;
        if ( substringIsMatch( subject, startPos, patternString ) ) {
            if ( !edgesGoodAndReplacementSuccessful( subject, sm ) ) {
                ++pos;
            }
            continue;
        }
        std::string indent( begin, end );  // to use later if first check of second line does not match
        auto linesIt = lines.begin();

        if ( !lineEdgesAreGood( *linesIt, subject ) ) {
            ++pos;
            continue;
        }
        // At this point lines[0] has matched its line
        addIndentation = false;

        if ( !line2IsGood( subject, linesIt ) ) {
            ++pos;
            continue;
        }
        if ( !lines3AndOnAreGood( subject, linesIt, lines.end() ) ) {
            ++pos;
            continue;
        }
        lengthToRemove = end - startPos;

        setPermutationIndentation( sm, indent );

        ++matches;
        if ( sm.data.isNewLined ) {
            pos = end - subject.begin() + 1;
            permutationString.push_back( '\n' );
            lengthToRemove = 0;
        }
        subject.replace( pos, lengthToRemove, permutationString );
        pos += permutationString.length();
    }
    return matches;
}

bool TextReplacer::lineEdgesAreGood( const std::string& str, const std::string& subject ) {
    if ( lastNonWhiteSpace( begin, end ) != std::string::npos ) {
        return false;
    }
    begin = ( end += ( str.size() ) );
    if ( *( begin - 1 ) == '\n' ) {
        return true;
    }
    while ( ( *end != '\n' ) && ( end != subject.end() ) ) {
        ++end;
    }
    return ( lastNonWhiteSpace( begin, end ) == std::string::npos );
}

// good
bool TextReplacer::substringIsMatch( const std::string& subject, std::string::iterator it,
                                     const std::string& str ) const {
    if ( str.size() ) {
        for ( const auto& c : str ) {
            if ( it != subject.end() ) {
                if ( *it != c ) {
                    return false;
                }
                ++it;
            }
            else {
                return false;
            }
        }
    };
    return true;
}

// good
void TextReplacer::setPermutationIndentation( const SelectedMutation& sm, const std::string& indent ) {
    if ( sm.data.isNewLined ) {
        permutationString = indent;
    }
    if ( isMultilineString( sm.mutation ) && addIndentation ) {
        std::vector<std::string> permLines = separateLinesIntoVector( sm.mutation );
        permutationString += permLines[0];

        for ( size_t i = 1; i < permLines.size(); ++i ) {
            permutationString += indent + permLines[i];
        }
    }
    else {
        permutationString += std::string( sm.mutation );
    }
}

bool TextReplacer::edgesGoodAndReplacementSuccessful( std::string& subject, const SelectedMutation& sm ) {
    std::string indent( begin, end );
    if ( !lineEdgesAreGood( patternString, subject ) ) {
        return false;
    }
    addIndentation = !isMultilineString( sm.pattern );

    setPermutationIndentation( sm, indent );

    if ( sm.data.isNewLined ) {
        permutationString.push_back( '\n' );
        if ( end == subject.end() ) {
            subject.push_back( '\n' );
            end = subject.end() - 1;
        }
        pos = end - subject.begin() + 1;
        lengthToRemove = 0;
    }

    ++matches;
    subject.replace( pos, lengthToRemove, permutationString );
    pos += permutationString.length();
    return true;
}

bool TextReplacer::lines3AndOnAreGood( const std::string& subject, std::vector<std::string>::iterator& linesIt,
                                       const std::vector<std::string>::iterator& vecEnd ) {
    while ( ++linesIt != vecEnd ) {
        if ( !wholeSublineOfMultilineIsMatch( subject, *linesIt ) ) {
            return false;
        }
    }
    return true;
}

bool TextReplacer::wholeSublineOfMultilineIsMatch( const std::string& subject, const std::string& str ) {
    if ( addIndentation ) {
        end = begin + indentation;
    }
    if ( !substringIsMatch( subject, end, str ) ) {
        return false;
    }
    if ( addIndentation ) {
        end = begin + indentation;
    }
    if ( !lineEdgesAreGood( str, subject ) ) {
        return false;
    }
    return true;
}

bool TextReplacer::line2IsGood( const std::string& subject, std::vector<std::string>::iterator& linesIt ) {
    if ( !substringIsMatch( subject, end, *( ++linesIt ) ) ) {
        if ( indentation ) {
            addIndentation = true;  // try again but now indented
        }
        else {
            return false;
        }

        if ( !wholeSublineOfMultilineIsMatch( subject, *linesIt ) ) {
            return false;
        }
    }
    else if ( !lineEdgesAreGood( *linesIt, subject ) ) {
        return false;
    }
    return true;
}
