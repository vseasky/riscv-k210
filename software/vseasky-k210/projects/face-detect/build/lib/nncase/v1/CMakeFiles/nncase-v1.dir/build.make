# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = D:\SoftwareTools\CMake\bin\cmake.exe

# The command to remove a file.
RM = D:\SoftwareTools\CMake\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = E:\github\k210\software\vseasky-k210\kendryte-standalone-sdk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\github\k210\software\vseasky-k210\projects\face_detect\build

# Include any dependencies generated for this target.
include lib/nncase/v1/CMakeFiles/nncase-v1.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include lib/nncase/v1/CMakeFiles/nncase-v1.dir/compiler_depend.make

# Include the progress variables for this target.
include lib/nncase/v1/CMakeFiles/nncase-v1.dir/progress.make

# Include the compile flags for this target's objects.
include lib/nncase/v1/CMakeFiles/nncase-v1.dir/flags.make

lib/nncase/v1/CMakeFiles/nncase-v1.dir/nncase_v1.cpp.obj: lib/nncase/v1/CMakeFiles/nncase-v1.dir/flags.make
lib/nncase/v1/CMakeFiles/nncase-v1.dir/nncase_v1.cpp.obj: E:/github/k210/software/vseasky-k210/kendryte-standalone-sdk/lib/nncase/v1/nncase_v1.cpp
lib/nncase/v1/CMakeFiles/nncase-v1.dir/nncase_v1.cpp.obj: lib/nncase/v1/CMakeFiles/nncase-v1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\k210\software\vseasky-k210\projects\face_detect\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/nncase/v1/CMakeFiles/nncase-v1.dir/nncase_v1.cpp.obj"
	cd /d E:\github\k210\software\vseasky-k210\projects\face_detect\build\lib\nncase\v1 && D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/nncase/v1/CMakeFiles/nncase-v1.dir/nncase_v1.cpp.obj -MF CMakeFiles\nncase-v1.dir\nncase_v1.cpp.obj.d -o CMakeFiles\nncase-v1.dir\nncase_v1.cpp.obj -c E:\github\k210\software\vseasky-k210\kendryte-standalone-sdk\lib\nncase\v1\nncase_v1.cpp

lib/nncase/v1/CMakeFiles/nncase-v1.dir/nncase_v1.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nncase-v1.dir/nncase_v1.cpp.i"
	cd /d E:\github\k210\software\vseasky-k210\projects\face_detect\build\lib\nncase\v1 && D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\github\k210\software\vseasky-k210\kendryte-standalone-sdk\lib\nncase\v1\nncase_v1.cpp > CMakeFiles\nncase-v1.dir\nncase_v1.cpp.i

lib/nncase/v1/CMakeFiles/nncase-v1.dir/nncase_v1.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nncase-v1.dir/nncase_v1.cpp.s"
	cd /d E:\github\k210\software\vseasky-k210\projects\face_detect\build\lib\nncase\v1 && D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S E:\github\k210\software\vseasky-k210\kendryte-standalone-sdk\lib\nncase\v1\nncase_v1.cpp -o CMakeFiles\nncase-v1.dir\nncase_v1.cpp.s

# Object files for target nncase-v1
nncase__v1_OBJECTS = \
"CMakeFiles/nncase-v1.dir/nncase_v1.cpp.obj"

# External object files for target nncase-v1
nncase__v1_EXTERNAL_OBJECTS =

lib/nncase/v1/libnncase-v1.a: lib/nncase/v1/CMakeFiles/nncase-v1.dir/nncase_v1.cpp.obj
lib/nncase/v1/libnncase-v1.a: lib/nncase/v1/CMakeFiles/nncase-v1.dir/build.make
lib/nncase/v1/libnncase-v1.a: lib/nncase/v1/CMakeFiles/nncase-v1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=E:\github\k210\software\vseasky-k210\projects\face_detect\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libnncase-v1.a"
	cd /d E:\github\k210\software\vseasky-k210\projects\face_detect\build\lib\nncase\v1 && $(CMAKE_COMMAND) -P CMakeFiles\nncase-v1.dir\cmake_clean_target.cmake
	cd /d E:\github\k210\software\vseasky-k210\projects\face_detect\build\lib\nncase\v1 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\nncase-v1.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/nncase/v1/CMakeFiles/nncase-v1.dir/build: lib/nncase/v1/libnncase-v1.a
.PHONY : lib/nncase/v1/CMakeFiles/nncase-v1.dir/build

lib/nncase/v1/CMakeFiles/nncase-v1.dir/clean:
	cd /d E:\github\k210\software\vseasky-k210\projects\face_detect\build\lib\nncase\v1 && $(CMAKE_COMMAND) -P CMakeFiles\nncase-v1.dir\cmake_clean.cmake
.PHONY : lib/nncase/v1/CMakeFiles/nncase-v1.dir/clean

lib/nncase/v1/CMakeFiles/nncase-v1.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" E:\github\k210\software\vseasky-k210\kendryte-standalone-sdk E:\github\k210\software\vseasky-k210\kendryte-standalone-sdk\lib\nncase\v1 E:\github\k210\software\vseasky-k210\projects\face_detect\build E:\github\k210\software\vseasky-k210\projects\face_detect\build\lib\nncase\v1 E:\github\k210\software\vseasky-k210\projects\face_detect\build\lib\nncase\v1\CMakeFiles\nncase-v1.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : lib/nncase/v1/CMakeFiles/nncase-v1.dir/depend

