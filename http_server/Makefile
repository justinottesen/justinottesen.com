EXE := http_server.exe

CC := clang
CXX := clang++
LD := clang++

CFLAGS := 
CXXFLAGS := -std=c++17
CPPFLAGS := -Wall -Wextra -Werror -g
DEPFLAGS := -MMD -MP
LDFLAGS :=
LDLIBS := 

BIN := bin
OBJ := obj
SRC := src

HEADERS := $(wildcard $(SRC)/*.h $(SRC)/*.hpp)
SOURCES := $(wildcard $(SRC)/*.c $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(wildcard $(SRC)/*.c)) \
					 $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(wildcard $(SRC)/*.cpp))
DEPENDS := $(OBJECTS:.o=.d)

COMPILE.c = $(CC) $(DEPFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o $@
COMPILE.cxx = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCLUDES) -c -o $@
LINK.o = $(LD) $(LDFLAGS) $(LDLIBS) $(OBJECTS) -o $@

.DEFAULT_GOAL = all

.PHONY: all
all: $(BIN)/$(EXE)

$(BIN)/$(EXE): $(SRC) $(OBJ) $(BIN) $(OBJECTS)
	$(LINK.o)

$(SRC):
	mkdir -p $(SRC)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

$(OBJ)/%.o: $(SRC)/%.c
	$(COMPILE.c) $<

$(OBJ)/%.o: $(SRC)/%.cpp
	$(COMPILE.cxx) $<

.PHONY: remake
remake: clean $(BIN)/$(EXE)

.PHONY: run
run: $(BIN)/$(EXE)
	./$(BIN)/$(EXE)

.PHONY: clean
clean:
	$(RM) $(OBJECTS)
	$(RM) $(DEPENDS)
	$(RM) $(BIN)/$(EXE)

.PHONY: reset
reset:
	$(RM) -r $(OBJ)
	$(RM) -r $(BIN)

.PHONY: format
format:
	clang-format src/* -i

-include $(DEPENDS)