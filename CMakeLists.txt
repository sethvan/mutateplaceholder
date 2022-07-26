cmake_minimum_required(VERSION 3.16)

project( mutateplaceholder LANGUAGES "CXX" )

add_executable( mutateplaceholder 
src/common.cpp 
src/iohelpers.cpp 
src/commands/validate/validateCommand.cpp
src/commands/cli-options.cpp 
src/commands/score/scoreCommand.cpp 
src/commands/highlight/highlightCommand.cpp 
src/commands/cli-parser.cpp 
src/chacharng/seedHelper.cpp 
src/chacharng/chacharng.cpp 
src/commands/mutate/mutationsRetriever.cpp 
src/commands/mutate/mutator.cpp 
src/commands/mutate/mutationsSelector.cpp 
src/commands/mutate/mutateCommand.cpp 
src/main.cpp )

target_include_directories( mutateplaceholder PRIVATE include )
target_compile_features( mutateplaceholder PRIVATE cxx_std_17)
target_link_options( mutateplaceholder PRIVATE -lpcre2-8 -fsanitize=address )
target_compile_options( mutateplaceholder PRIVATE 
	-Wall -Wextra -Werror -Wl,-z,defs  -lpcre2-8 -fwrapv
	$<$<CONFIG:Debug>:
		-D_GLIBCXX_ASSERTIONS=1 -ggdb3 -fno-omit-frame-pointer -faas -fasynchronous-unwind-tables -fsanitize=address -fstack-protector-all 
	>
)

