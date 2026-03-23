UNAME_S := $(shell uname -s)

CC=g++
INCLUDE_DIRS=-I ./include -I /usr/local/include
OPTIONS=-std=c++17
OUTPUT=cpu-graph-cli
INSTALL_DIR=~/tools

ifeq ($(UNAME_S),Darwin)
    PLATFORM_SRC=./src/MacOsMetricsProvider.cpp
    POST_OPS=-lg-lib -L/usr/local/lib -Wl,-rpath,/usr/local/lib
else
    PLATFORM_SRC=./src/LinuxMetricsProvider.cpp
    POST_OPS=-lpthread -lg-lib -L/usr/local/lib -Wl,-rpath,/usr/local/lib
endif

REQUIRED_FILES=./src/main.cpp $(PLATFORM_SRC) ./src/GraphOutputCli.cpp

$(OUTPUT): $(REQUIRED_FILES)
	$(info Building cpu-graph-cli for $(UNAME_S)...)
	$(CC) $(OPTIONS) $(INCLUDE_DIRS) -o $(OUTPUT) $(REQUIRED_FILES) $(POST_OPS)

clean:
	$(info Removing executable...)
	rm $(OUTPUT)

install: $(OUTPUT)
	$(info Installing executable...)
	mkdir -p $(INSTALL_DIR)
	cp $(OUTPUT) $(INSTALL_DIR)/
