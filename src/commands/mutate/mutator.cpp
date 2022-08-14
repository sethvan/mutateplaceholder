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
        if (sm.data.isRegex)
            regexReplace(strippedStr, sm);
        else if (isMultilineStringView(sm.pattern))
            multilineReplace(strippedStr, sm);
        else
            replaceStringInPlace(strippedStr, sm);
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

        if (!edgesGoodAndReplacementSuccessful(begin, end, patternString, subject, sm, permutationString, pos,
                                               lengthToRemove, matches))
            INCREMENT_POS_AND_CONTINUE;
    }
    checkCountOfMatches(matches, sm);
}

void Mutator::regexReplace(std::string& subject, const SelectedMutation& sm) {
    size_t index = sm.pattern.find_last_of('/');
    if (index == std::string::npos) {
        std::ostringstream os;
        os << "Regex pattern cell in row beginning on line number " << sm.data.lineNumber << " is missing final \'/\'."
           << std::endl;
        throw TSVParsingException(os.str());
    }

    auto [pattern, modifiers] = getPatternAndModifiers(index, sm);
    std::set<std::string> matches = getMatches(pattern, subject, modifiers);

    for (const auto& str : matches) {
        std::string_view regexPattern(str);
        std::string mut = jp::Regex(pattern).replace(str, sm.mutation.data(), modifiers);
        std::string_view regexMutation(mut);
        SelectedMutation regexSm(regexPattern, regexMutation, sm.data);
        if (regexSm.pattern.size()) {
            if (isMultilineStringView(regexSm.pattern))
                multilineReplace(subject, regexSm);
            else
                replaceStringInPlace(subject, regexSm);
        }
    }
}

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
            if (!edgesGoodAndReplacementSuccessful(begin, end, patternString, subject, sm, permutationString, pos,
                                                   lengthToRemove, matches))
                ++pos;
            continue;
        }
        std::string indent(begin, end);  // to use later if first check of second line does not match
        auto linesIt = lines.begin();

        if (!lineEdgesAreGood(begin, end, *linesIt, subject)) INCREMENT_POS_AND_CONTINUE;
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

bool Mutator::lineEdgesAreGood(std::string::iterator& begin, std::string::iterator& end, const std::string& str,
                               const std::string& subject) {
    if (lastNonWhiteSpace(begin, end) != std::string::npos) return false;
    begin = (end += (str.size()));
    if (*(begin - 1) == '\n') return true;
    while ((*end != '\n') && (end != subject.end())) ++end;
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

bool Mutator::edgesGoodAndReplacementSuccessful(std::string::iterator& begin, std::string::iterator& end,
                                                const std::string& patternString, std::string& subject,
                                                const SelectedMutation& sm, std::string& permutationString, size_t& pos,
                                                size_t lengthToRemove, int& matches) {
    std::string indent(begin, end);
    if (!lineEdgesAreGood(begin, end, patternString, subject)) return false;

    if (sm.data.isNewLined) {
        permutationString = indent + sm.mutation.data();
        permutationString.push_back('\n');
        if (end == subject.end()) {
            subject.push_back('\n');
            end = subject.end() - 1;
        }
        pos = end - subject.begin() + 1;
        lengthToRemove = 0;
    }
    else
        permutationString = sm.mutation.data();

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
    if (!lineEdgesAreGood(begin, end, str, subject)) return false;
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
    else if (!lineEdgesAreGood(begin, end, *linesIt, subject))
        return false;
    return true;
}

std::tuple<std::string, std::string> Mutator::getPatternAndModifiers(size_t index, const SelectedMutation& sm) {
    std::string defaultFlags("AFgnm");
    std::string pattern{sm.pattern, 0, index};
    std::string userModifierInput(sm.pattern.begin() + index + 1, sm.pattern.end());
    std::string modifiers;
    index = userModifierInput.find_first_of('-');
    if (index != std::string::npos) {
        std::string additionalModFlags(userModifierInput, 0, index);
        std::string flagsToBeRemoved(userModifierInput.begin() + index + 1, userModifierInput.end());
        if (additionalModFlags.size()) modifiers.append(additionalModFlags);
        for (const auto& c : defaultFlags) {
            if ((index = flagsToBeRemoved.find_first_of(c)) == std::string::npos) modifiers.push_back(c);
        }
    }
    else {
        modifiers = userModifierInput + defaultFlags;
    }
    return {pattern, modifiers};
}

std::set<std::string> Mutator::getMatches(const std::string& pattern, const std::string& subject,
                                          const std::string& modifiers) {
    jp::VecNum vec_num;
    jp::Regex re(pattern);
    jp::RegexMatch rr;
    rr.setRegexObject(&re).setSubject(&subject).addModifier(modifiers).setNumberedSubstringVector(&vec_num).match();

    std::set<std::string> strSet;
    for (auto& vec : vec_num) {
        for (auto& n : vec) strSet.insert(n);
    }

    return strSet;
}