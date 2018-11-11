
CFLAGS += -Wall -Werror

all: spy-files

spy-files: src/main.c src/spy.c src/helpers.c

tests: test_spy
	./test_spy

test_spy: CPPFLAGS += -Isrc -pthread
test_spy: tests/test_spy.c src/spy.c src/helpers.c

spy-files test_spy:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	rm -f src/*.o tests/*.o spy-files test_spy

.PHONY: all test clean
