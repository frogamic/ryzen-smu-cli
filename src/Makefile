CC := gcc
LIBSMU_DIR ?= ../ryzen_smu/lib
VERSION ?= 0.0.1
TARGET ?= rsmuctl
SRCS := $(wildcard *.c) ${LIBSMU_DIR}/libsmu.c
CFLAGS := -Wall -O2 -I${LIBSMU_DIR} -DVERSION=\"${VERSION}\" -DTARGET=\"${TARGET}\"

.PHONY: all install clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)
