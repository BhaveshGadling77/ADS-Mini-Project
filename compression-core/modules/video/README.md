# Video prototype files

`standalone/` contains experimental video compression/decompression source files. The web API invokes these binaries for raw YUV420 `.yuv` uploads; the CMake engine has separate video stubs.

The codec is lossy and requires even frame dimensions. The Docker application image builds and includes its binaries. See [the project guide](../../../docs/PROJECT_GUIDE.md#gui-file-formats) for GUI input details and the [team implementation guide](../../../docs/development/TEAM_IMPLEMENTATION_GUIDE.md) for integrated engine extension points.

## Build the prototype programs

These sources use OpenCV 4. Install its development package and `pkg-config`, then build from the repository root:

```bash
g++ -std=c++17 compression-core/modules/video/standalone/videoCompress.cpp \
  $(pkg-config --cflags --libs opencv4) -o compression-core/modules/video/videoCompress
g++ -std=c++17 compression-core/modules/video/standalone/videoDecompress.cpp \
  $(pkg-config --cflags --libs opencv4) -o compression-core/modules/video/videoDecompress
```

The generated binaries are ignored by Git. `scripts/build.sh` builds the video prototype when OpenCV 4 development files and `pkg-config` are installed; otherwise it stops with an installation hint. Install `libopencv-dev` and `pkg-config`, then rerun the build script.
