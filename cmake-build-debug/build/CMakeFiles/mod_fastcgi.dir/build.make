# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/renyu/workspace/opensc/lighttpd-1.4.41

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug

# Include any dependencies generated for this target.
include build/CMakeFiles/mod_fastcgi.dir/depend.make

# Include the progress variables for this target.
include build/CMakeFiles/mod_fastcgi.dir/progress.make

# Include the compile flags for this target's objects.
include build/CMakeFiles/mod_fastcgi.dir/flags.make

build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o: build/CMakeFiles/mod_fastcgi.dir/flags.make
build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o: ../src/mod_fastcgi.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o"
	cd /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/build && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o   -c /Users/renyu/workspace/opensc/lighttpd-1.4.41/src/mod_fastcgi.c

build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.i"
	cd /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/build && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/renyu/workspace/opensc/lighttpd-1.4.41/src/mod_fastcgi.c > CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.i

build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.s"
	cd /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/build && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/renyu/workspace/opensc/lighttpd-1.4.41/src/mod_fastcgi.c -o CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.s

build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o.requires:

.PHONY : build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o.requires

build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o.provides: build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o.requires
	$(MAKE) -f build/CMakeFiles/mod_fastcgi.dir/build.make build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o.provides.build
.PHONY : build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o.provides

build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o.provides.build: build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o


# Object files for target mod_fastcgi
mod_fastcgi_OBJECTS = \
"CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o"

# External object files for target mod_fastcgi
mod_fastcgi_EXTERNAL_OBJECTS =

build/mod_fastcgi.so: build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o
build/mod_fastcgi.so: build/CMakeFiles/mod_fastcgi.dir/build.make
build/mod_fastcgi.so: build/CMakeFiles/mod_fastcgi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared module mod_fastcgi.so"
	cd /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mod_fastcgi.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
build/CMakeFiles/mod_fastcgi.dir/build: build/mod_fastcgi.so

.PHONY : build/CMakeFiles/mod_fastcgi.dir/build

build/CMakeFiles/mod_fastcgi.dir/requires: build/CMakeFiles/mod_fastcgi.dir/mod_fastcgi.c.o.requires

.PHONY : build/CMakeFiles/mod_fastcgi.dir/requires

build/CMakeFiles/mod_fastcgi.dir/clean:
	cd /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/build && $(CMAKE_COMMAND) -P CMakeFiles/mod_fastcgi.dir/cmake_clean.cmake
.PHONY : build/CMakeFiles/mod_fastcgi.dir/clean

build/CMakeFiles/mod_fastcgi.dir/depend:
	cd /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/renyu/workspace/opensc/lighttpd-1.4.41 /Users/renyu/workspace/opensc/lighttpd-1.4.41/src /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/build /Users/renyu/workspace/opensc/lighttpd-1.4.41/cmake-build-debug/build/CMakeFiles/mod_fastcgi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : build/CMakeFiles/mod_fastcgi.dir/depend

