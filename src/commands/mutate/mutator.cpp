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
        else if (isMultilineString(sm.pattern)) {
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
    while ((pos = subject.find(sm.pattern, pos)) != std::string::npos) {
        ++matches;
        subject.replace(pos, sm.pattern.length(), sm.mutation);
        pos += sm.mutation.length();
    }
    if (!matches) { opts->addNoMatchLine(sm.data.lineNumber); }
    else if (matches > 1) {
        opts->addMultipleMatchLine(sm.data.lineNumber);
    }
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
bool Mutator::isMultilineString(std::string_view str) const {
    auto it = str.begin();
    if ((it + 1) != str.end()) {
        while ((it + 2) != str.end()) {
            ++it;
            if ((*it == '\n' || *it == '\n') && (*(it - 1) != '\n') && (*(it - 1) != '\r') && (*(it + 1) != '\n') &&
                (*(it + 1) != '\r'))
                return true;
        }
    }
    return false;
}

std::vector<std::string> Mutator::separateLinesIntoVector(std::string_view str) {
    std::istringstream is{std::string(str)};
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
    bool addIndentation = false;
    std::vector<std::string> lines = separateLinesIntoVector(sm.pattern);

    auto wholeLineDoesMatch = [&](const std::string& str) -> bool {
        if (addIndentation) end = begin + indentation;
        if (!(substringIsMatch(subject, end, str))) return false;
        if (addIndentation) end = begin + indentation;
        if (!(lineEdgesAreGood(begin, end, str))) return false;
        return true;
    };

    while ((pos = subject.find(lines[0], pos)) != std::string::npos) {
        begin = end = startPos = subject.begin() + pos;
        indentation = 0;
        while (*(begin - 1) != '\n') {
            --begin;
            ++indentation;
        }

        if (substringIsMatch(subject, startPos, std::string(sm.pattern))) {
            if (lastNonWhiteSpace(begin, end) != std::string::npos) {
                ++pos;
                continue;
            }
            ++matches;
            subject.replace(pos, sm.pattern.length(), sm.mutation);
            pos += sm.mutation.length();
            continue;
        }

        std::string indent(begin, end);  // to use later if first check of second line does not match
        auto linesIt = lines.begin();

        if (!(lineEdgesAreGood(begin, end, *linesIt))) {
            ++pos;
            continue;
        }

        // At this point lines[0] has matched its line
        if (!(substringIsMatch(subject, end, *(++linesIt)))) {
            if (indentation)
                addIndentation = true;  // try again but now indented
            else {
                ++pos;
                continue;
            }
            if (!(wholeLineDoesMatch(*linesIt))) {
                ++pos;
                continue;
            }
        }
        else if (!(lineEdgesAreGood(begin, end, *linesIt))) {
            ++pos;
            continue;
        }

        // At this point lines[1] has matched its line
        bool doesNotMach = false;
        while (++linesIt != lines.end()) {
            if (!(wholeLineDoesMatch(*linesIt))) {
                doesNotMach = true;
                break;
            }
        }
        if (doesNotMach) {
            doesNotMach = false;
            ++pos;
            continue;
        }

        std::string permutationString;
        if (isMultilineString(sm.mutation) && addIndentation) {
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
        size_t lengthToRemove = end - startPos;
        ++matches;
        if (sm.data.isNewLined) {
            pos = end - subject.begin() + 1;
            permutationString.push_back('\n');
            lengthToRemove = 0;
        }
        subject.replace(pos, lengthToRemove, permutationString);
        pos += permutationString.length();
        addIndentation = false;
    }
    if (!matches)
        opts->addNoMatchLine(sm.data.lineNumber);
    else if (matches > 1)
        opts->addMultipleMatchLine(sm.data.lineNumber);
}

bool Mutator::lineEdgesAreGood(std::string::iterator& begin, std::string::iterator& end, const std::string& str) {
    if (lastNonWhiteSpace(begin, end) != std::string::npos) return false;
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