CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Isrc
LDFLAGS = -lm
PYTHON = python

SRCS = src/graph.c src/bfs.c src/dfs.c src/unionfind.c src/dijkstra.c src/faults.c src/cli.c src/api.c src/main.c
TEST_SRCS = src/graph.c src/bfs.c src/dfs.c src/unionfind.c src/dijkstra.c src/faults.c tests/test_c_core.c

ifeq ($(OS),Windows_NT)
    TARGET = analyzer.exe
    TEST_TARGET = test_c_core.exe
    RM = del /Q /F
else
    TARGET = analyzer
    TEST_TARGET = test_c_core
    RM = rm -f
endif

.PHONY: all clean gui test test-c test-api test-all

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_TARGET): $(TEST_SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-$(RM) $(TARGET) $(TEST_TARGET) *.o 2>nul || true

gui: $(TARGET)
	$(PYTHON) server.py

test-c: $(TEST_TARGET)
	./$(TEST_TARGET)

test-api: $(TARGET)
	$(PYTHON) -m unittest discover -s tests -p "test_*.py" -v

test: test-all

test-all: test-c test-api
