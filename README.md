## Dependencies
Install Homebrew from http://brew.sh/ if you haven't already. Then:
```
# Install CMake (to build GLFW)
brew install cmake

# Install GLFW
curl https://codeload.github.com/glfw/glfw/zip/3.0.4 -o glfw-3.0.4.zip
unzip glfw-3.0.4.zip
cd glfw-3.0.4
cmake .
make
make install # MAY NEED SUDO

# Install PortAudio
brew install portaudio

# Install fftw3
brew install fftw
```

- **GLFW:** http://www.glfw.org/
- **PortAudio:** http://www.portaudio.com/

## Building

```
make
```
