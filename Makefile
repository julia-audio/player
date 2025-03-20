BUILD_DIR = ./build
TARGET = $(BUILD_DIR)/player

TARGET_FILES = src/main.c src/wav_reader.c src/miniaudio.c
TARGET_HEADERS = src/wav_reader.h src/miniaudio.h
LDFLAGS = -lm

$(TARGET): $(TARGET_FILES) | $(BUILD_DIR)
	$(CC) $(TARGET_FILES) -o $(TARGET) -Wall -Wextra -pedantic -std=c99 $(LDFLAGS)

$(BUILD_DIR):
	mkdir -pv $(BUILD_DIR)

%.o: %.c $(TARGET_HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(BUILD_DIR)
