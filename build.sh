#!/bin/sh

OPT="-O2"
# OPT="-g"

# clang++ -std=c++2b *.cpp $OPT -o 23a.out
# clang++ -std=c++17 *.cpp $OPT -o 17a.out

# g++ -std=c++23 -E *.cpp  # see code after preprocessor
# g++ -std=c++23 -dM -E *.cpp  # see all defined macros after
g++ -std=c++23 *.cpp $OPT -o 23a.out
g++ -std=c++17 *.cpp $OPT -o 17a.out
