/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutationsRetriever.cpp: This class parses and validates the TSV input,
 *  capturing the possible mutations.
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

#include "commands/mutate/mutationsRetriever.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <set>

#include "commands/tsvFileHelpers.hpp"
#include "common.hpp"
#include "excepts.hpp"

MutationsRetriever::MutationsRetriever( std::string tsvInput ) : tsvStream{ tsvInput } {}

void MutationsRetriever::capturePossibleMutations() {
    std::vector<TSVRow> rows = getRows();
    auto rowsIt = rows.begin();
    std::string line;

    do {
        line = rowsIt->row;
        std::string::iterator lineIt = line.begin();
        int lineNumber = rowsIt->lineNumber;

        checkIndentation( lineIt, line.end(), lineNumber );

        std::string pattern = getPatternOrPermutation( lineIt, line.end(), lineNumber, rowsIt->lineNumber );
        possibleMutations.push_back( std::move( pattern ) );

        verifyHasPermutation( lineIt, line.end(), lineNumber, rowsIt->lineNumber );

        while ( lineIt != line.end() ) {
            while ( *lineIt == '\t' )
                ++lineIt;  // will later have option to disable ignoring of white space
                           // cells
            std::string permutation = getPatternOrPermutation( lineIt, line.end(), lineNumber, rowsIt->lineNumber );
            possibleMutations.back().permutations.push_back( std::move( permutation ) );
        }
        possibleMutations.back().data.lineNumber = rowsIt->lineNumber;
    } while ( ++rowsIt != rows.end() );
}

void MutationsRetriever::categorizeMutations() {
    auto pmIt = possibleMutations.begin();
    while ( pmIt != possibleMutations.end() ) {
        if ( 0 == pmIt->pattern.find_first_not_of( "^@+/!" ) &&
             ( ( pmIt + 1 ) != possibleMutations.end() ? ( 0 == ( pmIt + 1 )->pattern.find_first_not_of( "^@" ) )
                                                       : true ) ) {
            ++pmIt;
            continue;
        }
        if ( 0 == pmIt->pattern.find_first_not_of( "^@" ) && ( pmIt + 1 ) != possibleMutations.end() &&
             0 == ( pmIt + 1 )->pattern.find_first_of( "^@" ) &&
             1 == ( pmIt + 1 )->pattern.find_first_not_of( "^@" ) ) {
            pmIt->data.depth = 1;  // group leader
        }
        if ( 0 == pmIt->pattern.find_first_of( "^@/+!" ) ) {
            auto patIt = pmIt->pattern.begin();
            switch ( *patIt ) {
                case '^':
                    caseCaret( patIt, pmIt );
                    break;
                case '@':
                    caseSynced( patIt, pmIt );
                    break;
                default:
                    caseSpecialChars( patIt, pmIt );
                    break;
            }
        }
        ++pmIt;
    }
}

PossibleMutVec& MutationsRetriever::getPossibleMutations() {
    capturePossibleMutations();
    categorizeMutations();
    checkNesting();
    return possibleMutations;
}

void MutationsRetriever::checkNesting() {
    assert( possibleMutations.size() );
    auto it = possibleMutations.begin();
    auto throwInvalidNesting = [&]() {
        std::ostringstream os;
        os << " Error : Invalid group nesting syntax in TSV File.\n"
           << "Notice :\n     Nested pattern cell in row number "
           << ( it == possibleMutations.begin() ? it : it + 1 )->data.lineNumber << " has no corresponding parent."
           << std::endl;
        throw TSVParsingException( os.str() );
    };
    if ( it->data.depth > 1 )
        throwInvalidNesting();
    while ( ( it + 1 ) != possibleMutations.end() ) {
        if ( ( ( it->data.depth < ( it + 1 )->data.depth ) && ( ( ( it + 1 )->data.depth - it->data.depth ) > 1 ) ) ||
             ( ( ( it + 1 )->data.depth > 2 ) && ( it + 1 )->data.depth <= it->data.depth ) ) {
            throwInvalidNesting();
        }
        ++it;
    }
}

std::vector<TSVRow> MutationsRetriever::getRows() {
    std::vector<TSVRow> temp;
    temp.push_back( { "", 1 } );
    char c, last;
    int QMarkCount = 0,
        lineNumber = 1;  // QMarks are quotation marks not question marks
    bool countTheQMarks = true;

    tsvStream.get( c );
    if ( ( last = c ) == '\n' ) {  // in case first line is empty
        ++lineNumber;
    }
    else {
        if ( c == '"' ) {
            ++QMarkCount;
        }
        else {
            countTheQMarks = false;
        }
        temp.back().row.push_back( c );
    }

    while ( tsvStream.get( c ) ) {
        if ( c == '\t' && !( QMarkCount % 2 ) && countTheQMarks ) {
            QMarkCount = 0;
            countTheQMarks = false;
        }
        if ( c == '"' ) {
            if ( !countTheQMarks ) {
                if ( !temp.back().row.size() || last == '\t' ) {
                    ++QMarkCount;
                    countTheQMarks = true;
                }
            }
            else
                ++QMarkCount;
        }

        if ( c == '\n' ) {
            ++lineNumber;
            if ( last == '\n' && !( QMarkCount % 2 ) )
                continue;
            if ( ( last != '\n' && !( QMarkCount % 2 ) ) || temp.back().row[0] == '#' ) {
                temp.push_back( { "", lineNumber } );
                QMarkCount = 0;
                last = c;
                continue;
            }
        }
        temp.back().row.push_back( c );
        last = c;
    }
    if ( !temp.back().row.size() )
        temp.pop_back();

    std::vector<TSVRow> rows;
    rows.reserve( temp.size() );
    std::for_each( temp.begin(), temp.end(), [&]( TSVRow& Row ) {
        if ( Row.row[0] != '#' )
            rows.push_back( Row );
    } );

    if ( !rows.size() ) {
        throw TSVParsingException( "No mutations found in TSV file." );
    }

    return rows;
}
