# STRUCTURE

SRC_DIR := ./src
INC_DIR := ./inc
APP_DIR := ./app
TEST_DIR := ./test

# FILES

MODE := app
SRC := $(wildcard $(SRC_DIR)/*.c) # list of all the C source files in the SRC_DIR directory

ifeq ($(MODE), test) # SRC + list of all the C source files in TEST_DIR directory
	APP_SRC := $(SRC) $(wildcard $(TEST_DIR)/*.c)
	TARGET := $(TEST_DIR)/test.out
else # SRC + list of all the C source files in APP_DIR directory
	APP_SRC := $(SRC) $(wildcard $(APP_DIR)/*.c)
	TARGET := main.out
endif

OBJ := $(APP_SRC:%.c=%.o)	# for each file in APP_SRC replace .c with .o 
DEPS := $(OBJ:%.o=%.d) 		# for each file in OBJ replace .c with .o   

# LIBRARIES

LIB := pthread

# COMPILATION

CC ?= gcc													# default compiler
C_FLAGS := -Wall -Wextra -Werror 	# compiler flags
DEP_FLAGS := -MMD -MP							# automatically generate .d files

INC_INC := $(foreach d, $(INC_DIR), -I$d)
LIB_INC := $(foreach l, $(LIB), -l$l)

# is CC is clang, add -Weverything to flags
ifeq ($(CC),clang)
	C_FLAGS += -Weverything
endif

# Set -O (optimalization) flag if O=... is given, default -O3
ifeq ("$(origin O)", "command line")
	OPT := -O$(O)
else
	OPT := -O3
endif

# Set -G flag if G=... is given, default=nothing
ifeq ("$(origin G)", "command line")
	GGDB := -ggdb$(G)
else
	GGDB :=
endif

C_FLAGS += $(OPT) $(GGDB) $(DEP_FLAGS)

#

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(C_FLAGS) $(INC_INC) $(OBJ) -o $@ $(LIB_INC)

%.o:%.c %.d
	$(CC) $(C_FLAGS) $(INC_INC) -c $< -o $@

clean:
	rm -rf $(EXE)
	rm -rf $(OBJ)
	rm -rf $(DEPS)

$(DEPS):
include $(wildcard $(DEPS))
