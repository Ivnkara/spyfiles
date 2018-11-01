
CFLAGS += -Wall -Werror

all: spy-files

spy-files: src/main.c src/spy.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^