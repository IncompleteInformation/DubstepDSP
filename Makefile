FLAGS=-O3

default: bleep_test

bench: backend.* bench.* dywapitchtrack.* pitch.* windowing.*
	@cc ${FLAGS} backend.c bench.c dywapitchtrack.c pitch.c windowing.c -o bench \
		-lfftw3 \
		-lsndfile \
		-lglfw3 \
		-framework Cocoa \
		-framework IOKit \
		-framework OpenGL \
		-framework CoreVideo

bleep: backend.c dywapitchtrack.c gui.c main.c midi.c pitch.c serial.c windowing.c
	@cc ${FLAGS} backend.c dywapitchtrack.c gui.c main.c midi.c pitch.c serial.c windowing.c -o bleep \
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
	@cc ${FLAGS} midi_test.c midi.c -o midi_test \
		-lportmidi

midi_test: midi
	@./midi_test

pitch: pitch.c pitch.h pitch_test.c
	@cc ${FLAGS} pitch_test.c pitch.c -o pitch_test \
		-lfftw3 \
		-lsndfile

pitch_test: pitch
	@./pitch_test

serial: serial.c serial.h serial_test.c
	@cc ${FLAGS} serial_test.c serial.c -o serial_test \

serial_test: serial
	@./serial_test

simulator: simulator.c
	@cc ${FLAGS} simulator.c -o simulator \
		-lfftw3 \
		-lglfw3 \
		-lportaudio \
		-lsndfile \
		-framework Cocoa \
		-framework IOKit \
		-framework OpenGL \
		-framework CoreVideo
