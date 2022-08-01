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
#include <sstream>

#include "commands/mutate/jpcre2.hpp"
#include "common.hpp"
#include "excepts.hpp"

#define INCREMENT_POS_AND_CONTINUE \
    {                              \
        ++pos;                     \
        continue;                  \
    }

typedef jpcre2::select<char> jp;

Mutator::Mutator(std::string _sourceString, std::string& _outputString, SelectedMutVec _selectedMutations,
                 CLIOptions* _opts)
    : sourceString{_sourceString}, outputString{_outputString}, selectedMutations{_selectedMutations}, opts{_opts} {
    mutate();
}

void Mutator::mutate() {
    std::string strippedStr = removeSrcStrComments();
    for (const auto& sm : selectedMutations) {
        if (sm.data.isRegex) {
            // regexReplace(sourceString, sm);
            std::cout << "Is regex, ";
        }
        else if (isMultilineStringView(sm.pattern)) {
            multilineReplace(strippedStr, sm);
        }
        else {
            replaceStringInPlace(strippedStr, sm);
        }
    }
    outputString = strippedStr;
}

// adapted from https://stackoverflow.com/questions/4643512/replace-substring-with-another-substring-c/14678946#14678946
void Mutator::replaceStringInPlace(std::string& subject, const SelectedMutation& sm) {
    int matches = 0;
    size_t pos = 0;
    std::string::iterator begin, end;

    while ((pos = subject.find(sm.pattern, pos)) != std::string::npos) {
        begin = end = subject.begin() + pos;
        std::string patternString{sm.pattern};
        std::string permutationString;
        size_t lengthToRemove = sm.pattern.length();

        while (*(begin - 1) != '\n') --begin;

        if (!checkEdgesAndReplaceSuccessful(begin, end, patternString, subject, sm, permutationString, pos,
                                            lengthToRemove, matches))
            INCREMENT_POS_AND_CONTINUE;
    }
    checkCountOfMatches(matches, sm);
}

// void Mutator::regexReplace(std::string& subject, const SelectedMutation& sm) {
// 	size_t index = sm.mutation.find_first_of('/');
// 	std::string pattern{sm.pattern, 0, index};
// 	std::string modifiers{sm.pattern, index+1, sm.pattern.size()-1};
// 	subject = jp::Regex(pattern).replace(subject, sm.mutation.data(), modifiers);
// }

// This is just a temporary stand in method to use until we have better regex patterns
// So that we can continue developing meanwhile
std::string Mutator::removeSrcStrComments() {
    std::string subject = jp::Regex("\\/\\*.*\\*\\/").replace(sourceString, "", "gm");
    subject = jp::Regex(";.*?\\/\\/[^\"\n]*\n").replace(subject, ";\n", "gm");
    subject = jp::Regex("({\\s*?\\/\\/[^\"\n]*\n)").replace(subject, "{\n", "gm");
    subject = jp::Regex("()\\s*?\\/\\/[^\"\n]*\n)").replace(subject, ")\n", "gm");
    subject = jp::Regex("\n\\s*?\\/\\/.*\n").replace(subject, "\n", "gm");
    return subject;
}

// Does not consider consecutive newlines '\n' to mean multi line if they are at the beginning or end
bool Mutator::isMultilineStringView(std::string_view sv) const {
    auto it = sv.begin();
    if ((it + 1) != sv.end()) {
        while ((it + 2) != sv.end()) {
            ++it;
            if ((*it == '\n' || *it == '\n') && (*(it - 1) != '\n') && (*(it - 1) != '\r') && (*(it + 1) != '\n') &&
                (*(it + 1) != '\r'))
                return true;
        }
    }
    return false;
}

std::vector<std::string> Mutator::separateLinesIntoVector(std::string_view sv) {
    std::istringstream is{sv.data()};
    std::string line;
    std::vector<std::string> vec;

    while (std::getline(is, line)) { vec.push_back(line + "\n"); }
    vec.back().pop_back();
    return vec;
}

void Mutator::multilineReplace(std::string& subject, const SelectedMutation& sm) {
    int matches = 0, indentation = 0;
    size_t pos = 0;
    std::string::iterator begin, end, startPos;
    std::vector<std::string> lines = separateLinesIntoVector(sm.pattern);

    while ((pos = subject.find(lines[0], pos)) != std::string::npos) {
        begin = end = startPos = subject.begin() + pos;
        indentation = 0;
        while (*(begin - 1) != '\n') {
            --begin;
            ++indentation;
        }
        size_t lengthToRemove = sm.pattern.length();
        std::string patternString{sm.pattern};
        std::string permutationString;

        if (substringIsMatch(subject, startPos, patternString)) {
            if (!checkEdgesAndReplaceSuccessful(begin, end, patternString, subject, sm, permutationString, pos,
                                                lengthToRemove, matches))
                ++pos;
            continue;
        }
        std::string indent(begin, end);  // to use later if first check of second line does not match
        auto linesIt = lines.begin();

        if (!lineEdgesAreGood(begin, end, *linesIt)) INCREMENT_POS_AND_CONTINUE;
        // At this point lines[0] has matched its line
        bool addIndentation = false;

        if (!line2IsGood(subject, end, linesIt, indentation, addIndentation, begin)) INCREMENT_POS_AND_CONTINUE;
        if (!lines3AndOnAreGood(addIndentation, indentation, subject, begin, end, linesIt, lines.end()))
            INCREMENT_POS_AND_CONTINUE;
        lengthToRemove = end - startPos;

        checkPermutation(sm, addIndentation, permutationString, indent);

        ++matches;
        if (sm.data.isNewLined) {
            pos = end - subject.begin() + 1;
            permutationString.push_back('\n');
            lengthToRemove = 0;
        }
        subject.replace(pos, lengthToRemove, permutationString);
        pos += permutationString.length();
    }
    checkCountOfMatches(matches, sm);
}

bool Mutator::lineEdgesAreGood(std::string::iterator& begin, std::string::iterator& end, const std::string& str) {
    if (lastNonWhiteSpace(begin, end) != std::string::npos) {
        // printf("found mismatch\n");
        return false;
    }
    begin = (end += (str.size()));
    if (*(begin - 1) == '\n') return true;
    while (*end != '\n') ++end;
    return (lastNonWhiteSpace(begin, end) == std::string::npos);
}

bool Mutator::substringIsMatch(const std::string& subject, std::string::iterator it, const std::string& str) {
    if (str.size()) {
        for (const auto& c : str) {
            if (it != subject.end()) {
                if (*it != c) return false;
                ++it;
            }
            else
                return false;
        }
    };
    return true;
}

void Mutator::checkPermutation(const SelectedMutation& sm, bool addIndentation, std::string& permutationString,
                               const std::string& indent) {
    if (isMultilineStringView(sm.mutation) && addIndentation) {
        std::vector<std::string> permLines = separateLinesIntoVector(sm.mutation);
        if (sm.data.isNewLined)
            permutationString += indent + permLines[0];
        else
            permutationString = permLines[0];
        for (size_t i{}; i < permLines.size(); ++i) {
            if (i != 0) permutationString += indent + permLines[i];
        }
    }
    else {
        if (sm.data.isNewLined)
            permutationString += indent + std::string(sm.mutation);
        else
            permutationString = std::string(sm.mutation);
    }
}

bool Mutator::checkEdgesAndReplaceSuccessful(std::string::iterator& begin, std::string::iterator& end,
                                             const std::string& patternString, std::string& subject,
                                             const SelectedMutation& sm, std::string& permutationString, size_t& pos,
                                             size_t lengthToRemove, int& matches) {
    if (!lineEdgesAreGood(begin, end, patternString)) return false;
    if (sm.data.isNewLined) {
        std::string indent(begin, end);
        permutationString += indent + sm.mutation.data();
        pos = end - subject.begin() + 1;
        lengthToRemove = 0;
    }
    else
        permutationString = sm.mutation.data();

    permutationString.push_back('\n');
    ++matches;
    subject.replace(pos, lengthToRemove, permutationString);
    pos += permutationString.length();
    return true;
}

void Mutator::checkCountOfMatches(int matches, const SelectedMutation& sm) {
    if (!matches) opts->addNoMatchLine(sm.data.lineNumber);
    if (matches > 1) opts->addMultipleMatchLine(sm.data.lineNumber);
}

bool Mutator::lines3AndOnAreGood(bool addIndentation, int indentation, const std::string& subject,
                                 std::string::iterator& begin, std::string::iterator& end,
                                 std::vector<std::string>::iterator& linesIt,
                                 const std::vector<std::string>::iterator& vecEnd) {
    while (++linesIt != vecEnd) {
        if (!wholeSublineOfMultilineIsMatch(addIndentation, indentation, subject, begin, end, *linesIt)) return false;
    }
    return true;
}

bool Mutator::wholeSublineOfMultilineIsMatch(bool addIndentation, int indentation, const std::string& subject,
                                             std::string::iterator& begin, std::string::iterator& end,
                                             const std::string& str) {
    if (addIndentation) end = begin + indentation;
    if (!substringIsMatch(subject, end, str)) return false;
    if (addIndentation) end = begin + indentation;
    if (!lineEdgesAreGood(begin, end, str)) return false;
    return true;
}

bool Mutator::line2IsGood(const std::string& subject, std::string::iterator& end,
                          std::vector<std::string>::iterator& linesIt, int indentation, bool& addIndentation,
                          std::string::iterator& begin) {
    if (!substringIsMatch(subject, end, *(++linesIt))) {
        if (indentation)
            addIndentation = true;  // try again but now indented
        else
            return false;

        if (!wholeSublineOfMultilineIsMatch(addIndentation, indentation, subject, begin, end, *linesIt)) return false;
    }
    else if (!lineEdgesAreGood(begin, end, *linesIt))
        return false;
    return true;
}