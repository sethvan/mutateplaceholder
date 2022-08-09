/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutationsSelector.cpp: This class randomly selects from the possible mutations captured by the MutationsRetriever
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

#define printDatos()                                                                             \
    std::cout << "lineNumber: " << it->data.lineNumber << ", depth: " << it->data.depth          \
              << ", synced: " << it->data.isIndexSynced << ", optional: " << it->data.isOptional \
              << ", newLined: " << it->data.isNewLined << ", regex: " << it->data.isRegex        \
              << ", groupNumber: " << it->data.groupNumber << ", negated: " << it->data.mustPass << std::endl

#include "commands/mutate/mutationsSelector.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <unordered_map>

#include "excepts.hpp"

#define OFFSET \
    ((it->data.depth ? it->data.depth - 1 : 0) + it->data.isOptional + it->data.isNewLined + it->data.mustPass + it->data.isRegex)

MutationsSelector::MutationsSelector(CLIOptions* _opts, PossibleMutVec& _possibleMutations)
    : opts(_opts), possibleMutations(_possibleMutations), pmVecSize{possibleMutations.size()} {}

SelectedMutVec& MutationsSelector::getSelectedMutations() {
    selectMutations();
    std::cout << selectedMutations.size() << " possible mutations have been selected" << std::endl;
    return selectedMutations;
}

void MutationsSelector::selectMutations() {
    if (!selectedIndexes.size())  // When testing, this vector will be passed in with test values
        selectIndexes();

    bool negatedTest = possibleMutations[selectedIndexes[0]].data.mustPass;

    // std::cout << "Size of PosMutVec: " << possibleMutations.size() << std::endl;

    std::vector<size_t> leaderIndexes{0};
    size_t newGroupNumber{0};
    for (const auto& i : selectedIndexes) {
        if (selectedMutations.size() < selectedIndexes.size()) {
            auto posMutVecIt = possibleMutations.begin() + i;
            if (posMutVecIt->data.groupNumber > 0) continue;
            if (!posMutVecIt->data.depth) {
                selectPermutation(nextRNGBetween(0, posMutVecIt->permutations.size(), rng), posMutVecIt);
            }
            else {
                size_t existingGroupNumber{0};
                auto leader = posMutVecIt;
                while (leader->data.depth != 1) --leader;
                if ((existingGroupNumber = leader->data.groupNumber) > 0) {
                    addNestedLine(leaderIndexes, existingGroupNumber, posMutVecIt);
                }
                else {
                    addNewGroup(leaderIndexes, newGroupNumber, leader);
                    if (leader != posMutVecIt && !posMutVecIt->data.groupNumber) {
                        addNestedLine(leaderIndexes, newGroupNumber, posMutVecIt);
                    }
                }
            }
        }
        else
            break;
    }
    sortOutNegatedLines(negatedTest);
    std::sort(selectedMutations.begin(), selectedMutations.end(),
              [](auto a, auto b) { return a.data.lineNumber > b.data.lineNumber; });
}

void MutationsSelector::setSeedArray() {
    std::string seedString;
    size_t seedStringLength{};

    if ((seedStringLength = (seedString = opts->getSeed()).length())) {
        if (seedStringLength < 64) {
            throw InvalidSeedException(" Error : Invalid input seed. Expected 64 hexadecimal digits");
        }
        if (!parseHexString(seedString.c_str(), seedArray.data(), SEED_SIZE_BYTES)) {
            throw InvalidSeedException(" Error : Seed being passed in is not valid hexidecimal number");
        }
        std::cout << "Using provided seed: " << seedString << std::endl;
    }
    else {
        seedArray = generateSeed();
        std::uint8_t hexSeedString[SEED_SIZE_BYTES * 2 + 1] = {0};
        if (!writeHexString((const char*)seedArray.data(), hexSeedString, SEED_SIZE_BYTES))
            throw InvalidSeedException(" Error: Failed to write out a string as hexadecimal");
        opts->setSeed((char*)hexSeedString);
        std::cout << "Using generated seed: " << hexSeedString << std::endl;
    }
}

void MutationsSelector::setSelectedMutCount() {
    if (opts->hasMutCount()) {
        selectedMutCount = std::min(opts->getMutCount(), static_cast<int>(pmVecSize));
        if (opts->getMutCount() > static_cast<int>(pmVecSize)) {
            std::ostringstream os;
            os << "--count=NUMBER entered exceeded possible amount contained in TSV, maximum available count of "
               << selectedMutCount << " from TSV was instead used.";
            opts->addWarning(os.str());
        }
    }
    else {
        int minMutCount = opts->hasMinMutCount() ? opts->getMinMutCount() : 1;
        int maxMutCount = opts->hasMaxMutCount() ? opts->getMaxMutCount() : pmVecSize + 1;
        selectedMutCount = nextRNGBetween(minMutCount, maxMutCount, rng);
        std::ostringstream os;
        os << selectedMutCount;
        opts->setMutCount(os.str().c_str());
    }
}

// This method also trims the string_view of the pattern cell
void MutationsSelector::selectPermutation(size_t index, PossibleMutVec::iterator& it) {
    index = index > (it->permutations.size() - 1) ? it->permutations.size() - 1
                                                  : index;  // for synced lines with less permutations than leader
    size_t offset = OFFSET, bytes, endPos;
    auto patIt = it->pattern.begin();
    auto end = it->pattern.end();
    while ((bytes = isWhiteSpace((patIt + offset), end))) offset += bytes;
    endPos = lastNonWhiteSpace(patIt, end);

    std::string_view pattern(it->pattern.c_str() + offset,
                             (endPos == std::string::npos ? it->pattern.size() : endPos + 1) - offset);
    std::string_view mutation = it->permutations[index];
    selectedMutations.emplace_back(pattern, mutation, it->data);
    // printDatos();
}

void MutationsSelector::groupedSelectPermutation(const std::vector<size_t>& indexes, size_t groupNumber,
                                                 PossibleMutVec::iterator& it) {
    it->data.groupNumber = groupNumber;
    if (it->data.isIndexSynced) { selectPermutation(indexes[groupNumber], it); }
    else {
        selectPermutation(nextRNGBetween(0, it->permutations.size(), rng), it);
    }
}

void MutationsSelector::addAnythingElseNested(const std::vector<size_t>& indexes, size_t groupNumber,
                                              PossibleMutVec::iterator& it) {
    auto upwardsIt = it;
    while (!((upwardsIt - 1)->data.groupNumber) && (upwardsIt - 1)->data.depth < upwardsIt->data.depth) {
        --upwardsIt;
        groupedSelectPermutation(indexes, groupNumber, upwardsIt);
    }
    while (!((it + 1)->data.groupNumber) && !((it + 1)->data.isOptional) && it + 1 != possibleMutations.end() &&
           (it + 1)->data.depth > it->data.depth) {
        ++it;
        groupedSelectPermutation(indexes, groupNumber, it);
    }
}

void MutationsSelector::addNewGroup(std::vector<size_t>& indexes, size_t& newGroupNumber,
                                    PossibleMutVec::iterator& leader) {
    leader->data.groupNumber = ++newGroupNumber;
    size_t leaderIndex = nextRNGBetween(0, leader->permutations.size(), rng);
    indexes.push_back(leaderIndex);
    selectPermutation(leaderIndex, leader);

    auto posMutVecIt = leader;

    bool okToAdd = true;
    while ((posMutVecIt + 1)->data.depth > 1 && posMutVecIt + 1 != possibleMutations.end()) {
        ++posMutVecIt;
        if (posMutVecIt->data.depth == 2) okToAdd = true;
        if (posMutVecIt->data.isOptional) okToAdd = false;
        if (okToAdd) { groupedSelectPermutation(indexes, newGroupNumber, posMutVecIt); }
    }
}

void MutationsSelector::selectIndexes() {
    setSeedArray();
    rng = State(seedArray.data());
    setSelectedMutCount();  // reminder: rng needs to be constructed with seed for this method to work

    std::set<size_t> set;
    while (set.size() < static_cast<size_t>(selectedMutCount)) {
        set.insert(nextRNGBetween(0, static_cast<std::uint32_t>(pmVecSize), rng));
    }
    for (const auto& i : set) { selectedIndexes.push_back(i); }
}

void MutationsSelector::addNestedLine(const std::vector<size_t>& indexes, size_t groupNumber,
                                      PossibleMutVec::iterator& it) {
    groupedSelectPermutation(indexes, groupNumber, it);
    addAnythingElseNested(indexes, groupNumber, it);
}

void MutationsSelector::sortOutNegatedLines(bool negatedTest) {
    SelectedMutVec temp;
    if (negatedTest) {
        std::for_each(selectedMutations.begin(), selectedMutations.end(), [&](SelectedMutation& sm) {
            if (sm.data.mustPass) { temp.push_back(sm); }
        });
    }
    else {
        std::for_each(selectedMutations.begin(), selectedMutations.end(), [&](SelectedMutation& sm) {
            if (!sm.data.mustPass) { temp.push_back(sm); }
        });
    }
    selectedMutations = temp;
}