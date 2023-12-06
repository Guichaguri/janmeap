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
bool nmea_read_coordinate(char **message, nmea_coordinate_t *coord);
#endif // NMEA_PARSER

#if NMEA_PARSER_UTILITIES
void nmea_get_coordinate_dd(nmea_coordinate_t coord, double *decimal_degrees);
void nmea_get_coordinate_dms(nmea_coordinate_t coord, uint8_t *degrees, uint8_t *minutes, double *seconds);
void nmea_get_coordinate_dmm(nmea_coordinate_t coord, uint8_t *degrees, double *decimal_minutes);
#endif // NMEA_PARSER_UTILITIES

#ifdef __cplusplus
}
#endif

#endif // _NMEA_H_