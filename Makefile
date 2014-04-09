default: test-combined

pitch: pitch.c pitch.h pitch_test.c
	@cc pitch_test.c pitch.c -o pitch_test \
		-lfftw3 \
		-lsndfile

pitch_test: pitch
	@./pitch_test

audio: audio.c
	@cc audio.c -o audio \
		-lportaudio
		-lfftw3 \

video: video.c
	@cc video.c -o video \
		-lglfw3 \
		-framework Cocoa \
		-framework IOKit \
		-framework OpenGL \
		-framework CoreVideo

combined: combined.c pa_ringbuffer.c pitch.c
	@cc combined.c pa_ringbuffer.c pitch.c -o combined \
		-lportaudio \
		-lglfw3 \
		-lfftw3 \
		-framework Cocoa \
		-framework IOKit \
		-framework OpenGL \
		-framework CoreVideo

test-audio: audio
	@./audio

test-video: video
	@./video

test-combined: combined
	@./combined