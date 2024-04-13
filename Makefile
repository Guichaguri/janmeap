build_sample:
	gcc -o sample.out ./src/sample.c ./src/nmea_parser.c ./src/nmea_stream.c

run_sample: build_sample
	./sample.out