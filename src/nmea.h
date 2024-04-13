#ifndef _JANMEAP_NMEA_H_
#define _JANMEAP_NMEA_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NMEA message buffer max length
 * Defaults to 82 characters, enough space to fit a single message
 */
#ifndef NMEA_MESSAGE_BUFFER_MAX_LENGTH
#define NMEA_MESSAGE_BUFFER_MAX_LENGTH 82
#endif // NMEA_BUFFER_MAX_LENGTH

/**
 * NMEA character buffer max length
 * Defaults to 164 characters, enough space to fit two messages (or partial messages)
 */
#ifndef NMEA_BUFFER_MAX_LENGTH
#define NMEA_BUFFER_MAX_LENGTH (NMEA_MESSAGE_BUFFER_MAX_LENGTH * 2)
#endif // NMEA_BUFFER_MAX_LENGTH

/**
 * Whether it should disable the parser functions
 */
#ifndef NMEA_PARSER
#define NMEA_PARSER 1
#endif

/**
 * Whether it should disable the coordinate utility functions
 */
#ifndef NMEA_PARSER_UTILITIES
#define NMEA_PARSER_UTILITIES 1
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * Represents a coordinate, in DMM format (Degrees and decimal minutes)
 * 
 * Other formats:
 * Decimal degrees (DD): 41.40338, 2.17403
 * Degrees, minutes, and seconds (DMS): 41°24'12.2"N 2°10'26.5"E
 * Degrees and decimal minutes (DMM): 41 24.2028, 2 10.4418
 */
typedef struct {
	uint8_t degrees; // 0-180
	double decimal_minutes; // 0-60
} nmea_coordinate_t;

/**
 * Represents a date.
 * The year is composed of two digits (e.g. 2023 would be 23)
 */
typedef struct {
	uint8_t date; // 1-31
	uint8_t month; // 1-12
	uint8_t year; // 00-99
} nmea_date_t;

/**
 * Represents a timestamp.
 * The number of seconds may have up to two decimal digits.
 */
typedef struct {
	uint8_t hours; // 0-24
	uint8_t minutes; // 0-60
	float seconds;  // 0-60
} nmea_time_t;

/**
 * Represents all types of error the stream can return
 */
typedef enum {
	NMEA_ERROR_CHECKSUM = 1,
	NMEA_ERROR_BUFFER_OVERFLOW = 2
} nmea_error_t;

typedef void (*nmea_process_message_t)(char *message, int length);
typedef void (*nmea_process_error_t)(nmea_error_t error_type, char *message, int length);

#if NMEA_BUFFER_MAX_LENGTH > 255
typedef uint16_t nmea_buffer_index_t;
#else
typedef uint8_t nmea_buffer_index_t;
#endif

/**
 * Represents an NMEA reader instance
 */
typedef struct {
	char buffer[NMEA_BUFFER_MAX_LENGTH];
	char message[NMEA_MESSAGE_BUFFER_MAX_LENGTH];
	nmea_buffer_index_t length;
	nmea_buffer_index_t buffer_head; // head
	nmea_buffer_index_t buffer_tail; // tail
	bool buffer_dirty;
	nmea_process_message_t process_message;
	nmea_process_error_t process_error;
} nmea_reader_t;

/**
 * @brief Initializes the reader
 * 
 * @param reader The reader pointer
 * @param process_message A function pointer to process nmea messages
 */
void nmea_reader_init(nmea_reader_t *reader, nmea_process_message_t process_message);

/**
 * @brief Adds an error callback to the reader.
 * 
 * Checksum and buffer overflow errors will be fowarded to this callback.
 * 
 * @param reader The reader pointer
 * @param process_error The function pointer to receive errors. NULL disables the callback.
 */
void nmea_reader_set_error_callback(nmea_reader_t *reader, nmea_process_error_t process_error);

/**
 * @brief Apprends a character to the nmea buffer
 * 
 * @param reader The reader pointer
 * @param c The character to be appended
 */
void nmea_reader_add_char(nmea_reader_t *reader, char c);

/**
 * @brief Processes characters inside the nmea buffer trying to find messages
 * 
 * @param reader The reader pointer
 */
void nmea_reader_process(nmea_reader_t *reader);

/**
 * @brief Appends and processes a character inside the nmea buffer.
 * 
 * Shorthand for `nmea_reader_add_char(reader, c)` followed by `nmea_reader_process(reader)`.
 * 
 * @param reader The reader pointer
 * @param c The character to be appended
 */
void nmea_reader_process_char(nmea_reader_t *reader, char c);

/**
 * @brief Clears the nmea buffer
 * 
 * @param reader The reader pointer
 */
void nmea_reader_clear(nmea_reader_t *reader);

#if NMEA_PARSER

/**
 * @brief Skips/ignores an NMEA field, moving the message pointer to the next field.
 * 
 * @param message The message pointer
 */
void nmea_skip_field(char **message);

/**
 * @brief Reads a 8 bit unsigned integer
 * 
 * @param message The message pointer
 * @param num The number output
 * @return true when parsing was successful
 */
bool nmea_read_uint8(char **message, uint8_t *num);

/**
 * @brief Reads a 16 bit unsigned integer
 * 
 * @param message The message pointer
 * @param num The number output
 * @return true when parsing was successful
 */
bool nmea_read_uint16(char **message, uint16_t *num);

/**
 * @brief Reads a 32 bit unsigned integer
 * 
 * @param message The message pointer
 * @param num The number output
 * @return true when parsing was successful
 */
bool nmea_read_uint32(char **message, uint32_t *num);

/**
 * @brief Reads a floating point number
 * 
 * @param message The message pointer
 * @param num The number output
 * @return true when parsing was successful
 */
bool nmea_read_float(char **message, float *num);

/**
 * @brief Reads a single character
 * 
 * @param message The message pointer
 * @param c The character output
 * @return true when parsing was successful
 */
bool nmea_read_char(char **message, char *c);

/**
 * @brief Reads a string
 * 
 * @param message The message pointer
 * @param str The string output
 * @param max_length The maximum length of the output string
 * @return The amount of characters read
 */
int nmea_read_string(char **message, char *str, int max_length);

/**
 * @brief Reads a latitude/longitude coordinate in "ddmm.mm" or "dddmm.mm"
 * 
 * @param message The message pointer
 * @param coord The coordinate output
 * @param deg_3_digits true if the format is "dddmm.mm", false if the format is "ddmm.mm"
 * @return true when parsing was successful
 */
bool nmea_read_coordinate(char **message, nmea_coordinate_t *coord, bool deg_3_digits);

/**
 * @brief Read a latitude coordinate in "ddmm.mm".
 * 
 * Shorthand for `nmea_read_coordinate(message, coord, false)`.
 * 
 * @param message The message pointer
 * @param coord The coordinate output
 * @return true when parsing was succesful
 */
bool nmea_read_latitude(char **message, nmea_coordinate_t *coord);

/**
 * @brief Read a longitude coordinate in "dddmm.mm".
 * 
 * Shorthand for `nmea_read_coordinate(message, coord, true)`.
 * 
 * @param message The message pointer
 * @param coord The coordinate output
 * @return true when parsing was succesful
 */
bool nmea_read_longitude(char **message, nmea_coordinate_t *coord);

/**
 * @brief Reads a date in "ddmmyy"
 * 
 * @param message The message pointer
 * @param date The date output
 * @return true when parsing was sucessful
 */
bool nmea_read_date(char **message, nmea_date_t *date);

/**
 * @brief Reads a time in "hhmmss.ss"
 * 
 * @param message The message pointer
 * @param time The time output
 * @return true when parsing was sucessful
 */
bool nmea_read_time(char **message, nmea_time_t *time);

#endif // NMEA_PARSER

#if NMEA_PARSER_UTILITIES

/**
 * @brief Converts a coordinate to Decimal Degrees (DD).
 * 
 * Sample output: 41.40338, 2.17403
 * 
 * @param coord The coordinate
 * @param decimal_degrees The output coordinate in decimal degrees
 */
void nmea_get_coordinate_dd(nmea_coordinate_t coord, double *decimal_degrees);

/**
 * @brief Converts a coordinate to Degrees, Minutes and Seconds (DMS).
 * 
 * Sample output: 41°24'12.2"N 2°10'26.5"E
 * 
 * @param coord The coordinate
 * @param degrees The degrees output
 * @param minutes The minutes output
 * @param seconds The seconds output
 */
void nmea_get_coordinate_dms(nmea_coordinate_t coord, uint8_t *degrees, uint8_t *minutes, double *seconds);

/**
 * @brief Converts a coordinate to Degrees and Decimal Minutes (DMM).
 * 
 * Sample output: 41 24.2028, 2 10.4418
 * 
 * @param coord The coordinate
 * @param degrees The degrees output
 * @param decimal_minutes The decimal degrees output
 */
void nmea_get_coordinate_dmm(nmea_coordinate_t coord, uint8_t *degrees, double *decimal_minutes);

/**
 * @brief Converts a timestamp into milliseconds since the start of the day
 * 
 * @param time The time
 * @param milliseconds The milliseconds output
 */
void nmea_get_time_ms(nmea_time_t time, uint32_t *milliseconds);

#endif // NMEA_PARSER_UTILITIES

#ifdef __cplusplus
}
#endif

#endif // _JANMEAP_NMEA_H_