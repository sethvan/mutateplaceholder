
/*
 * iohelper.cpp: All I/O and input arguments validation definitions
 *
 * - The CLIOptions class holds the values of the parsed
     input command line arguments
 * - (note that cli-parser.cpp fills up instances of this class with the actual
     values to be used)
 * - This is one of the few files that reads input files/stdin and writes to
     output files/stdout
 * - This file is also tasked with input argument validation
 * - This file MUST NOT validate the syntax/correctness of input file contents.
     That is a job for elsewhere
 * - This file MUST NOT process default value logic unless its purely internal
 (e.x. defaulting to reading from stdin).
 *   \- Instead, users of this class check hasValue() to determine whether to
     getValue() or defaultValue
 * - NOTICE: This file does have quite a broad scope of things it does, and that
     is okay. If decoupling a group of closely intertwined functionalities into
     separate files would signifigantly increase the complexity and lines of code,
 then its usually fine to lump the group of things together into one file.
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

#include "commands/cli-options.hpp"

#include <errno.h>

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

#include "chacharng/chacharng.hpp"
#include "chacharng/seedHelper.hpp"
#include "common.hpp"
#include "excepts.hpp"
#include "iohelpers.hpp"

[(<<<<< CELL 1-A >>>>>)]
    : srcInput(stdin), tsvInput(stdin), resOutput(stdout), seedInput(nullptr), seedOutput(nullptr) {}

[(<<<<< CELL 2-A >>>>>)]
                                  [(<<<<< CELL 15-B >>>>>)]
    [(<<<<< CELL 4-B >>>>>)]
        [(<<<<< CELL 17-A >>>>>)]
        [(<<<<< CELL 18-C >>>>>)]
        [(<<<<< CELL 91-B >>>>>)]
    }

    *srcOrTsv = std::fopen(path, mode);
    [(<<<<< CELL 8-B >>>>>)]
        [(<<<<< CELL 9-A >>>>>)]
        return;
    }
    else {
        [(<<<<< CELL 22-B >>>>>)]
        [(<<<<< CELL 23-A >>>>>)]
        [(<<<<< CELL 24-B >>>>>)]
        [(<<<<< CELL 91-B >>>>>)]
    }
}

[(<<<<< CELL 14-B >>>>>)]
                                      [(<<<<< CELL 15-B >>>>>)]
    [(<<<<< CELL 20-A >>>>>)]
        [(<<<<< CELL 17-A >>>>>)]
        [(<<<<< CELL 18-C >>>>>)]
        [(<<<<< CELL 91-B >>>>>)]
    }

    *inOrOut = std::fopen(path, mode);
    [(<<<<< CELL 20-A >>>>>)]
        [(<<<<< CELL 21-D >>>>>)]
        return;
    }
    else {
        [(<<<<< CELL 22-B >>>>>)]
        [(<<<<< CELL 23-A >>>>>)]
        [(<<<<< CELL 24-B >>>>>)]
        [(<<<<< CELL 91-B >>>>>)]
    }
}

[(<<<<< CELL 26-A >>>>>)]
    [(<<<<< CELL 33-C >>>>>)]
        [(<<<<< CELL 131-A >>>>>)]
        [(<<<<< CELL 29-C >>>>>)]
        [(<<<<< CELL 36-B >>>>>)]
    }
    setSrcOrTsvInput(&(srcInput), path, "r", _IOFBF, "source code input");
}

[(<<<<< CELL 32-C >>>>>)]
    [(<<<<< CELL 33-C >>>>>)]
        [(<<<<< CELL 131-A >>>>>)]
        [(<<<<< CELL 35-B >>>>>)]
        [(<<<<< CELL 36-B >>>>>)]
    }
    setSrcOrTsvInput(&(tsvInput), path, "r", _IOFBF, "TSV mutations input");
}

[(<<<<< CELL 38-B >>>>>)]

[(<<<<< CELL 39-A >>>>>)]
    setSrcOrTsvInput(&(resOutput), path, "w", _IONBF, "resulting output");
}

[(<<<<< CELL 41-A >>>>>)]
    [(<<<<< CELL 42-A >>>>>)]
        [(<<<<< CELL 50-A >>>>>)]
    }
    setSeedInputOrOutput(&(seedInput), path, "r", _IONBF, "seed input");
}

[(<<<<< CELL 45-B >>>>>)]
    setSeedInputOrOutput(&(seedOutput), path, "w", _IONBF, "seed output");
}

[(<<<<< CELL 47-A >>>>>)]
    [(<<<<< CELL 48-B >>>>>)]

    [(<<<<< CELL 49-A >>>>>)]
        [(<<<<< CELL 50-A >>>>>)]
    }

    [(<<<<< CELL 51-B >>>>>)]

    [(<<<<< CELL 52-B >>>>>)]
        [(<<<<< CELL 53-A >>>>>)]
        [(<<<<< CELL 54-C >>>>>)]
        [(<<<<< CELL 55-A >>>>>)]
    }
}

[(<<<<< CELL 56-B >>>>>)]
                                     [(<<<<< CELL 57-B >>>>>)]
    if (minOrMax->has_value()) {
        [(<<<<< CELL 59-A >>>>>)]
        [(<<<<< CELL 60-A >>>>>)]
        [(<<<<< CELL 91-B >>>>>)]
    }

    [(<<<<< CELL 62-B >>>>>)]
    [(<<<<< CELL 63-C >>>>>)]

    [(<<<<< CELL 64-A >>>>>)]
        [(<<<<< CELL 65-B >>>>>)]
        [(<<<<< CELL 66-B >>>>>)]
        [(<<<<< CELL 67-A >>>>>)]
        [(<<<<< CELL 91-B >>>>>)]
    }

    *minOrMax = (std::int32_t)retStatus;

    [(<<<<< CELL 69-A >>>>>)]
        [(<<<<< CELL 70-E >>>>>)]
            [(<<<<< CELL 71-C >>>>>)]
    }
}

[(<<<<< CELL 72-C >>>>>)]

[(<<<<< CELL 73-A >>>>>)]
    [(<<<<< CELL 74-A >>>>>)]
}

[(<<<<< CELL 75-A >>>>>)]

[(<<<<< CELL 76-D >>>>>)]
    [(<<<<< CELL 77-A >>>>>)]
}

[(<<<<< CELL 78-A >>>>>)]
[(<<<<< CELL 79-D >>>>>)]
    [(<<<<< CELL 80-B >>>>>)]
    return in;
}

[(<<<<< CELL 81-B >>>>>)]
    [(<<<<< CELL 82-B >>>>>)]

    [(<<<<< CELL 83-A >>>>>)]
    [(<<<<< CELL 84-B >>>>>)]

    [(<<<<< CELL 85-A >>>>>)]
    else if (0 == std::strcmp(str.c_str(), "srctext") || 0 == std::strcmp(str.c_str(), "srctxt")) {
        [(<<<<< CELL 86-A >>>>>)]
    }
    else if (0 == std::strcmp(str.c_str(), "tsvtext") || 0 == std::strcmp(str.c_str(), "tsvtxt")) {
        [(<<<<< CELL 87-B >>>>>)]
    }
    else {
        [(<<<<< CELL 88-C >>>>>)]
        [(<<<<< CELL 89-A >>>>>)]
        [(<<<<< CELL 90-A >>>>>)]
        [(<<<<< CELL 91-B >>>>>)]
    }
}

[(<<<<< CELL 92-B >>>>>)]
    [(<<<<< CELL 93-A >>>>>)]
        [(<<<<< CELL 94-D >>>>>)]
    }
    [(<<<<< CELL 95-D >>>>>)]
        [(<<<<< CELL 96-B >>>>>)]
    }
    return srcString.value();
}

[(<<<<< CELL 97-A >>>>>)]
    [(<<<<< CELL 98-E >>>>>)]
    [(<<<<< CELL 99-A >>>>>)]
        [(<<<<< CELL 100-B >>>>>)]
    }
    return tsvString.value();
}

[(<<<<< CELL 101-A >>>>>)]
    [(<<<<< CELL 102-C >>>>>)]
}

[(<<<<< CELL 103-D >>>>>)]

[(<<<<< CELL 104-A >>>>>)]

[(<<<<< CELL 105-B >>>>>)]

[(<<<<< CELL 106-A >>>>>)]

[(<<<<< CELL 107-D >>>>>)]

[(<<<<< CELL 108-A >>>>>)]

[(<<<<< CELL 109-A >>>>>)]

[(<<<<< CELL 110-B >>>>>)]

[(<<<<< CELL 111-A >>>>>)]

[(<<<<< CELL 112-A >>>>>)]

[(<<<<< CELL 113-A >>>>>)]

[(<<<<< CELL 114-B >>>>>)]
    [(<<<<< CELL 115-C >>>>>)]
        [(<<<<< CELL 116-B >>>>>)]
            readSeedFileIntoString(seedInput, &(seedString));
        }
        else {
            return std::string("");
        }

        [(<<<<< CELL 118-C >>>>>)]
        }
    }
    return seedString.value();
}

[(<<<<< CELL 119-A >>>>>)]
[(<<<<< CELL 120-A >>>>>)]
[(<<<<< CELL 121-D >>>>>)]
[(<<<<< CELL 122-A >>>>>)]

[(<<<<< CELL 123-B >>>>>)]
    [(<<<<< CELL 124-D >>>>>)]
    [(<<<<< CELL 125-A >>>>>)]
    [(<<<<< CELL 126-A >>>>>)]
    [(<<<<< CELL 127-B >>>>>)]
    [(<<<<< CELL 128-B >>>>>)]
}

[(<<<<< CELL 129-A >>>>>)]

[(<<<<< CELL 130-B >>>>>)]
    [(<<<<< CELL 131-A >>>>>)]
    [(<<<<< CELL 132-B >>>>>)]

    [(<<<<< CELL 133-C >>>>>)]
        [(<<<<< CELL 134-B >>>>>)]
           [(<<<<< CELL 135-A >>>>>)]
           [(<<<<< CELL 136-A >>>>>)]
        [(<<<<< CELL 137-B >>>>>)]
            [(<<<<< CELL 138-A >>>>>)]
        }
        [(<<<<< CELL 139-B >>>>>)]
    }
    [(<<<<< CELL 140-B >>>>>)]
        [(<<<<< CELL 141-B >>>>>)]
           [(<<<<< CELL 142-A >>>>>)]
           [(<<<<< CELL 143-A >>>>>)]
        [(<<<<< CELL 144-B >>>>>)]
            [(<<<<< CELL 145-A >>>>>)]
        }
        [(<<<<< CELL 146-A >>>>>)]
    }
    [(<<<<< CELL 147-A >>>>>)]
        [(<<<<< CELL 148-A >>>>>)]
        [(<<<<< CELL 149-B >>>>>)]
    }
    [(<<<<< CELL 150-C >>>>>)]
        [(<<<<< CELL 151-B >>>>>)]
        [(<<<<< CELL 152-A >>>>>)]
        return retVal;
    }
    return os.str();
}

[(<<<<< CELL 153-B >>>>>)]

[(<<<<< CELL 154-A >>>>>)]