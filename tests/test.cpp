/* SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later */
/*
 * test.cpp: Run various tests on all sorts of things to ensure that everything is up to snuff
 *
 * - Bootstraps the startup routine and
 * - Should be the *only* file that prints errors to std:cerr
 * - Organizer of all other files
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

#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdbool>
#include <ranges>
#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <memory>
#include <iomanip>

#define protected public
#define private public
#define class struct
#include "common.hpp"
#include "commands/highlight/highlightCommand.hpp"
#include "commands/mutate/mutateCommand.hpp"
#include "commands/score/scoreCommand.hpp"
#include "commands/validate/validateCommand.hpp"
#include "commands/cli-options.hpp"
#include "commands/cli-parser.hpp"
#undef protected
#undef private
#undef class
#include "excepts.hpp"
#include "commands/mutate/mutationsRetriever.hpp"
#include "commands/mutate/mutationsSelector.hpp"

// template <typename T>
// std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec ) {
// 	//std::ostream os;
// 	os << "{";
// 	for(const auto& el: vec) {
// 		os << el << (&el == &vec.back() ? " }" :", ");
// 	}
// 	os << std::endl;
// 	return os;
// }



typedef std::pair<const char *, std::string> FailedTest;

static std::vector<FailedTest> failedTestArray;
static std::ostringstream testLog;
static std::size_t passedTests, totalTests;

static std::string JSON_stringify_ascii(std::string input) {
	static const char * hexabet = "0123456789ABCDEF";
	
	std::size_t inLen = input.size();
	std::unique_ptr<char[]> outBuff(new char[inLen * 4 + 3]); // RAII?
	char * startPtr = &( outBuff[0] );
	char * outPtr = startPtr;

	*outPtr++ = '"';
	for (std::size_t i=0; i < inLen; i++) {
		char cur = input[i];
		if (31 < cur && cur < 127) {
			*outPtr++ = cur;
		} 
		else if (cur == '\n') {
			*outPtr++ = '\\';
			*outPtr++ = 'n';
		} 
		else if (cur == '\r') {
			*outPtr++ = '\\';
			*outPtr++ = 'r';
		} 
		else if (cur == '\t') {
			*outPtr++ = '\\';
			*outPtr++ = 't';
		} 
		else if (cur == '\f') {
			*outPtr++ = '\\';
			*outPtr++ = 'f';
		} 
		else if (cur == '\0') {
			*outPtr++ = '\\';
			*outPtr++ = '0';
		} 
		else {
			*outPtr++ = '\\';
			*outPtr++ = 'x';
			*outPtr++ = hexabet[(unsigned)(cur) >> 4];
			*outPtr++ = hexabet[(unsigned)(cur) & 15];
		}
	}
	*outPtr++ = '"';
	*outPtr = 0; // null terminate just in case
	
	return std::string( (const char *) outBuff.get(), outPtr - startPtr );
}

template<typename T>
void printThisValueAsJSONToOStream(std::ostream& os, T& value) {
    os << value; // master case
}
template<>
void printThisValueAsJSONToOStream<const std::string>(std::ostream& os, const std::string& value) {
    os << JSON_stringify_ascii( value ); // slave case
}
template<>
void printThisValueAsJSONToOStream<const char *>(std::ostream& os, const char *& value) {
    os << JSON_stringify_ascii( std::string(value) ); // slave case
}

// Based upon https://stackoverflow.com/a/10758845/5601591
template<class T>
static std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    if ( !v.empty() ) {
        out << '{';
        //std::ranges::copy(v, std::ostream_iterator<char>(out, ", "));
		bool isFirst = true;
		for (const auto& valueCur : v) {
			if (isFirst) {
				isFirst = false;
			} 
			else {
				out << ", ";
			}
			printThisValueAsJSONToOStream(out, valueCur);
			

			// if (std::is_base_of<std::string, T>::value) {
			// 	out << JSON_stringify_ascii( valueCur );
			// } 
			// else if (std::is_base_of<const char *, const T>::value) {
			// 	out << JSON_stringify_ascii( std::string(valueCur) );
			// } 
			// else {
			// 	out << valueCur;
			// }
		}
        out << '}';
    }
    return out;
}

void printFailedTestResults(void) {
	if (0 < failedTestArray.size()) {
		printf("\n");
	}
	
	for (const FailedTest& value : failedTestArray) {
		printf("\n\x1B[31mFAIL\033[0m %s\n%s", value.first, value.second.c_str());
	}

	printf("\n");

	if (passedTests == totalTests) {
		printf("\n\x1B[92mAll %zu tests are passing!\033[0m\n", totalTests);
	} 
	else if (0 < passedTests) {
		printf("\n\x1B[93m%zu/%zu tests are passing\033[0m\n", passedTests, totalTests);
	} 
	else {
		printf("\n\x1B[91mNone of the %zu tests are passing!\033[0m\n", totalTests);
	}
}

// Notice that `true` indicates error and `false` indicates okay
#define POOR_MANS_TEST(name, function, ...) \
	do { \
		++totalTests; \
		testLog = std::ostringstream(); \
		if ( function(__VA_ARGS__) ) { \
			failedTestArray.emplace_back( name, testLog.str() ); \
		} else { \
			++passedTests; \
			printf("\x1B[32mPASS\033[0m %s\n", name); \
		} \
	} while(false)

#define INDENT "    "

/*bool makeSureTestSystemWorks() {
	testLog << INDENT "NOTICE: this test should always fail and this message printed. If this test is not failing, then there's an issue with the test system." << '\n';
	return true; // change this to `true` to reveal the test log
}*/

static bool testDoubleDashPositionalArgs() {
	testLog << INDENT "Passing {\"./test\", \"--\", \"--seed\", \"71E8DC1EC351FAFA40998B1178F7AE00328B4D464172111F6B2AA49D4BC6C1A6\"} to parseArgs" << '\n';
	const char * argv[] = {
		"./test",
		"--",
		"--seed",
		"71E8DC1EC351FAFA40998B1178F7AE00328B4D464172111F6B2AA49D4BC6C1A6",
		nullptr
	};
	int argc = 0;
	while (argv[argc] != nullptr) ++argc;
	
	CLIOptions parsedArgs;
	std::vector<std::string> nonpositionals;
	ParseArgvStatusCode status = parseArgs(&parsedArgs, &nonpositionals, argc, argv);

	if (status != ParseArgvStatusCode::SUCCESS) {
		testLog << INDENT "ERR: failed to parse arguments. Got ParseArgvStatusCode code " << (int) status << '\n';
		return true;
	}

	std::vector<std::string> expected {
		"--seed",
		"71E8DC1EC351FAFA40998B1178F7AE00328B4D464172111F6B2AA49D4BC6C1A6"
	};

	testLog << INDENT "Expecting " << expected << '\n';
	testLog << INDENT "Got       " << nonpositionals << '\n';

	if (expected != nonpositionals) {
		return true; // error
	}

	return false; // change this to `true` to reveal the test log
}

static bool testSeedParsing() {
	const char * expectedSeed = "71E8DC1EC351FAFA40998B1178F7AE00328B4D464172111F6B2AA49D4BC6C1A6";
	
	testLog << INDENT "Passing {\"./test\", \"--seed\", \"" << expectedSeed << "\"} to parseArgs" << '\n';
	const char * argv[] = {
		"./test",
		"--seed",
		expectedSeed,
		nullptr
	};
	int argc = 0;
	while (argv[argc] != nullptr) ++argc;
	
	CLIOptions parsedArgs;
	std::vector<std::string> nonpositionals;
	ParseArgvStatusCode status = parseArgs(&parsedArgs, &nonpositionals, argc, argv);

	if (status != ParseArgvStatusCode::SUCCESS) {
		testLog << INDENT "ERR: failed to parse arguments. Got ParseArgvStatusCode code " << (int) status << '\n';
		return true;
	}

	if ( ! parsedArgs.hasSeed()) {
		testLog << INDENT "ERR: parsedArgs did not recieve and define the seed property" << '\n';
		return true;
	}

	testLog << INDENT "Expecting the seed to be " << JSON_stringify_ascii( std::string(expectedSeed) ) << '\n';
	testLog << INDENT "Got the seed being       " << JSON_stringify_ascii( parsedArgs.getSeed() ) << '\n';

	if (std::string(expectedSeed) != parsedArgs.getSeed()) {
		return true; // error
	}

	return false; // change this to `true` to reveal the test log
}

static bool testSrcDevNullInput() {
	const char * inputFile = "/dev/null";
	const char * inputContents = "";
	
	testLog << INDENT "Passing {\"./test\", \"--input\", \"" << inputFile << "\"} to parseArgs" << '\n';
	const char * argv[] = {
		"./test",
		"--input",
		inputFile,
		nullptr
	};
	int argc = 0;
	while (argv[argc] != nullptr) ++argc;
	
	CLIOptions parsedArgs;
	std::vector<std::string> nonpositionals;
	ParseArgvStatusCode status = parseArgs(&parsedArgs, &nonpositionals, argc, argv);

	if (status != ParseArgvStatusCode::SUCCESS) {
		testLog << INDENT "ERR: failed to parse arguments. Got ParseArgvStatusCode code " << (int) status << '\n';
		return true;
	}

	if (parsedArgs.srcInput == stdin) {
		testLog << INDENT "ERR: parsedArgs did not recieve and define the source code input file property (srcInput)" << '\n';
		return true;
	}

	testLog << INDENT "Expecting the src input file to be " << JSON_stringify_ascii( std::string(inputContents) ) << '\n';
	testLog << INDENT "Got the src input file being       " << JSON_stringify_ascii( parsedArgs.getSrcString() ) << '\n';

	if (std::string(inputContents) != parsedArgs.getSrcString()) {
		return true; // error
	}

	return false; // change this to `true` to reveal the test log
}

static bool testTsvRealFileInput() {
	char inputFile [L_tmpnam] = {0};
	const char * inputContents = "# mutation test file example\nmyString = \"hello\";\tmyString = \"world\";\n";

	std::tmpnam(inputFile);
	FILE * tmpHandle = std::fopen(inputFile, "w");
	fwrite((const void *) inputContents, 1, std::strlen(inputContents), tmpHandle);
	fclose(tmpHandle);
	
	testLog << INDENT "Passing {\"./test\", \"--mutations\", \"" << inputFile << "\"} to parseArgs" << '\n';
	const char * argv[] = {
		"./test",
		"--mutations",
		inputFile,
		nullptr
	};
	int argc = 0;
	while (argv[argc] != nullptr) ++argc;
	
	CLIOptions parsedArgs;
	std::vector<std::string> nonpositionals;
	ParseArgvStatusCode status = parseArgs(&parsedArgs, &nonpositionals, argc, argv);

	if (status != ParseArgvStatusCode::SUCCESS) {
		testLog << INDENT "ERR: failed to parse arguments. Got ParseArgvStatusCode code " << (int) status << '\n';
		remove( inputFile );
		return true;
	}

	if (parsedArgs.tsvInput == stdin) {
		testLog << INDENT "ERR: parsedArgs did not recieve and define the tsv mutations input file property (tsvInput)" << '\n';
		remove( inputFile );
		return true;
	}

	testLog << INDENT "Expecting the tsv input file to be " << JSON_stringify_ascii( std::string(inputContents) ) << '\n';
	testLog << INDENT "Got the tsv input file being       " << JSON_stringify_ascii( parsedArgs.getTsvString() ) << '\n';

	remove( inputFile );
	
	if (std::string(inputContents) != parsedArgs.getTsvString()) {
		return true; // error
	}

	return false; // change this to `true` to reveal the test log
}

// static bool patternOperatorsTest(std::vector<size_t> passedIndexes, std::vector<size_t> expectedIndexes) {
// 	const char* inputFile = "./src/commands/cli-options.cpp";
// 	const char* tsvFile = "../small-samples/mySamples/cli-options.tsv";
// 	testLog << INDENT "Passing {\"./test\", \"mutate\", \"-i\", \"" << inputFile << "\", \"-m\", \"" << tsvFile << "\"} to parseArgs" << '\n';
// 	const char * argv[] = {"./test", "mutate", "-i", inputFile, "-m", tsvFile, nullptr};
// 	int argc = 0;
// 	while (argv[argc] != nullptr) ++argc;
	
// 	CLIOptions parsedArgs;
// 	std::vector<std::string> nonpositionals;
// 	parseArgs(&parsedArgs, &nonpositionals, argc, argv);
// 	std::istringstream tsvStream{parsedArgs.getTsvString()};
// 	if(!tsvStream) throw IOErrorException("I/O error opening TSV File");
// 	std::istringstream srcStream{parsedArgs.getSrcString()};
// 	if(!srcStream) throw IOErrorException("I/O error opening Source File");

// 	MutationsRetriever mRetriever{tsvStream};
// 	MutationsSelector mSelector{&parsedArgs, mRetriever.getPossibleMutations()};
// 	testLog << INDENT "Passing indexes " << passedIndexes << " to MutationsSelector\n";
	
// 	mSelector.selectedIndexes = passedIndexes;
// 	SelectedMutVec sm = mSelector.getSelectedMutations();
// 	std::vector<size_t> expectedLines = expectedIndexes;
// 	std::vector<size_t> selectedLines{};
// 	for(const auto& e : sm) {
// 		selectedLines.push_back(e.data.lineNumber);
// 	}

// 	testLog << INDENT "Expect the following indexes to be selected: " << expectedIndexes << "\n";
// 	testLog << INDENT " Received the following indexes as selected: " << selectedLines << "\n";
// 	// for(const auto& e: selectedLines) {
// 	// 	testLog << e << (e==selectedLines.back()? "" : ", ");
// 	// }

// 	// testLog << "}\n";

// 	return (expectedLines != selectedLines);
// }

bool insertionOperatorTest() {
	std::string expectedStr = "{10, 9, 8, 7, 6, 5, 4, 3, 2, 1}";
	testLog << INDENT "Expected: " << JSON_stringify_ascii(expectedStr) << std::endl;

	std::vector<int> vec{10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
	std::ostringstream os;
	os << vec;
	std::string actual{os.str()};
	testLog << INDENT "     Got: " << JSON_stringify_ascii(actual) << std::endl;

	return expectedStr != actual;
}

bool bruteForceUnicodeWhitespaceUnitTest() {
    std::string testData;
    int count = 0;
    uint_least32_t codepoint = 0;
    auto testWhitespaceAtCP = [&testData, &count, &codepoint]() {
        if (0 != isWhiteSpace(testData.begin(), testData.end())) {
            testLog << "Detected whitespace codepoint 0x" << std::hex
                    << codepoint << " (bytes "
                    << std::hex << std::setw(2) << std::setfill('0') << (int) (unsigned char) testData[0];
            if (1 < testData.size())
                testLog << ' ' << std::hex << std::setw(2) << std::setfill('0') << (int) (unsigned char) testData[1];
            if (2 < testData.size())
                testLog << ' ' << std::hex << std::setw(2) << std::setfill('0') << (int) (unsigned char) testData[2];
            testLog << ")\n";
            ++count;
        }
    };

    for (testData.resize(1); codepoint <= 0x7F; codepoint++)
        testData[0] = codepoint,
        testWhitespaceAtCP();

    for (testData.resize(2); codepoint <= 0x7FF; codepoint++)
        testData[0] = 0xC0 | (codepoint >> 6),
        testData[1] = 0x80 | (codepoint & 0x3F),
        testWhitespaceAtCP();

    for (testData.resize(3); codepoint <= 0xFFFF; codepoint++)
        testData[0] = 0xE0 | (codepoint >> 12),
        testData[1] = 0x80 | ((codepoint >> 6) & 0x3F),
        testData[2] = 0x80 | (codepoint & 0x3F),
        testWhitespaceAtCP();

    testLog << "Found " << std::dec << count << " whitespace characters; expected 25.\n";

    return count != 25;
}



int main(int argc, const char **argv) {
	(void) argc;
	(void) argv;

	//POOR_MANS_TEST("Make sure test system works", makeSureTestSystemWorks);

// #define PASSED_INDEXES {80, 78, 84, 103, 104, 105, 106, 107, 108, 109, 110}
// #define EXPECTED_LINES {81, 79, 84, 87, 106, 109}

	POOR_MANS_TEST("Double-dash (--) ends options and starts positional arguments", testDoubleDashPositionalArgs);

	POOR_MANS_TEST("Parse --seed CLI option", testSeedParsing);

	POOR_MANS_TEST("Read --input src /dev/null empty file", testSrcDevNullInput);

	POOR_MANS_TEST("Read --mutations tsv real file", testTsvRealFileInput);

	POOR_MANS_TEST("Insertion Operator Test", insertionOperatorTest);

	//POOR_MANS_TEST("Line selection matchup", patternOperatorsTest, {18, 33, 48, 51, 46, 3, 2, 1, 0}, {64, 61, 59, 58, 56, 45, 24, 4, 3});
	
	POOR_MANS_TEST("Test isWhiteSpace() function", bruteForceUnicodeWhitespaceUnitTest);

	printFailedTestResults();
	
	return 0;
}

