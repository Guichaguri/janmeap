#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nmea.h"

nmea_coordinate_t gps_latitude;
nmea_coordinate_t gps_longitude;
char gps_north_south;
char gps_east_west;
uint8_t gps_quality_indicator = 0;
uint8_t gps_number_of_satellites = 0;
float gps_altitude_meters = 0; // Meters
float gps_speed_over_ground_knots = 0; // Knots
float gps_speed_over_ground_km_h = 0; // Km/h
float gps_track_angle_degrees = 0; // Degrees
nmea_time_t gps_time;
nmea_date_t gps_date;

static inline void process_nmea_gga(char *message) {
	// $GNGGA,001043.00,4404.14036,N,12118.85961,W,1,12,0.98,1113.0,M,-21.3,M,,*47

	// Time (hhmmss.ss)
	nmea_read_time(&message, &gps_time);

	// Latitude (ddmm.mmmm)
	nmea_read_latitude(&message, &gps_latitude);

	// North/South (N/S)
	nmea_read_char(&message, &gps_north_south);

	// Longitude (dddmm.mmmm)
	nmea_read_longitude(&message, &gps_longitude);

	// East/West (E/W)
	nmea_read_char(&message, &gps_east_west);

	// Quality indicator (enum)
	nmea_read_uint8(&message, &gps_quality_indicator);

	// Number of satellites (0-12)
	nmea_read_uint8(&message, &gps_number_of_satellites);

	// Horizontal Dilution of precision (meters)
	nmea_skip_field(&message); // Not interested in that, so I'll skip it

	// Antenna Altitude above/below mean-sea-level (meters)
	nmea_read_float(&message, &gps_altitude_meters);

	// I don't care about the rest, so I'll just not parse through it
}

static inline void process_nmea_rmc(char *message) {
	// $GNRMC,001031.00,A,4404.13993,N,12118.86023,W,0.146,,100117,,,A*7B

	// Time (hhmmss.ss)
	nmea_read_time(&message, &gps_time);

	// Validity (A/V)
	char validity;
	nmea_read_char(&message, &validity);
	if (validity != 'A') {
		// do something?
	}

	// Latitude (ddmm.mmmm)
	nmea_read_latitude(&message, &gps_latitude);

	// North/South (N/S)
	nmea_read_char(&message, &gps_north_south);

	// Longitude (dddmm.mmmm)
	nmea_read_longitude(&message, &gps_longitude);

	// East/West
	nmea_read_char(&message, &gps_east_west);

	// Speed over ground (knots)
	nmea_read_float(&message, &gps_speed_over_ground_knots);

	// Track Angle (degrees)
	nmea_read_float(&message, &gps_track_angle_degrees);

	// Date (ddmmyy)
	nmea_read_date(&message, &gps_date);

	// I don't care about the rest
}

static inline void process_nmea_vtg(char *message) {
	// $GPVTG,220.86,T,,M,2.550,N,4.724,K,A*34

	// Course over ground (degrees true)
	nmea_skip_field(&message); // I don't need that

	// T = True
	nmea_skip_field(&message); // I don't need that

	// Course over ground (degrees magnetic)
	nmea_skip_field(&message); // I don't need that

	// M = Magnetic
	nmea_skip_field(&message); // I don't need that

	// Speed over ground (knots)
	nmea_read_float(&message, &gps_speed_over_ground_knots);

	// N = Knots
	nmea_skip_field(&message); // I don't need that

	// Speed over ground (Km/h)
	nmea_read_float(&message, &gps_speed_over_ground_km_h);

	// I don't care about the rest
}

static inline void process_nmea_gll(char *message) {
	// $GNGLL,4404.14012,N,12118.85993,W,001037.00,A,A*67
	
	// Latitude (ddmm.mmmm)
	nmea_read_latitude(&message, &gps_latitude);

	// North/South (N/S)
	nmea_read_char(&message, &gps_north_south);

	// Longitude (dddmm.mmmm)
	nmea_read_longitude(&message, &gps_longitude);

	// East/West (E/W)
	nmea_read_char(&message, &gps_east_west);

	// Time (hhmmss.ss)
	nmea_read_time(&message, &gps_time);

	// Validity (A/V)
	char validity;
	nmea_read_char(&message, &validity);
	if (validity != 'A') {
		// do something?
	}

	// I don't care about the rest
}

static inline void process_nmea_message(char *message, int length) {
	printf("Raw Message: %s\n", message);

	char type[4];
	nmea_read_string(&message, type, 4);

	printf("Parsing %s message\n", type);

	if (memcmp(type, "RMC", 3) == 0) {
		process_nmea_rmc(message);
	} else if (memcmp(type, "GGA", 3) == 0) {
		process_nmea_gga(message);
	} else if (memcmp(type, "GLL", 3) == 0) {
		process_nmea_gll(message);
	} else if (memcmp(type, "VTG", 3) == 0) {
		process_nmea_vtg(message);
	} else {
		printf("Unknown message type: %s\n", type);
	}
}

void main() {
	nmea_reader_t reader;
	nmea_reader_init(&reader, process_nmea_message);
	
	char str[] = "$GNRMC,001031.00,A,4404.13993,N,12118.86023,W,0.146,,100117,,,A*7B\r\n"
		"$GNGGA,001043.00,4404.14036,N,12118.85961,W,1,12,0.98,1113.0,M,-21.3,M*47\r\n"
		"$GNGLL,4404.14012,N,12118.85993,W,001037.00,A,A*67\r\n"
		"$GPVTG,220.86,T,,M,2.550,N,4.724,K,A*34\r\n";

	for (int i = 0; i < strlen(str); i++) {
		nmea_reader_process_char(&reader, str[i]);
	}

	printf("Last Lat: %i %f %c\n", gps_latitude.degrees, gps_latitude.decimal_minutes, gps_north_south);
	printf("Last Lon: %i %f %c\n", gps_longitude.degrees, gps_longitude.decimal_minutes, gps_east_west);
	printf("Date: %i-%i-%i\n", gps_date.year, gps_date.month, gps_date.date);
	printf("Time: %i:%i:%.2f\n", gps_time.hours, gps_time.minutes, gps_time.seconds);
}