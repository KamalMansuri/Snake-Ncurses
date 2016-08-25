OBJ=obj
SRC=src

DEPS = $(SRC)/frontend.h $(SRC)/backend.h

LINK_TARGET = snake

CC=gcc
CFLAGS=-w -I$(SRC)
LIBS=-lmenu -lncurses 

SRC_FILES = $(wildcard $(SRC)/*.c)
OBJ_FILES = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRC_FILES))

all : dirs $(LINK_TARGET) 

$(OBJ)/%.o : $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

$(LINK_TARGET) : $(OBJ_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

dirs : 
	mkdir -p $(OBJ)

run :
	./$(LINK_TARGET)

clean :
	rm $(OBJ_FILES) $(LINK_TARGET) high-scores-open.txt high-scores-closed.txt -f

.PHONY : clean

  

