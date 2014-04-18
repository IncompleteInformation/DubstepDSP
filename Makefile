default: bleep_test

bleep: main.c pitch.c
	@cc main.c midi.c pitch.c -o bleep \
		-lfftw3 \
		-lglfw3 \
		-lportaudio \
		-lportmidi \
		-framework Cocoa \
		-framework IOKit \
		-framework OpenGL \
		-framework CoreVideo

bleep_test: bleep
	@./bleep

midi: midi.c midi.h midi_test.c
	@cc midi_test.c midi.c -o midi_test \
		-lportmidi

midi_test: midi
	@./midi_test

pitch: pitch.c pitch.h pitch_test.c
	@cc pitch_test.c pitch.c -o pitch_test \
		-lfftw3 \
		-lsndfile

pitch_test: pitch
	@./pitch_test

serial: serial.c serial.h serial_test.c
	@cc serial_test.c serial.c -o serial_test \

serial_test: serial
	@./serial_test
