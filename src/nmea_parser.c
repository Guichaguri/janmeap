
#include "nmea.h"

#if NMEA_PARSER

#include <string.h>
#include <stdlib.h>

static inline uint8_t nmea_is_not_delimiter(char c) {
	return c != ',' && c != '*' && c != '\0';
}

static char *nmea_find_delimiter(char *message) {
	while (nmea_is_not_delimiter(*message)) {
		message++;
	}

	return message;
}

inline void nmea_skip_field(char **message) {
	*message = nmea_find_delimiter(*message) + 1;
}

bool nmea_read_uint8(char **message, uint8_t *num) {
	if (nmea_is_not_delimiter(**message)) {
		*num = atoi(*message);
		nmea_skip_field(message);
		return true;
	}

	(*message)++;
	return false;
}

bool nmea_read_uint16(char **message, uint16_t *num) {
	if (nmea_is_not_delimiter(**message)) {
		*num = atoi(*message);
		nmea_skip_field(message);
		return true;
	}

	(*message)++;
	return false;
}

bool nmea_read_uint32(char **message, uint32_t *num) {
	if (nmea_is_not_delimiter(**message)) {
		*num = atol(*message);
		nmea_skip_field(message);
		return true;
	}

	(*message)++;
	return false;
}

bool nmea_read_float(char **message, float *num) {
	if (nmea_is_not_delimiter(**message)) {
		*num = atof(*message);
		nmea_skip_field(message);
		return true;
	}

	(*message)++;
	return false;
}

bool nmea_read_char(char **message, char *c) {
	if (nmea_is_not_delimiter(**message)) {
		*c = **message;
		nmea_skip_field(message);
		return true;
	}

	(*message)++;
	return false;
}

int nmea_read_string(char **message, char *str, int max_length) {
	char* end = nmea_find_delimiter(*message);
	int length = end - *message;

	if (max_length <= length) {
		length = max_length - 1;
	}

	for (int i = 0; i < length; i++) {
		str[i] = (*message)[i];
	}
	str[length] = '\0';

	*message = end + 1;

	return max_length;
}

bool nmea_read_coordinate(char **message, nmea_coordinate_t *coord) {
	char *end = nmea_find_delimiter(*message);
	int length = end - *message;
	bool readable = length >= 3;

	if (readable) {
		coord->degrees = ((*message)[0] - '0') * 10 + ((*message)[1] - '0');
		coord->decimal_minutes = strtod(*message + 2, end);
	}

	*message = end + 1;

	return readable;
}

#endif // NMEA_PARSER

#if NMEA_PARSER_UTILITIES

void nmea_get_coordinate_dd(nmea_coordinate_t coord, double *decimal_degrees) {
	*decimal_degrees = coord.degrees + coord.decimal_minutes / 60.0;
}

void nmea_get_coordinate_dms(nmea_coordinate_t coord, uint8_t *degrees, uint8_t *minutes, double *seconds) {
	uint8_t min = (uint8_t) coord.decimal_minutes;
	*degrees = coord.degrees;
	*minutes = min;
	*seconds = (coord.decimal_minutes - min) * 60.0;
}

void nmea_get_coordinate_dmm(nmea_coordinate_t coord, uint8_t *degrees, double *decimal_minutes) {
	*degrees = coord.degrees;
	*decimal_minutes = coord.decimal_minutes;
}

#endif // NMEA_PARSER_UTILITIES