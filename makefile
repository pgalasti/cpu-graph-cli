CC=g++
INCLUDE_DIRS=-I ./include -I /usr/local/include
OPTIONS=-std=c++17
POST_OPS=-lpthread -lg-lib -L/usr/local/lib -Wl,-rpath,/usr/local/lib
REQUIRED_FILES=./src/main.cpp ./src/CpuParser.cpp ./src/GraphOutputCli.cpp
OUTPUT=cpu-graph-cli
INSTALL_DIR=~/tools
$(OUTPUT): $(REQUIRED_FILES)
	$(info Building cpu-graph-cli executable...)
	$(CC) $(OPTIONS) $(INCLUDE_DIRS) -o $(OUTPUT) $(REQUIRED_FILES) $(POST_OPS)

clean:
	$(info Removing executable...)
	rm $(OUTPUT)

install: $(OUTPUT)
	$(info Installing executable...)
	cp $(OUTPUT) $(INSTALL_DIR)
