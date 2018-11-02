
CFLAGS += -Wall -Werror

all: spy-files

spy-files: src/main.c src/spy.c

tests: test_spy
	./test_spy

test_spy: CPPFLAGS += -Isrc
test_spy: tests/test_spy.c src/spy.c

spy-files test_spy:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	rm -f src/*.o tests/*.o spy-files test_spy

.PHONY: all test clean
