/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutationsSelector.hpp: This class randomly selects from the possible mutations captured by the MutationsRetriever
 class.
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

#ifndef _INCLUDED_MUTATIONSSELECTOR_HPP_
#define _INCLUDED_MUTATIONSSELECTOR_HPP_

#include <optional>
#include <vector>

#include "../cli-options.hpp"
#include "chacharng/chacharng.hpp"
#include "chacharng/seedHelper.hpp"
#include "commands/mutate/mutateDataStructures.hpp"

class MutationsSelector {
   private:
    CLIOptions* opts;

    PossibleMutVec& possibleMutations;

    SelectedMutVec selectedMutations;

    SeedArray seedArray;

    State rng;

    int selectedMutCount;

    size_t pmVecSize;  // just to not calculate or retrieve more than once

    void selectPermutation(size_t index, PossibleMutVec::iterator& it);

    void setSeedArray();

    void setSelectedMutCount();

    void selectMutations();

    void groupedSelectPermutation(const std::vector<size_t>& indexes, size_t groupNumber, PossibleMutVec::iterator& it);

    void addAnythingElseNested(const std::vector<size_t>& indexes, size_t groupNumber, PossibleMutVec::iterator& it);

    void addNestedLine(const std::vector<size_t>& indexes, size_t groupNumber, PossibleMutVec::iterator& it);

    void addNewGroup(std::vector<size_t>& indexes, size_t& newGroupNumber, PossibleMutVec::iterator& leader);

    void sortOutNegatedLines(bool negatedTest);

    void selectIndexes();

   public:
    MutationsSelector(CLIOptions* _opts, PossibleMutVec& _possibleMutations);

    SelectedMutVec& getSelectedMutations();

    std::vector<size_t> selectedIndexes;  // Public for testing purposes
};

#endif  // _INCLUDED_MUTATIONSSELECTOR_HPP_