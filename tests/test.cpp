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

#include <algorithm>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define protected public
#define private public
#define class struct
#include "commands/cli-options.hpp"
#include "commands/cli-parser.hpp"
#include "commands/highlight/highlightCommand.hpp"
#include "commands/mutate/mutateCommand.hpp"
#include "commands/mutate/mutationsRetriever.hpp"
#include "commands/mutate/mutationsSelector.hpp"
#include "commands/score/scoreCommand.hpp"
#include "commands/validate/validateCommand.hpp"
#include "common.hpp"
#undef protected
#undef private
#undef class
#include "excepts.hpp"

// (extern variable from common.hpp) do not need to see process status messages for these tests
const bool verbose = false;

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

typedef std::pair<const char*, std::string> FailedTest;

static std::vector<FailedTest> failedTestArray;
static std::ostringstream testLog;
static std::size_t passedTests, totalTests;

static std::string JSON_stringify_ascii(std::string input) {
    static const char* hexabet = "0123456789ABCDEF";

    std::size_t inLen = input.size();
    std::unique_ptr<char[]> outBuff(new char[inLen * 4 + 3]);  // RAII?
    char* startPtr = &(outBuff[0]);
    char* outPtr = startPtr;

    *outPtr++ = '"';
    for (std::size_t i = 0; i < inLen; i++) {
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
            *outPtr++ = hexabet[(unsigned)(cur)&15];
        }
    }
    *outPtr++ = '"';
    *outPtr = 0;  // null terminate just in case

    return std::string((const char*)outBuff.get(), outPtr - startPtr);
}

template <typename T>
void printThisValueAsJSONToOStream(std::ostream& os, T& value) {
    os << value;  // master case
}
template <>
void printThisValueAsJSONToOStream<const std::string>(std::ostream& os, const std::string& value) {
    os << JSON_stringify_ascii(value);  // slave case
}
template <>
void printThisValueAsJSONToOStream<const char*>(std::ostream& os, const char*& value) {
    os << JSON_stringify_ascii(std::string(value));  // slave case
}

// Based upon https://stackoverflow.com/a/10758845/5601591
template <class T>
static std::ostream& operator<<(std::ostream& out, const std::vector<T>& v) {
    if (!v.empty()) {
        out << '{';
        // std::ranges::copy(v, std::ostream_iterator<char>(out, ", "));
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
#define POOR_MANS_TEST(name, function, ...)                    \
    do {                                                       \
        ++totalTests;                                          \
        testLog = std::ostringstream();                        \
        if (function(__VA_ARGS__)) {                           \
            failedTestArray.emplace_back(name, testLog.str()); \
        }                                                      \
        else {                                                 \
            ++passedTests;                                     \
            printf("\x1B[32mPASS\033[0m %s\n", name);          \
        }                                                      \
    } while (false)

#define INDENT "    "

struct parsingBoilerPlate {
    CLIOptions parsedArgs;
    std::vector<std::string> nonpositionals;
    ParseArgvStatusCode status;

    parsingBoilerPlate(const char* argv[]) {
        int argc = 0;
        while (argv[argc] != nullptr) ++argc;
        status = parseArgs(&parsedArgs, &nonpositionals, argc, argv);
        testLog << INDENT "Passing {";
        for (int i = 0; i < argc; ++i) {
            testLog << "\"" << argv[i] << "\"" << (i == argc - 1 ? "} to parseArgs \n" : ", ");
        }
    }
};

/*bool makeSureTestSystemWorks() {
        testLog << INDENT "NOTICE: this test should always fail and this message printed. If this test is not failing,
then there's an issue with the test system." << '\n'; return true; // change this to `true` to reveal the test log
}*/

static bool testDoubleDashPositionalArgs() {
    const char* argv[] = {"./test", "--", "--seed", "71E8DC1EC351FAFA40998B1178F7AE00328B4D464172111F6B2AA49D4BC6C1A6",
                          nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    if (status != ParseArgvStatusCode::SUCCESS) {
        testLog << INDENT "ERR: failed to parse arguments. Got ParseArgvStatusCode code " << (int)status << '\n';
        return true;
    }

    std::vector<std::string> expected{"--seed", "71E8DC1EC351FAFA40998B1178F7AE00328B4D464172111F6B2AA49D4BC6C1A6"};

    testLog << INDENT "Expecting " << expected << '\n';
    testLog << INDENT "Got       " << nonpositionals << '\n';

    if (expected != nonpositionals) {
        return true;  // error
    }

    return false;  // change this to `true` to reveal the test log
}

static bool testSeedParsing() {
    const char* expectedSeed = "71E8DC1EC351FAFA40998B1178F7AE00328B4D464172111F6B2AA49D4BC6C1A6";
    const char* argv[] = {"./test", "--seed", expectedSeed, nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    if (status != ParseArgvStatusCode::SUCCESS) {
        testLog << INDENT "ERR: failed to parse arguments. Got ParseArgvStatusCode code " << (int)status << '\n';
        return true;
    }

    if (!parsedArgs.hasSeed()) {
        testLog << INDENT "ERR: parsedArgs did not recieve and define the seed property" << '\n';
        return true;
    }

    testLog << INDENT "Expecting the seed to be " << JSON_stringify_ascii(std::string(expectedSeed)) << '\n';
    testLog << INDENT "Got the seed being       " << JSON_stringify_ascii(parsedArgs.getSeed()) << '\n';

    if (std::string(expectedSeed) != parsedArgs.getSeed()) {
        return true;  // error
    }

    return false;  // change this to `true` to reveal the test log
}

static bool testSrcDevNullInput() {
    const char* inputFile = "/dev/null";
    const char* inputContents = "";
    const char* argv[] = {"./test", "--input", inputFile, nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    if (status != ParseArgvStatusCode::SUCCESS) {
        testLog << INDENT "ERR: failed to parse arguments. Got ParseArgvStatusCode code " << (int)status << '\n';
        return true;
    }

    if (parsedArgs.srcInput == stdin) {
        testLog << INDENT "ERR: parsedArgs did not recieve and define the source code input file property (srcInput)"
                << '\n';
        return true;
    }

    testLog << INDENT "Expecting the src input file to be " << JSON_stringify_ascii(std::string(inputContents)) << '\n';
    testLog << INDENT "Got the src input file being       " << JSON_stringify_ascii(parsedArgs.getSrcString()) << '\n';

    if (std::string(inputContents) != parsedArgs.getSrcString()) {
        return true;  // error
    }

    return false;  // change this to `true` to reveal the test log
}

static bool testTsvRealFileInput() {
    char inputFile[L_tmpnam] = {0};
    const char* inputContents = "# mutation test file example\nmyString = \"hello\";\tmyString = \"world\";\n";

    std::tmpnam(inputFile);
    FILE* tmpHandle = std::fopen(inputFile, "w");
    fwrite((const void*)inputContents, 1, std::strlen(inputContents), tmpHandle);
    fclose(tmpHandle);

    const char* argv[] = {"./test", "--mutations", inputFile, nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    if (status != ParseArgvStatusCode::SUCCESS) {
        testLog << INDENT "ERR: failed to parse arguments. Got ParseArgvStatusCode code " << (int)status << '\n';
        remove(inputFile);
        return true;
    }

    if (parsedArgs.tsvInput == stdin) {
        testLog << INDENT "ERR: parsedArgs did not recieve and define the tsv mutations input file property (tsvInput)"
                << '\n';
        remove(inputFile);
        return true;
    }

    testLog << INDENT "Expecting the tsv input file to be " << JSON_stringify_ascii(std::string(inputContents)) << '\n';
    testLog << INDENT "Got the tsv input file being       " << JSON_stringify_ascii(parsedArgs.getTsvString()) << '\n';

    remove(inputFile);

    if (std::string(inputContents) != parsedArgs.getTsvString()) {
        return true;  // error
    }

    return false;  // change this to `true` to reveal the test log
}

static bool patternOperatorsTest(const char* tsvFile, std::vector<size_t> passedLines,
                                 std::vector<size_t> expectedLines) {
    const char* argv[] = {"./test", "mutate", "-m", tsvFile, nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    MutationsRetriever mRetriever{parsedArgs.getTsvString()};
    MutationsSelector mSelector{&parsedArgs, mRetriever.getPossibleMutations()};
    testLog << INDENT "Passing lines " << passedLines << " to MutationsSelector\n";

    // passedLines are really line(or row depending) numbers, thus subtracting 1 to match array element
    std::transform(passedLines.begin(), passedLines.end(), passedLines.begin(), [](auto n) { return n - 1; });

    mSelector.selectedIndexes = passedLines;
    SelectedMutVec sm = mSelector.getSelectedMutations();
    std::vector<size_t> selectedLines{};
    for (const auto& e : sm) {
        selectedLines.push_back(e.data.lineNumber);
    }

    testLog << INDENT "Expect the following lines to be selected: " << expectedLines << "\n";
    testLog << INDENT " Received the following lines as selected: " << selectedLines << "\n";

    return (expectedLines != selectedLines);
}

bool insertionOperatorTest() {
    std::string expectedStr = "{10, 9, 8, 7, 6, 5, 4, 3, 2, 1}";
    testLog << INDENT "Expected: " << JSON_stringify_ascii(expectedStr) << std::endl;

    std::vector<int> vec{10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
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
            testLog << "Detected whitespace codepoint 0x" << std::hex << codepoint << " (bytes " << std::hex
                    << std::setw(2) << std::setfill('0') << (int)(unsigned char)testData[0];
            if (1 < testData.size())
                testLog << ' ' << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)testData[1];
            if (2 < testData.size())
                testLog << ' ' << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)testData[2];
            testLog << ")\n";
            ++count;
        }
    };

    for (testData.resize(1); codepoint <= 0x7F; codepoint++) testData[0] = codepoint, testWhitespaceAtCP();

    for (testData.resize(2); codepoint <= 0x7FF; codepoint++)
        testData[0] = 0xC0 | (codepoint >> 6), testData[1] = 0x80 | (codepoint & 0x3F), testWhitespaceAtCP();

    for (testData.resize(3); codepoint <= 0xFFFF; codepoint++)
        testData[0] = 0xE0 | (codepoint >> 12), testData[1] = 0x80 | ((codepoint >> 6) & 0x3F),
        testData[2] = 0x80 | (codepoint & 0x3F), testWhitespaceAtCP();

    testLog << "Found " << std::dec << count << " whitespace characters; expected 25.\n";

    return count != 25;
}

static bool testOutputFileExistsDefault() {
    std::string errMsg;
    const char* inputFile = "./ioFiles/rawFiles/cli-options.cpp";
    const char* tsvFile = "./ioFiles/rawFiles/cli-options.tsv";
    const char* outputFile = "./ioFiles/forceOverwrite/existingFile.cpp";
    const char* argv[] = {"./test", "mutate", "-i", inputFile, "-m", tsvFile, "-o", outputFile, nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    bool caughtException = false;
    try {
        execMutate(&parsedArgs, &nonpositionals);
    } catch (const std::exception& ex) {
        errMsg = std::string(ex.what());
        caughtException = true;
    }
    std::ostringstream os;
    os << "Output file \'" << outputFile << "\' already exists. Use \'-F\' to force overwrite.";
    std::string expected{sanitizeOutputMessage(os.str())};

    if (caughtException) {
        testLog << INDENT << "Expected the following what() from exception: " << expected << "\n";
        testLog << INDENT << "Received the following: " << errMsg << "\n";
    }
    else {
        testLog << INDENT << "Expected std::exception to be thrown and none was thrown.\n";
    }

    return (expected != errMsg);
}

static bool testOverwriteFlag() {
    std::string errMsg;
    const char* inputFile = "./ioFiles/rawFiles/cli-options.cpp";
    const char* tsvFile = "./ioFiles/rawFiles/cli-options.tsv";
    const char* outputFile = "./ioFiles/forceOverwrite/existingFile.cpp";
    const char* argv[] = {"./test", "mutate", "-i", inputFile, "-m", tsvFile, "-o", outputFile, "-F", nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    bool caughtException = false;
    try {
        execMutate(&parsedArgs, &nonpositionals);
    } catch (const std::exception& ex) {
        errMsg = std::string(ex.what());
        caughtException = true;
    }
    testLog << INDENT << "Expected no std::exception to be thrown and one was thrown.\n";
    testLog << INDENT << "Received the following: " << errMsg << "\n";

    return caughtException;
}

static bool testCaptureSingleLineTSVRows() {
    const char* tsvFile = "./ioFiles/captureRows/singleLines/cli-options.tsv";
    const char* argv[] = {"./test", "mutate", "-m", tsvFile, nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    MutationsRetriever mRetriever{parsedArgs.getTsvString()};
    int expectedLineCount = 154;
    int receivedLineCount = static_cast<int>(mRetriever.getRows().size());
    testLog << INDENT << "Expected to capture " << expectedLineCount << " rows in TSV file.\n";
    testLog << INDENT << "Instead " << receivedLineCount << " rows were captured.\n";
    return expectedLineCount != receivedLineCount;
}

static bool testCaptureMultipleLineTSVRows() {
    const char* tsvFile = "./ioFiles/captureRows/multipleLines/cli-options.tsv";
    const char* argv[] = {"./test", "mutate", "-m", tsvFile, nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    MutationsRetriever mRetriever{parsedArgs.getTsvString()};
    int expectedLineCount = 157;
    int receivedLineCount = static_cast<int>(mRetriever.getRows().size());
    testLog << INDENT << "Expected to capture " << expectedLineCount << " rows in TSV file.\n";
    testLog << INDENT << "Instead " << receivedLineCount << " rows were captured.\n";
    return expectedLineCount != receivedLineCount;
}

// helper function
static bool compareOutcome(bool caughtException, std::string expected, std::string received) {
    if (caughtException) {
        testLog << INDENT << "Expected the following what() from exception: " << expected << "\n";
        testLog << INDENT << "Received the following: " << received << "\n";
    }
    else {
        testLog << INDENT << "Expected std::exception to be thrown and none was thrown.\n";
    }
    return expected != received;
}

// helper function
static bool testMutationsRetrieverException(const char* tsvFile, std::string expected) {
    const char* argv[] = {"./test", "mutate", "-m", tsvFile, nullptr};
    parsingBoilerPlate bp(argv);
    auto& [parsedArgs, nonpositionals, status] = bp;

    MutationsRetriever mRetriever{parsedArgs.getTsvString()};
    std::string errMsg;
    bool caughtException = false;

    try {
        mRetriever.getPossibleMutations();
    } catch (const std::exception& ex) {
        errMsg = std::string(ex.what());
        caughtException = true;
    }

    return compareOutcome(caughtException, expected, errMsg);
}

static bool indentationCheck() {
    const char* tsvFile = "./ioFiles/checkTSVIndentation/cli-options.tsv";
    std::ostringstream os;
    os << " Error : Indentation detected.\n"
       << "Notice :\n    Cells in TSV format should not be indented.\n"
       << "    Indentation found at row 129 of TSV File." << std::endl;
    std::string expected{os.str()};

    return testMutationsRetrieverException(tsvFile, expected);
}

static bool checkQuotedCellEndings() {
    const char* tsvFile = "./ioFiles/quotedCells/endOfCell/cli-options.tsv";
    std::ostringstream os;
    os << " Error : Invalid syntax found at index 27 of line number 7 in TSV\n"
       << "Notice :\n    Currently found in your TSV : ... \"['SPACE']...\n"
       << "    Expected to be found in TSV : ... \"['TAB']...\n"
       << "\nIf index 26 is not intended end of quoted cell, "
       << "check preceding section of the row beginning with pattern cell on "
          "line number 5\nfor any extra or missing "
       << "QUOTATION MARKS and/or TABs as they are likely cause of error." << std::endl;
    std::string expected{os.str()};

    return testMutationsRetrieverException(tsvFile, expected);
}

static bool checkTerminatingQuoteException() {
    const char* tsvFile = "./ioFiles/quotedCells/endOfFile/ODV1.tsv";
    std::ostringstream os;
    os << " Error : Terminating quote missing.\n"
       << "Notice :\n    Cells beginning with QUOTATION MARK must end with "
       << "QUOTATION MARK.\n"
       << "    Final cell of row beginning on line number 109 missing terminating QUOTATION MARK." << std::endl;
    std::string expected{os.str()};

    return testMutationsRetrieverException(tsvFile, expected);
}

static bool verifyHasPermutations() {
    const char* tsvFile = "./ioFiles/hasData/hasPermutations/ODV1.tsv";
    std::ostringstream os;
    os << " Error : Permutation cell missing in TSV File.\n"
       << "Notice :\n    Missing permutation cell on line number 87\n    Row that begins with pattern cell on line "
          "number 87 has no corresponding permutation cell(s)."
       << std::endl;
    std::string expected{os.str()};

    return testMutationsRetrieverException(tsvFile, expected);
}

static bool verifyHasMutations() {
    const char* tsvFile = "./ioFiles/hasData/hasMutations/ODV1.tsv";
    std::ostringstream os;
    os << "No mutations found in TSV file.";
    std::string expected{os.str()};

    return testMutationsRetrieverException(tsvFile, expected);
}

static bool verifyGrouping(const char* tsvFile) {
    // true will be returned if any of these tests return true
    return 0 <
           (0 + patternOperatorsTest(tsvFile, {1}, {5, 1}) + patternOperatorsTest(tsvFile, {2}, {5, 2, 1}) +
            patternOperatorsTest(tsvFile, {3}, {5, 4, 3, 2, 1}) + patternOperatorsTest(tsvFile, {4}, {5, 4, 3, 2, 1}) +
            patternOperatorsTest(tsvFile, {5}, {5, 1}) + patternOperatorsTest(tsvFile, {6}, {6, 5, 1}));
}

static bool checkNesting() {
    const char* tsvFile = "./ioFiles/specialChars/nesting/specialChars.tsv";
    std::ostringstream os;
    os << " Error : Invalid group nesting syntax in TSV File.\n"
       << "Notice :\n     Nested pattern cell in row number 4 has no corresponding parent." << std::endl;
    std::string expected{os.str()};

    return testMutationsRetrieverException(tsvFile, expected);
}

// static bool verifyNegatedSelection(const char* tsvFile) {
//     patternOperatorsTest(tsvFile, {}, {});
//     patternOperatorsTest(tsvFile, {}, {});
// }

int main(int argc, const char** argv) {
    (void)argc;
    (void)argv;

    POOR_MANS_TEST("Double-dash (--) ends options and starts positional arguments", testDoubleDashPositionalArgs);

    POOR_MANS_TEST("Parse --seed CLI option", testSeedParsing);

    POOR_MANS_TEST("Read --input src /dev/null empty file", testSrcDevNullInput);

    POOR_MANS_TEST("Read --mutations tsv real file", testTsvRealFileInput);

    POOR_MANS_TEST("Insertion Operator Test", insertionOperatorTest);

    POOR_MANS_TEST("Test isWhiteSpace() function", bruteForceUnicodeWhitespaceUnitTest);

    POOR_MANS_TEST("Check default is to not overWrite existing files", testOutputFileExistsDefault);

    POOR_MANS_TEST("Test overwrite flag", testOverwriteFlag);

    POOR_MANS_TEST("Capturing single line TSV rows", testCaptureSingleLineTSVRows);

    POOR_MANS_TEST("Capturing multiple line TSV rows", testCaptureMultipleLineTSVRows);

    POOR_MANS_TEST("Verify has mutations", verifyHasMutations);

    POOR_MANS_TEST("Check TSV indentation", indentationCheck);

    POOR_MANS_TEST("Check quoted cell endings", checkQuotedCellEndings);

    POOR_MANS_TEST("Verify has permutations", verifyHasPermutations);

    POOR_MANS_TEST("Check terminating quote exception", checkTerminatingQuoteException);

    POOR_MANS_TEST("Verify grouping", verifyGrouping, "./ioFiles/specialChars/grouping/specialChars.tsv");

    POOR_MANS_TEST("Check nesting", checkNesting);

    // POOR_MANS_TEST("Verify negated selection", verifyNegatedSelection,
    //                "./ioFiles/specialChars/negating/specialChars.tsv");

    printFailedTestResults();

    return 0;
}
