## Dependencies
```
# Install GLFW
curl https://codeload.github.com/glfw/glfw/zip/3.0.4 -o glfw-3.0.4.zip
unzip glfw-3.0.4.zip
cd glfw-3.0.4
cmake .
make
make install # MAY NEED SUDO

# Install PortAudio
brew install portaudio
```

- **GLFW:** http://www.glfw.org/
- **PortAudio:** http://www.portaudio.com/

## Building

```
make
```