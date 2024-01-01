CC				= g++
CC_FLAGS 		= -g -Wall -std=c++14
BUILD_DIR		= ./bin
SRC_DIR			= ./src
LIB_DIR			= ./lib

build:
	$(CC) $(CC_FLAGS) -o $(BUILD_DIR)/gitc $(SRC_DIR)/gitc.cpp $(LIB_DIR)/*.cpp
	@echo "Build Complete"

clean:
	rm -r $(BUILD_DIR)/*

run:
	$(BUILD_DIR)/gitc
