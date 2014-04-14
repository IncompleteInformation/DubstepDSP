default: test-combined

pitch: pitch.c pitch.h pitch_test.c
	@cc pitch_test.c pitch.c -o pitch_test \
		-lfftw3 \
		-lsndfile

pitch_test: pitch
	@./pitch_test

combined: combined.c pa_ringbuffer.c pitch.c
	@cc combined.c pa_ringbuffer.c pitch.c -o combined \
		-lfftw3 \
		-lglfw3 \
		-lportaudio \
		-lportmidi \
		-framework Cocoa \
		-framework IOKit \
		-framework OpenGL \
		-framework CoreVideo

test-combined: combined
	@./combined