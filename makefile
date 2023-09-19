# Useful: https://makefiletutorial.com/

# the compiler - we only use cpp files, so only use g++.
CXX := g++
# compiler flags:
#	-g	- this flag adds debugging information to the executable file
CPPFLAGS := -g 

EXT_LIB_DIRS := -L${MILK_INSTALLDIR}/lib/
#	-lImageStreamIO		- Interfacing with milk data streams
LDFLAGS := $(EXT_LIB_DIRS) -lImageStreamIO
EXT_INC_DIRS := -I${MILK_INSTALLDIR}/include/ImageStreamIO/

# =========================

TARGET_EXEC := milk-2-dynamic-optics-dm

BUILD_DIR := ./build
SRC_DIRS := ./src

# Find all the C++ files we want to compile
# Note the single quotes around the * expression. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')

# String substitution for every C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(CPPFLAGS) $(INC_FLAGS) $(EXT_INC_DIRS) -MMD -MP

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@if [ ! -f build/libImageStreamIO.so ]; then ln -s ${MILK_INSTALLDIR}/lib/libImageStreamIO.so build/libImageStreamIO.so; fi

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Custom: Shortcut to run the application with or without input commands.
run: build/$(TARGET_EXEC)
	@(cd build; \
	if [ -z "$(INSTREAM)" ]; then \
		echo "\nEnter an input string using 'make run INSTREAM=<streamname>'.\n"; \
		./$(TARGET_EXEC); \
		EXIT_STATUS=$$?; \
	else \
		./$(TARGET_EXEC) $(INSTREAM); \
		EXIT_STATUS=$$?; \
	fi; \
	if [ $$EXIT_STATUS -ne 0 ]; then \
		echo "Executable returned a non-zero exit status: $$EXIT_STATUS"; \
	fi;)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
