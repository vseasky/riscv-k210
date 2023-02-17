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
CMAKE_SOURCE_DIR = E:\github\riscv-k210\software\k210-standalone\kendryte-standalone-sdk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build

# Include any dependencies generated for this target.
include CMakeFiles/k210-yolo.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/k210-yolo.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/k210-yolo.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/k210-yolo.dir/flags.make

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\image_process.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\image_process.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\board\camera\image_process.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\board\camera\image_process.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\image_process.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\board\camera\image_process.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\image_process.c.s

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\ov2640.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\ov2640.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\board\camera\ov2640.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\board\camera\ov2640.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\ov2640.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\board\camera\ov2640.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\ov2640.c.s

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\ov5640.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\ov5640.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\board\camera\ov5640.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\board\camera\ov5640.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\ov5640.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\board\camera\ov5640.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\camera\ov5640.c.s

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\flash\w25qxx.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\flash\w25qxx.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\board\flash\w25qxx.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\board\flash\w25qxx.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\flash\w25qxx.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\board\flash\w25qxx.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\flash\w25qxx.c.s

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/board/gpio.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\gpio.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\gpio.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\board\gpio.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\board\gpio.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\gpio.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\board\gpio.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\gpio.c.s

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\lcd\lcd.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\lcd\lcd.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\board\lcd\lcd.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\board\lcd\lcd.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\lcd\lcd.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\board\lcd\lcd.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\lcd\lcd.c.s

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\lcd\nt35310.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\lcd\nt35310.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\board\lcd\nt35310.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\board\lcd\nt35310.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\lcd\nt35310.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\board\lcd\nt35310.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\lcd\nt35310.c.s

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\region_layer\region_layer.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\region_layer\region_layer.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\board\region_layer\region_layer.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\board\region_layer\region_layer.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\region_layer\region_layer.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\board\region_layer\region_layer.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\board\region_layer\region_layer.c.s

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.obj: CMakeFiles/k210-yolo.dir/flags.make
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.obj: E:/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c
CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.obj: CMakeFiles/k210-yolo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building C object CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.obj"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.obj -MF CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\k210-yolo\src\main.c.obj.d -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\k210-yolo\src\main.c.obj -c E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\src\main.c

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.i"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\src\main.c > CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\k210-yolo\src\main.c.i

CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.s"
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\src\main.c -o CMakeFiles\k210-yolo.dir\E_\github\riscv-k210\software\k210-standalone\projects\k210-yolo\src\main.c.s

# Object files for target k210-yolo
k210__yolo_OBJECTS = \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.obj" \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.obj" \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.obj" \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.obj" \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.obj" \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.obj" \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.obj" \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.obj" \
"CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.obj"

# External object files for target k210-yolo
k210__yolo_EXTERNAL_OBJECTS =

k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/image_process.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov2640.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/camera/ov5640.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/flash/w25qxx.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/gpio.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/lcd.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/lcd/nt35310.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/board/region_layer/region_layer.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/E_/github/riscv-k210/software/k210-standalone/projects/k210-yolo/src/main.c.obj
k210-yolo: CMakeFiles/k210-yolo.dir/build.make
k210-yolo: lib/libkendryte.a
k210-yolo: lib/nncase/libnncase-wrapper.a
k210-yolo: lib/nncase/v0/libnncase-v0.a
k210-yolo: lib/nncase/v1/libnncase-v1.a
k210-yolo: E:/github/riscv-k210/software/k210-standalone/kendryte-standalone-sdk/lib/nncase/v1/lib/libnncase.rt_modules.k210.a
k210-yolo: E:/github/riscv-k210/software/k210-standalone/kendryte-standalone-sdk/lib/nncase/v1/lib/libnncase.runtime.a
k210-yolo: CMakeFiles/k210-yolo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Linking C executable k210-yolo"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\k210-yolo.dir\link.txt --verbose=$(VERBOSE)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating .bin file ..."
	D:\SoftwareTools\kendryte-toolchain\bin\riscv64-unknown-elf-objcopy.exe --output-format=binary E:/github/riscv-k210/software/k210-standalone/projects/k210-yolo/build/k210-yolo E:/github/riscv-k210/software/k210-standalone/projects/k210-yolo/build/k210-yolo.bin

# Rule to build all files generated by this target.
CMakeFiles/k210-yolo.dir/build: k210-yolo
.PHONY : CMakeFiles/k210-yolo.dir/build

CMakeFiles/k210-yolo.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\k210-yolo.dir\cmake_clean.cmake
.PHONY : CMakeFiles/k210-yolo.dir/clean

CMakeFiles/k210-yolo.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" E:\github\riscv-k210\software\k210-standalone\kendryte-standalone-sdk E:\github\riscv-k210\software\k210-standalone\kendryte-standalone-sdk E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build E:\github\riscv-k210\software\k210-standalone\projects\k210-yolo\build\CMakeFiles\k210-yolo.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/k210-yolo.dir/depend
