/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * common.cpp: Common utilities definitions
 *
 * - Basically any generic routine useful in multiple files but not fitting into
 the context of other files
 * - In other words, this file is a miscellaneous trash dump of whatever for
 whereever
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

#include "common.hpp"

#include <cstddef>
#include <cstring>
#include <memory>
#include <cassert>



// remove special characters from a string so it can be safely shown in the
// console without risk of introducing security vulnerabilities
std::string sanitizeOutputMessage(const char *input) {
	std::size_t inLen = std::strlen(input);
	std::unique_ptr<char[]> outBuff(new char[inLen + 1]);  // RAII?

	for (std::size_t i = 0; i < inLen; i++) {
		if (31 < input[i] && input[i] < 127) {
		outBuff[i] = input[i];
		} 
		else {
		outBuff[i] = '?';  // indicate unknown/invalid character
		}
	}
	outBuff[inLen] = 0;  // null terminate just in case

	return std::string((const char *)outBuff.get(), inLen);
}

std::string sanitizeOutputMessage(std::string input) {
	return sanitizeOutputMessage(input.c_str());
}

// If not white space returns 0, if white space, returns how many bytes it takes up
// created to check for unicode white spaces as well as ascii
// Only works if iterators passed in do not cutoff multibyte utf-8 characters
unsigned int isWhiteSpace(const std::string::iterator& it, const std::string::iterator& end) {
    if(std::isspace(*it)) return 1;
    int value = *it;
    if(value <= 127 && value >= 0) return 0;
    std::uint8_t first = value;
    std::uint8_t second;
    std::uint8_t third;
    if((it + 1) != end ) {
        second = *(it + 1);
        if(first == 0xC2 && second == 0xA0) return 2;
        if((it + 2) != end) {
            third = *(it + 2);
            if(first == 0xE1 && second == 0x9A && third == 0x80) return 3;
            if(first == 0xE3 && second == 0x80 && third == 0x80) return 3;
            if(first == 0xEF && second == 0xBB && third == 0xBF) return 3;
            if(first == 0xE2 && second == 0x81 && third == 0x9F) return 3;
            if(first == 0xE2 && second == 0x80) {
            if((third >= 0x80) && (third <= 0x8A || third == 0xAF || third == 0xA8 || third == 0xA9) )
                return 3;
            }
       }    
    }    
    return 0;
} 

// Returns position from starting position `begin` to last non white character in the section of std::string being passed in
// Uses isWhiteSpace() function above to check for unicode white spaces as well as ascii
// Parameter `end` should either be one position passed the last position you want checked or .end() of the std::string 
// If passed string/sub-string contains only white spaces, function returns std::string::npos 
// NOTE: THIS FUNCTION ONLY WORKS IF PASSED IN STRING DOES NOT CUTOFF ANY PORTION OF MULTI-BYTE CHARACTERS BUT SINCE THE
//      USE CASES IN THIS PROJECT DO NUT RUN THAT RISK IT IS OK FOR OUR PURPOSES. 
size_t lastNonWhiteSpace(std::string::iterator begin, std::string::iterator end) {
   // assert(end > (begin + 1)); // to see if and when this occurs
    if(end <= begin) return std::string::npos;
    auto it = end - 1;
    std::uint8_t value;

    while(true) {
        size_t index = it - begin;
        value = *it;
        while((value & 0x80) && (value < 0xC0) && it != begin) value = *(--it);

        if(isWhiteSpace(it, end)) {
            if(it == begin) {
                return std::string::npos;
            }
            else {
                --it;
            }            
        }
        else {
            return index;
        }
    }
    return std::string::npos; // for compiler
}