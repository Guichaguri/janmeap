# Just Another NMEA Parser

A NMEA streaming and parsing library in C99.

## Features
- Allows streaming characters one by one
- Can parse any NMEA message
- Validates checksums
- Allocation free
- Plain C99

## Usage

### Streaming

```c
#include "nmea.h"

void process_nmea_msg(char *message, int length) {
    printf("Received a message of length %i\n", length);
    // ...
}

void process_nmea_error(nmea_error_t error_type, char *message, int length) {
    printf("Received an error of type %i\n", error_type);
}

void main() {
    nmea_reader_t reader;
    nmea_reader_init(&reader, process_nmea_msg);
    nmea_reader_set_error_callback(&reader, process_nmea_error);

    nmea_reader_add_char(&reader, '$');
    nmea_reader_add_char(&reader, 'G');
    nmea_reader_add_char(&reader, 'N');
    nmea_reader_add_char(&reader, 'G');
    nmea_reader_add_char(&reader, 'L');
    nmea_reader_add_char(&reader, 'L');
    // ...
}
```

### Parsing

To parse an NMEA message, you have to read field by field. Check the [message documentation](https://gpsd.gitlab.io/gpsd/NMEA.html) for details of each field.

```c
void process_nmea_msg(char *message, int length) {
    char type[4];
    nmea_read_string(&message, type, 4);

    if (memcmp(type, "GLL", 3) == 0) {
        // GLL
        // $GNGLL,4404.14012,N,12118.85993,W,001037.00,A,A*67

        // Latitude (ddmm.mmmm)
        nmea_coordinate_t latitude;
        nmea_read_coordinate(&message, &latitude);

        // North/South (N/S)
        char north_south;
        nmea_read_char(&message, &north_south);

        // Longitude (ddmm.mmmm)
        nmea_coordinate_t longitude;
        nmea_read_coordinate(&message, &longitude);

        // East/West (E/W)
        char east_west;
        nmea_read_char(&message, &east_west);

        // Time (hhmmss.ss)
        float time;
        gps_read_float(&message, &time);

        // Validity (A/V)
        char validity;
        gps_read_char(&message, &validity);

        // ...
    }
}
```

A full and functional example can be seen in the `sample.c` file.