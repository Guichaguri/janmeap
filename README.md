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

The functions available are documented in [nmea.h](./src/nmea.h).

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

    nmea_reader_process_char(&reader, '$');
    nmea_reader_process_char(&reader, 'G');
    nmea_reader_process_char(&reader, 'N');
    nmea_reader_process_char(&reader, 'G');
    nmea_reader_process_char(&reader, 'L');
    nmea_reader_process_char(&reader, 'L');
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

### Parallel streaming

The library allows you to buffer characters separated from the processing pipeline. This allows appending characters in interruptions (which must be as fast as possible), while processing the messages in the main loop.

Here's an example:
```c
nmea_reader_t reader;

void main() {
    // Creates the reader
    nmea_reader_init(&reader, process_nmea_msg);

    while (1) {
        // Processes the buffer in case there are full messages available, otherwise, does nothing
        // The process_nmea_msg function will be called here
        nmea_reader_process(&reader);
        sleep(0.1);
    }
}

// The interrupt function or event listener that is called when new characters are received
void itr_serial_rx_data(char ch) {
    // Appends a character to the buffer to be processed later in the main loop
    nmea_reader_add_char(&reader, ch);
}
```

### Parallel streaming with STM32 UART

This is a more practical example that uses the STM32 UART HAL library to read one character by one and feed it to the library

```c
nmea_reader_t reader;
char gps_buffer;

void main() {
    // Peripheral setup
    // ...

    // Creates the reader
    nmea_reader_init(&reader, process_nmea_msg);

    // Start reading the UART
    HAL_UART_Receive_IT(&huart1, (uint8_t*) &gps_buffer, 1);

    while(1) {
        // Processes the buffer if there are full messages available
        nmea_reader_process(&reader);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // Called when the UART completes reading
    // Appends a character to the buffer and request the next one
    nmea_reader_add_char(&reader, gps_buffer);
    HAL_UART_Receive_IT(&huart1, (uint8_t*) &gps_buffer, 1);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    // An error happened while receiving, we'll just restart everything
    nmea_reader_clear(&reader);
    HAL_UART_Receive_IT(&huart1, (uint8_t*) &gps_buffer, 1);
}
```