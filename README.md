# Just Another NMEA Parser

An NMEA 0183 streaming and parsing library written in plain C.

## Features
- Allows streaming characters one by one
- Can parse any NMEA 0183 message
- Validates checksums
- Allocation free
- Plain C99
- Designed to be used in microcontrollers
- Parses coordinates, timestamps, integers, floats and strings

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
        nmea_read_latitude(&message, &latitude);

        // North/South (N/S)
        char north_south;
        nmea_read_char(&message, &north_south);

        // Longitude (dddmm.mmmm)
        nmea_coordinate_t longitude;
        nmea_read_longitude(&message, &longitude);

        // East/West (E/W)
        char east_west;
        nmea_read_char(&message, &east_west);

        // Time (hhmmss.ss)
        nmea_time_t time;
        nmea_read_time(&message, &time);

        // Validity (A/V)
        char validity;
        nmea_read_char(&message, &validity);

        // ...
    }
}
```

A full and functional example can be seen in the `sample.c` file.

### Streaming with STM32 UART

This is a more practical example that uses the STM32 UART HAL library to read one character by one and feed it to the library

```c
nmea_reader_t reader;
char gps_buffer;
bool gps_done = 0;

void main() {
    // Peripheral setup
    // ...

    // Creates the reader
    nmea_reader_init(&reader, process_nmea_msg);

    // Start reading the UART
    HAL_UART_Receive_IT(&huart1, (uint8_t*) &gps_buffer, 1);

    while(1) {

        if (uart_done) {
            // Appends the read char to the reader
            nmea_reader_add_char(&reader, gps_buffer);

            // Receives a new char
            uart_done = 0;
            HAL_UART_Receive_IT(&huart1, (uint8_t*) &gps_buffer, 1);
        }

    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // Called when the UART completes reading
    // Sets the flag to process the character in the next main loop
    uart_done = 1;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    // An error happened while receiving, we'll just restart everything
    nmea_reader_clear(&reader);
    HAL_UART_Receive_IT(&huart1, (uint8_t*) &gps_buffer, 1);
}
```