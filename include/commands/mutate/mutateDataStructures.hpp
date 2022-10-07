#ifndef _INCLUDED_MUTATEDATASTRUCTURES_HPP_
#define _INCLUDED_MUTATEDATASTRUCTURES_HPP_

/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutateDataStructures.hpp: Contains data structures common to at least 2 of the mutate classes (MutationsRetriever,
 MutationsSelector, Mutator).
 *  - Since this is not a huge project, putting them here in the same header for convenience as each is used in more
 than one class.
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

#include <string>
#include <vector>

struct SelectedLineInfo {
    bool isRegex : 1;
    bool isNewLined : 1;
    bool isIndexSynced : 1;
    bool isOptional : 1;
    bool mustPass : 1;
    bool : 0;
    size_t depth;  // depth 0 = normal, 1 = group leader, 2 = one '^' or one '@', 3 = ^^ or ^@, etc..
    size_t groupNumber;
    size_t lineNumber;

    SelectedLineInfo()
        : isRegex{ 0 },
          isNewLined{ 0 },
          isIndexSynced{ 0 },
          isOptional{ 0 },
          mustPass{ 0 },
          depth{ 0 },
          groupNumber{ 0 },
          lineNumber{ 0 } {}
};

struct TsvFileLine {
    std::string pattern;
    std::vector<std::string> permutations;
    SelectedLineInfo data;

    TsvFileLine( std::string _pattern, std::vector<std::string> _permutations = std::vector<std::string>{} )
        : pattern{ _pattern }, permutations{ _permutations } {}
};
using PossibleMutVec = std::vector<TsvFileLine>;

struct SelectedMutation {
    std::string pattern;
    std::string replacement;
    SelectedLineInfo data;

    SelectedMutation( std::string _pattern, std::string _replacement, SelectedLineInfo info )
        : pattern{ _pattern }, replacement{ _replacement }, data{ info } {}
};
using SelectedMutVec = std::vector<SelectedMutation>;

#endif  // _INCLUDED_MUTATEDATASTRUCTURES_HPP_