

CC := g++

SOURCE := server.cpp webattack.cpp 
EXE := webattack

all: build

build: $(EXE)

$(EXE): $(SOURCE)
	@$(CC) $^ -o $@ -I/usr/include/libxml2 -lxml2 -lcurl
