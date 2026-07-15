# Simple build: compile every .c in src/ into one binary.
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -Iinclude
SRC     := $(wildcard src/*.c)
BIN     := datalogger

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

# Build, then run the end-to-end demo.
demo: $(BIN)
	@printf '\n=== 1. log 8 samples ===\n'
	./$(BIN) log 8
	@printf '\n=== 2. dump + verify (all good) ===\n'
	./$(BIN) dump
	@printf '\n=== 3. automated verification report (expect PASS) ===\n'
	python3 tools/parse_logs.py ./$(BIN)
	@printf '\n=== 4. inject a bit error and re-verify (expect FAIL detected) ===\n'
	./$(BIN) corrupt 20
	-python3 tools/parse_logs.py ./$(BIN)
	@printf '\n(the FAIL above is the CRC correctly catching the injected error)\n'

clean:
	rm -f $(BIN) flash.bin

.PHONY: demo clean
