#include <stdlib.h>
#include "nmea.h"

static int hex2int(char c);

void nmea_reader_init(nmea_reader_t* reader, nmea_process_message_t process_message) {
	reader->length = 0;
	reader->buffer_head = 0;
	reader->buffer_tail = 0;
	reader->buffer_dirty = false;
	reader->process_message = process_message;
	reader->process_error = NULL;
}

void nmea_reader_set_error_callback(nmea_reader_t* reader, nmea_process_error_t process_error) {
	reader->process_error = process_error;
}

void nmea_reader_process_char(nmea_reader_t* reader, char c) {
	nmea_reader_add_char(reader, c);
	nmea_reader_process(reader);
}

void nmea_reader_add_char(nmea_reader_t* reader, char c) {
	int index = (reader->buffer_head + 1) % NMEA_BUFFER_MAX_LENGTH;

	reader->buffer[reader->buffer_head] = c;
	reader->buffer_head = index;
	reader->buffer_dirty = true;

	if (reader->length == NMEA_BUFFER_MAX_LENGTH) {
		reader->buffer_tail = (reader->buffer_tail + 1) % NMEA_BUFFER_MAX_LENGTH;

		// Dispatch an error
		if (reader->process_error != NULL) {
			// TODO check error handling
			reader->process_error(NMEA_ERROR_BUFFER_OVERFLOW, reader->buffer, NMEA_BUFFER_MAX_LENGTH - 1);
		}
	}
	else {
		reader->length++;
	}
}

void nmea_reader_clear(nmea_reader_t* reader) {
	reader->buffer_head = 0;
	reader->buffer_tail = 0;
	reader->length = 0;
	reader->buffer_dirty = false;
}

void nmea_reader_process(nmea_reader_t* reader) {
	if (!reader->buffer_dirty) {
		return;
	}

	reader->buffer_dirty = false;

	while (reader->buffer[reader->buffer_tail] != '$') {
		reader->buffer_tail = (reader->buffer_tail + 1) % NMEA_BUFFER_MAX_LENGTH;

		if (reader->buffer_tail == reader->buffer_head) {
			// No start of message found yet, we'll need to buffer more
			return;
		}
	}

	nmea_buffer_index_t gps_buffer_end = reader->buffer_tail;

	while (reader->buffer[gps_buffer_end] != '*') {
		gps_buffer_end = (gps_buffer_end + 1) % NMEA_BUFFER_MAX_LENGTH;

		int distance_from_end = (NMEA_BUFFER_MAX_LENGTH + reader->buffer_head - gps_buffer_end) % NMEA_BUFFER_MAX_LENGTH;

		if (distance_from_end < 3) { // 3 = checks for the two hex characters plus the *
			// No end of message found yet, we'll need to buffer more
			return;
		}
	}

	// Calculates the message checksum and fills the message buffer
	uint8_t checksum = 0;
	nmea_buffer_index_t i = reader->buffer_tail + 1;
	nmea_buffer_index_t msg_index = 0;

	while (i != gps_buffer_end) {
		checksum ^= reader->buffer[i];
		reader->message[msg_index] = reader->buffer[i];

		i = (i + 1) % NMEA_BUFFER_MAX_LENGTH;
		msg_index++;
	}

	reader->message[msg_index] = '\0';

	uint8_t chk = hex2int(reader->buffer[(gps_buffer_end + 1) % NMEA_BUFFER_MAX_LENGTH]) << 4 |
		hex2int(reader->buffer[(gps_buffer_end + 2) % NMEA_BUFFER_MAX_LENGTH]);

	// $GNGGA,....
	char* message = reader->message + 2; // 2 = skips $GN
	int size = msg_index - 2;

	// Reset counts to read a new message
	reader->length = 0;
	reader->buffer_tail = (gps_buffer_end + 3) % NMEA_BUFFER_MAX_LENGTH;

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
