BUILD_DIR = ./build
TARGET = $(BUILD_DIR)/player

TARGET_FILES = src/player.c src/wav_reader.c
TARGET_HEADERS = src/wav_reader.h

$(TARGET): $(TARGET_FILES) | $(BUILD_DIR)
	$(CC) $(TARGET_FILES) -o $(TARGET) -Wall -Wextra -pedantic -std=c99

$(BUILD_DIR):
	mkdir -pv $(BUILD_DIR)

%.o: %.c $(TARGET_HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(BUILD_DIR)
