FLAGS=-O3

default: bleep_test

bleep: gui.c live.c main.c midi.c pitch.c glove.c
	@cc ${FLAGS} gui.c live.c main.c midi.c pitch.c glove.c -o bleep \
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

glove: glove.c glove.h glove_test.c
	@cc ${FLAGS} glove_test.c glove.c -o glove_test \

glove_test: glove
	@./glove_test

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