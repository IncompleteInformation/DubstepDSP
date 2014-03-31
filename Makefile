default: audio video test-audio test-video

audio: audio.c
	@clang audio.c -o audio \
		-lportaudio

video: video.c
	@clang video.c -o video \
		-lglfw3 -framework Cocoa \
		-framework IOKit \
		-framework OpenGL \
		-framework CoreVideo

test-audio: audio
	@./audio

test-video: video
	@./video