BUILD_DIR = ./build
TARGET = $(BUILD_DIR)/player

TARGET_FILES = src/main.c src/miniaudio.c
TARGET_HEADERS = src/miniaudio.h

PKG_CONFIG_LIBS = $(shell pkg-config --libs taglib ncurses)
PKG_CONFIG_CFLAGS = $(shell pkg-config --cflags taglib ncurses)

CFLAGS = -Wall -Wextra -pedantic -std=c99 $(PKG_CONFIG_CFLAGS)
LDFLAGS = -lm -ltag_c $(PKG_CONFIG_LIBS)

$(TARGET): $(TARGET_FILES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TARGET_FILES) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR):
	mkdir -pv $(BUILD_DIR)

%.o: %.c $(TARGET_HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(BUILD_DIR)
