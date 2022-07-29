/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * excepts.hpp: Contains custom exceptions. Since there are not that many, consolidated them into same file.
 * - If program becomes too big will separate into separate files.
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

#ifndef _INCLUDED_EXCEPTS_HPP_
#define _INCLUDED_EXCEPTS_HPP_

#include <stdexcept>
#include <string>

class TSVParsingException : public std::runtime_error {
   public:
    TSVParsingException() : std::runtime_error("Error parsing TSV File") {}
    explicit TSVParsingException(const std::string& message) : std::runtime_error(message) {}
    explicit TSVParsingException(const char* message) : std::runtime_error(message) {}
    ~TSVParsingException() = default;
};

class InvalidSeedException : public std::runtime_error {
   public:
    InvalidSeedException() : std::runtime_error("Error processing seed") {}
    explicit InvalidSeedException(const std::string& message) : std::runtime_error(message) {}
    explicit InvalidSeedException(const char* message) : std::runtime_error(message) {}
    ~InvalidSeedException() = default;
};

class InvalidArgumentException : public std::runtime_error {
   public:
    InvalidArgumentException() : std::runtime_error("Error processing arguments") {}
    explicit InvalidArgumentException(const std::string& message) : std::runtime_error(message) {}
    explicit InvalidArgumentException(const char* message) : std::runtime_error(message) {}
    ~InvalidArgumentException() = default;
};

class IOErrorException : public std::runtime_error {
   public:
    IOErrorException() : std::runtime_error("I/O error") {}
    explicit IOErrorException(const std::string& message) : std::runtime_error(message) {}
    explicit IOErrorException(const char* message) : std::runtime_error(message) {}
    ~IOErrorException() = default;
};

#endif  // _INCLUDED_EXCEPTS_HPP_
