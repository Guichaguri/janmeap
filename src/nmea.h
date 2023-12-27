#ifndef _NMEA_H_
#define _NMEA_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NMEA message buffer max length
 * Defaults to 164 characters, enough space to fit two messages (or partial messages)
 */
#ifndef NMEA_BUFFER_MAX_LENGTH
#define NMEA_BUFFER_MAX_LENGTH (82 * 2)
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
 * Represents a time.
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

typedef struct {
  char buffer[NMEA_BUFFER_MAX_LENGTH];
  uint16_t length;
  uint16_t buffer_start;
  nmea_process_message_t process_message;
  nmea_process_error_t process_error;
} nmea_reader_t;

void nmea_reader_init(nmea_reader_t *reader, nmea_process_message_t process_message);
void nmea_reader_set_error_callback(nmea_reader_t *reader, nmea_process_error_t process_error);

void nmea_reader_add_char(nmea_reader_t *reader, char c);
void nmea_reader_clear(nmea_reader_t *reader);

#if NMEA_PARSER
void nmea_skip_field(char **message);
bool nmea_read_uint8(char **message, uint8_t *num);
bool nmea_read_uint16(char **message, uint16_t *num);
bool nmea_read_uint32(char **message, uint32_t *num);
bool nmea_read_float(char **message, float *num);
bool nmea_read_char(char **message, char *c);
int nmea_read_string(char **message, char *str, int max_length);

/**
 * @brief Reads a latitude/longitude coordinate in "ddmm.mm" or "dddmm.mm"
 * 
 * @param message The message pointer
 * @param coord The output coordinate
 * @param deg_3_digits true if the format is "dddmm.mm", false if the format is "ddmm.mm"
 * @return true when parsing was sucessful
 */
bool nmea_read_coordinate(char **message, nmea_coordinate_t *coord, bool deg_3_digits);

/**
 * @brief Read a latitude coordinate in "ddmm.mm".
 * 
 * Shorthand for `nmea_read_coordinate(message, coord, false)`.
 * 
 * @param message The message pointer
 * @param coord The output coordinate
 * @return true when parsing was succesful
 */
bool nmea_read_latitude(char **message, nmea_coordinate_t *coord);

/**
 * @brief Read a longitude coordinate in "dddmm.mm".
 * 
 * Shorthand for `nmea_read_coordinate(message, coord, true)`.
 * 
 * @param message The message pointer
 * @param coord The output coordinate
 * @return true when parsing was succesful
 */
bool nmea_read_longitude(char **message, nmea_coordinate_t *coord);

/**
 * @brief Reads a date in "ddmmyy"
 * 
 * @param message The message pointer
 * @param date The output date
 * @return true when parsing was sucessful
 */
bool nmea_read_date(char **message, nmea_date_t *date);

/**
 * @brief Reads a time in "hhmmss.ss"
 * 
 * @param message The message pointer
 * @param time The output time
 * @return true when parsing was sucessful
 */
bool nmea_read_time(char **message, nmea_time_t *time);

#endif // NMEA_PARSER

#if NMEA_PARSER_UTILITIES
void nmea_get_coordinate_dd(nmea_coordinate_t coord, double *decimal_degrees);
void nmea_get_coordinate_dms(nmea_coordinate_t coord, uint8_t *degrees, uint8_t *minutes, double *seconds);
void nmea_get_coordinate_dmm(nmea_coordinate_t coord, uint8_t *degrees, double *decimal_minutes);
void nmea_get_time_ms(nmea_time_t time, uint32_t *milliseconds);
#endif // NMEA_PARSER_UTILITIES

#ifdef __cplusplus
}
#endif

#endif // _NMEA_H_