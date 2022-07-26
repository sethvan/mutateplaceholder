/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * mutationsRetriever.cpp: This class parses and validates the TSV input, capturing the possible mutations. 
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
#include "excepts.hpp"
#include "common.hpp"
#include <iostream>
#include <sstream>
#include <cctype>
#include <set>
#include <algorithm>
#include <cassert>

using stringIter = std::string::iterator;

MutationsRetriever::MutationsRetriever(std::istream& tsvInput)
	: tsvStream{tsvInput} {}

void MutationsRetriever::capturePossibleMutations() {
	std::vector<TSVRow> rows = getRows();
	auto rowsIt = rows.begin();
	std::string line;

	do {
		line = rowsIt->row;
		stringIter lineIt = line.begin();
		int lineNumber = rowsIt->lineNumber;

		checkIndentation(lineIt, line.end() ,lineNumber);

		std::string pattern = getPatternOrPermutation(lineIt, line.end(), lineNumber, rowsIt->lineNumber);
		possibleMutations.push_back(std::move(pattern));

		verifyHasPermutation(lineIt, line.end(), lineNumber, rowsIt->lineNumber);

		while (lineIt != line.end()) {
			while (*lineIt == '\t') ++lineIt; // will later have option to disable ignoring of white space cells
			std::string permutation = getPatternOrPermutation(lineIt, line.end(), lineNumber, rowsIt->lineNumber);
			possibleMutations.back().permutations.push_back(std::move(permutation));
		}
		possibleMutations.back().data.lineNumber = rowsIt->lineNumber;
	} while (++rowsIt != rows.end());
}

void MutationsRetriever::categorizeMutations() {
	auto pmIt = possibleMutations.begin();
	while( pmIt != possibleMutations.end()) {
		if( 0 == pmIt->pattern.find_first_not_of("^@+/!") && 
				((pmIt + 1) != possibleMutations.end() ? (0 == (pmIt+1)->pattern.find_first_not_of("^@")) : true)) {
			++pmIt;
			continue;
		} 
		if (0 == pmIt->pattern.find_first_not_of("^@") && (pmIt + 1) != possibleMutations.end() && 
				0 == (pmIt +1 )->pattern.find_first_of("^@") && 1 == (pmIt + 1)->pattern.find_first_not_of("^@")) {
			pmIt->data.depth = 1; // group leader
		}
		if(0 == pmIt->pattern.find_first_of("^@/+!")){
			auto patIt = pmIt->pattern.begin();
			switch(*patIt) {
				case '^' :				
					caseCaret(patIt, pmIt);
					break;
				case '@' :				
					caseSynced(patIt, pmIt);
					break;
				default :
					caseSpecialChars(patIt, pmIt);
					break;
			}
		}				
		++pmIt;
	}
}


void MutationsRetriever::checkIndentation(stringIter it, const stringIter& end, int& lineNumber) {
	if (isWhiteSpace(it, end)) {
		std::ostringstream os;
		os	<< " Error : Indentation detected.\n"
			<< "Notice :\n    Cells in TSV format should not be indented.\n"
			<< "    Indentation found at row " << lineNumber << " of TSV File."
			<< std::endl;
		throw TSVParsingException(os.str());
	}
}

std::string MutationsRetriever::getPatternOrPermutation(stringIter& it, const stringIter& end, int& lineNumber, int rowBeginningLine) {
	std::string res = "";
	char* start = &(*it);       // <- to calculate index if error
	int consecutiveQuotes = 0;  // <- to determine when a quoted cell has ended,
	                            // i.e. if '\t' appears and this number is odd
	if (*it == '"') {
		++it;
		while (it != end) {
			if (*it == '\n') {
				++lineNumber;
				start = &(*(it+1));
			}
			if (*it == '"') {
				++consecutiveQuotes;
				if ((*(it + 1) == '\t' && (consecutiveQuotes % 2)) || (it + 1) == end) {
					// end of quoted cell
					++it;
					break;
				} 
				else if (*(it + 1) == '"' && *(it + 2) != '\t') {
					// escaped quote in quoted cell
					++it;
					++consecutiveQuotes;
				} 
				else if (*(it + 1) != '\t' && (consecutiveQuotes % 2) && (it + 1) != end) {
					// invalid character after end of quoted cell
					int index = (&(*(it + 2))) - start;
					throwInvalidCharException(it, end, index, lineNumber, rowBeginningLine); // long enough to extract to own method
				}
			} else {
				consecutiveQuotes = 0;
			}
			res.push_back(*(it++));
		}
		if (!(consecutiveQuotes % 2)) {
			// final cell in row is missing terminating quote
			throwTerminatingQuoteException(lineNumber); // extracting to own method for consistency with above
		}
	} else
		while (it != end && *it != '\t') res.push_back(*(it++));
	return res;
}

void MutationsRetriever::verifyHasPermutation(stringIter it, const stringIter& end, int& lineNumber, int rowBeginningLine) {
	if (it == end || noPermutationsInLine(it, end)) {
		std::ostringstream os;
		os	<< " Error : Permutation cell missing in TSV File.\n"
			<< "Notice :\n    Missing permutation cell on line number " << lineNumber
			<< "\n    Row that begins with pattern cell on line number " << rowBeginningLine
			<< " has no corresponding permutation cell(s)."
			<< std::endl;
		throw TSVParsingException(os.str());
	}
}

// use of isWhiteSpace() ignores white space cells, changing back to accept white space cells for now
// if we add ignoring option, this method will be modified back
bool MutationsRetriever::noPermutationsInLine(stringIter it, const stringIter& end) {
	//unsigned int bytes;
	//while (bytes = isWhiteSpace(it, end)) it += bytes;
	while(*it == '\t') ++it;
	return it == end;
}

PossibleMutVec& MutationsRetriever::getPossibleMutations() {
	capturePossibleMutations();
	categorizeMutations();
	checkNesting();
	return possibleMutations;
}

void MutationsRetriever::throwInvalidCharException(stringIter it, const stringIter& end, int index, int lineNumber, int rowBeginningLine) {
	std::string c = "[ '0' ]";
	c[3] = *(it + 1);

	std::string invalidChar = isWhiteSpace((it + 1), end) ? "['SPACE']" : *(it + 1) == '"'? "['QUOTATION MARK']" : c;
	std::ostringstream os;

	os	<< " Error : Invalid syntax found at index " << index << " of line number " << lineNumber << " in TSV\n"
		<< "Notice :\n    Currently found in your TSV : ... \"" << invalidChar	<< "...\n"
		<< "    Expected to be found in TSV : ... \"['TAB']...\n"
		<< "\nIf index " << index - 1 << " is not intended end of quoted cell, "
		<< "check preceding section of the row beginning with pattern cell on line number "<< rowBeginningLine << "\nfor any extra or missing "
		<< "QUOTATION MARKS and/or TABs as they are likely cause of error." << std::endl;
	throw TSVParsingException(os.str());
}

void MutationsRetriever::throwTerminatingQuoteException(int lineNumber) {
	std::ostringstream os;
	os	<< " Error : Terminating quote missing.\n"
		<< "Notice :\n    Cells beginning with QUOTATION MARK must end with "
		<< "QUOTATION MARK.\n"
		<< "    Final cell on line number " << lineNumber
		<< " missing terminating QUOTATION MARK." << std::endl;
	throw TSVParsingException(os.str());
}

void MutationsRetriever::throwEmptyPatternException(int lineNumber) {
	std::ostringstream os;
	os	<< " Error : Cell content missing in TSV File.\n"
		<< "Notice :\n    Missing cell content for pattern cell on line number " << lineNumber
		<< std::endl;
	throw TSVParsingException(os.str());
}

void MutationsRetriever::caseCaret(stringIter patIt, PossibleMutVec::iterator& pmIt) {
	pmIt->data.depth = 2; 
	
	while(((patIt + 1) != pmIt->pattern.end()) && *(++patIt) == '^') ++(pmIt->data.depth);

	if((*patIt == '^'? ++patIt : patIt)  == pmIt->pattern.end()) {
		throwEmptyPatternException(pmIt->data.lineNumber);
	}
	if(*patIt == '@') {
		caseSynced(patIt, pmIt);
	}
	else if(pmIt->pattern.find_first_of("+?/!") == static_cast<size_t>(patIt - pmIt->pattern.begin())) {
		caseSpecialChars(patIt, pmIt);
	}
}

void MutationsRetriever::caseSynced(stringIter patIt, PossibleMutVec::iterator& pmIt) {
	pmIt->data.depth = pmIt->data.depth == 0 ? 2 : (pmIt->data.depth + 1); // depth of non group leaders cab never be 1
	pmIt->data.isIndexSynced = true;
	if(++patIt == pmIt->pattern.end()) {
		throwEmptyPatternException(pmIt->data.lineNumber);
	}
	if(pmIt->pattern.find_first_of("+?/!") == static_cast<size_t>(patIt - pmIt->pattern.begin())) {
		caseSpecialChars(patIt, pmIt);
	}
}

void MutationsRetriever::checkNesting() {
	assert(possibleMutations.size());
	auto it = possibleMutations.begin();
	auto throwInvalidNesting = [&](){
		std::ostringstream os;
		os	<< " Error : Invalid group nesting syntax in TSV File.\n"
			<< "Notice :\n     Nested pattern cell in row number " << (it+1)->data.lineNumber << " has no corresponding parent."
			<< std::endl;
		throw TSVParsingException(os.str());
	};
	if(it->data.depth > 1)
		throwInvalidNesting();
	while((it + 1) != possibleMutations.end()) {
		if(((it->data.depth < (it + 1)->data.depth) && (((it + 1)->data.depth - it->data.depth) > 1)) || 
		  (((it + 1)->data.depth > 2) && (it + 1)->data.depth <= it->data.depth)) {
			throwInvalidNesting();
		}
		++it;
	}
}

void MutationsRetriever::caseSpecialChars(stringIter patIt, PossibleMutVec::iterator& pmIt) {
	std::set<char> sChars{'+', '!', '?'};

	while(sChars.find(*patIt) != sChars.end() && (patIt) != pmIt->pattern.end()) {
        sChars.erase(*patIt);
        switch(*patIt) {
            case '+' : 
                pmIt->data.isNewLined = true;
                break;
            case '?' :
                pmIt->data.isOptional = true;
                break;
            case '!' :
                pmIt->data.mustPass = true;
                break;
        }
        ++patIt;
    }
	if((patIt) != pmIt->pattern.end() && *patIt == '/') {
		pmIt->data.isRegex = true;
		++patIt;
	}
	if(patIt == pmIt->pattern.end()) {	
		throwEmptyPatternException(pmIt->data.lineNumber);
	}
}

std::vector<TSVRow> MutationsRetriever::getRows() {
	std::vector<TSVRow> temp;
	temp.push_back({"",1});
	char c, last;
	int QMarkCount = 0, lineNumber = 1;
	bool countTheQMarks = true;

	tsvStream.get(c);
	if((last = c) == '\n') { // in case first line is empty
		++lineNumber;
	}
	else {
		if(c == '"') {
			++QMarkCount;
		}
		else {
			countTheQMarks = false;
		}
		temp.back().row.push_back(c);
	}		

	while(tsvStream.get(c)) {
		if(c == '\t' && !(QMarkCount%2) && countTheQMarks) { 
			QMarkCount = 0;
			countTheQMarks = false;
		}
		if(c == '"') {
			if(!countTheQMarks) {
				if(!temp.back().row.size() || last == '\t') {
				++QMarkCount;
				countTheQMarks = true;
				}
			}
			else 
				++QMarkCount;			
		}	

		if(c == '\n') {
			++lineNumber;
			if(last == '\n' && !(QMarkCount%2)) continue;
			if((last != '\n' && !(QMarkCount%2)) || temp.back().row[0] == '#') {
				temp.push_back({"", lineNumber});
				QMarkCount = 0;
				last = c;
				continue;
			}
		}
		temp.back().row.push_back(c);
		last = c;
	}
	if(!temp.back().row.size()) temp.pop_back();
	
	std::vector<TSVRow> rows;
	rows.reserve(temp.size());
	std::for_each(temp.begin(), temp.end(), [&](TSVRow& Row){
		if(Row.row[0] != '#') rows.push_back(Row);
	});
	// std::cout << "rows.size(): " << rows.size() << std::endl;
	// for(const auto& row: rows) {
	// 	std::cout << row.lineNumber << ": " << row.row << std::endl;
	// }
	return rows;
}