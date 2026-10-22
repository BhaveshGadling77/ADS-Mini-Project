# Compression modules

All modality-specific compression code lives under this directory.

- `image/`: standalone lossless BMP prediction + Huffman CLI. Build with `make -C compression-core/modules/image`.
- `audio/`: standalone MP3/PCM Huffman prototype. Compile with the commands in its README and install FFmpeg to run it.
- `video/`: OpenCV prototype sources. See its README for optional build commands.

The image module uses the shared `adsc::HuffmanCodec` in `../include/algorithms/huffman/HuffmanCodec.hpp`, just like the integrated text Huffman compressor. Each format keeps its own file header and bitstream I/O.

These are separate prototypes. The application-facing `compression-core/src/algorithms/` classes for image, audio, and video are still stubs; only the text codecs are currently integrated with the API and default CMake targets.
