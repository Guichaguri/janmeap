
#include "nmea.h"

static void nmea_process(struct nmea_reader *reader);
static int hex2int(char c);

void nmea_reader_init(nmea_reader_t *reader, nmea_process_message_t process_message) {
	reader->length = 0;
	reader->buffer_start = 0;
	reader->process_message = process_message;
	reader->process_error = NULL;
}

void nmea_reader_set_error_callback(nmea_reader_t *reader, nmea_process_error_t process_error) {
	reader->process_error = process_error;
}

void nmea_reader_add_char(nmea_reader_t *reader, char c) {
  if (reader->length + 1 >= NMEA_BUFFER_MAX_LENGTH) {
    // The buffer is too big, we'll discard the current message and start over
    reader->length = 0;

		// Dispatch an error
		if (reader->process_error != NULL) {
    	reader->process_error(NMEA_ERROR_BUFFER_OVERFLOW, reader->buffer, NMEA_BUFFER_MAX_LENGTH - 1);
		}
  }

  *(reader->buffer + reader->length) = c;
  reader->length++;

  return nmea_process(reader);
}

void nmea_reader_clear(nmea_reader_t *reader) {
	reader->buffer_start = 0;
	reader->length = 0;
}

static void nmea_process(nmea_reader_t *reader) {
	while (reader->buffer[reader->buffer_start] != '$') {
		reader->buffer_start++;

		if (reader->buffer_start >= reader->length) {
			// No start of message found yet, we'll need to buffer more
			return;
		}
	}

	uint16_t gps_buffer_end = reader->buffer_start;

	while (reader->buffer[gps_buffer_end] != '*') {
		gps_buffer_end++;

		if (gps_buffer_end + 2 >= reader->length) { // 2 = checks for the two hex characters after the *
			// No end of message found yet, we'll need to buffer more
			return;
		}
	}

	// Calculates the message checksum
	uint8_t checksum = 0;
	for (int i = reader->buffer_start + 1; i < gps_buffer_end; i++) {
		checksum ^= reader->buffer[i];
	}

	uint8_t chk = hex2int(reader->buffer[gps_buffer_end + 1]) << 4 | hex2int(reader->buffer[gps_buffer_end + 2]);

	// $GNGGA,....
	char *message = reader->buffer + reader->buffer_start + 3; // 3 = skips $GN
	int size = gps_buffer_end - reader->buffer_start - 3;

	// Reset counts to read a new message
	reader->length = 0;
	reader->buffer_start = 0;

	if (checksum != chk) {
		// Checksum doesn't match, we can't trust the data
		if (reader->process_error != NULL) {
    	reader->process_error(NMEA_ERROR_CHECKSUM, message, size);
		}

		return;
	}

	reader->process_message(message, size);
}

static int hex2int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}
