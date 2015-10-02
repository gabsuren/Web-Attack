

CC := g++

SOURCE := server.cpp webattack.cpp 
EXE := webattack

all: build

build: $(EXE)

$(EXE): $(SOURCE)
	@$(CC) $^ -o $@  -lxml2 -lcurl

clean:
	rm -rf $(EXE)
