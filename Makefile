default: audio test-audio

audio:
	@clang audio.c -lportaudio -o audio

test-audio:
	@./audio