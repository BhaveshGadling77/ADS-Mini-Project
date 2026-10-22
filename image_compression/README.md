# Image Compression Engine — ADS Mini Project

> **Module Owner:** Jibran Jamdar  
> **Topic:** Lossless Image Compression & Decompression (C++17)  
> **Repository Branch:** `Jibran/image_compression`

---

## 1. Overview

This module provides the core **Lossless Image Compression Engine** for the ADS Mini Project. It processes standard 24-bit uncompressed BMP images, computes symbol frequencies, builds a Huffman coding tree, and serializes the compressed data with a custom binary header. It also implements the inverse decompression pipeline to losslessly reconstruct the original image down to the exact pixel.

---

## 2. Implemented Components & Architecture

```text
[Input BMP Image]
       │
       ▼
[BMP Reader] ──────────► Parses file header, validates 24-bit RGB, reads pixel data
       │
       ▼
[Data Preprocessor] ───► Flattens pixels into raw byte streams (RGB components)
       │
       ▼
[Frequency Counter] ───► Analyzes byte distribution (0–255)
       │
       ▼
[Huffman Tree Engine] ─► Builds min-heap priority queue, derives prefix-free codes
       │
       ▼
[BitWriter & Header] ──► Serializes archive header + frequency table + packed bitstream
       │
       ▼
[Compressed .huf File]
```

### Key Modules

- **`image/bmp_reader.h` / `bmp_reader.cpp`**: Reads 24-bit uncompressed Windows BMP files, validates headers, handles 4-byte row padding, and inverts bottom-up scanlines into top-down RGB pixel arrays.
- **`image/bmp_writer.h` / `bmp_writer.cpp`**: Serializes RGB pixels back into valid 24-bit BMP images with proper row padding and standard headers.
- **`compression/frequency.h` / `frequency.cpp`**: Analyzes the raw pixel byte buffer and computes a 256-symbol frequency distribution.
- **`compression/huffman.h` / `huffman.cpp`**: Implements the Huffman tree algorithm using standard priority queues (`std::priority_queue`) to generate prefix-free variable-length binary codes.
- **`compression/bit_writer.h` / `bit_writer.cpp`**: Buffers arbitrary bit sequences into byte boundaries and writes packed binary data to disk.
- **`compression/bit_reader.h` / `bit_reader.cpp`**: Reads packed binary bitstreams bit-by-bit to traverse the Huffman tree during decoding.
- **`format/file_header.h` / `file_header.cpp`**: Defines the custom compressed container (`ImageHeader`) storing magic signature, dimensions, and serialized frequency tables needed for decompression.
- **`compression/encoder.h` / `encoder.cpp`**: Orchestrates the full compression pipeline.
- **`compression/decoder.h` / `decoder.cpp`**: Reads compressed archives, reconstructs the Huffman tree from stored frequencies, decodes the bitstream, and emits decompressed BMP files.
- **`compare_images.cpp`**: Standalone C++ verification tool for bit-exact comparison between original and decompressed BMP images.
- **`scripts/test_images.sh`**: Automated end-to-end benchmark suite testing compression, decompression, compression ratio, and exact pixel match across all test images.

---

## 3. Quick Start Guide for Teammates

### Prerequisites

- GCC / G++ supporting C++17
- `make`
- *(Optional for visual checks)*: ImageMagick (`compare`, `file`)

### Build Instructions

From within the `image_compression/` directory, simply run:

```bash
make
```

To clean all build artifacts and binaries:

```bash
make clean
```

---

## 4. Running the Compressor

### Interactive CLI

Launch the interactive tool:

```bash
./compressor
```

You will see the main menu:

```text
========== IMAGE COMPRESSION TOOL ==========
1. Compress Image
2. Decompress Image
0. Exit
============================================
Enter choice:
```

- **To Compress**: Enter `1`, specify the input BMP path (e.g., `input/images/small_photo_640x426.bmp`), and provide an output path (e.g., `output/compressed/small.huf`).
- **To Decompress**: Enter `2`, specify the compressed file path (`output/compressed/small.huf`), and provide an output BMP path (e.g., `output/decompressed/small_restored.bmp`).

---

## 5. Automated Verification & Benchmark Suite

An automated test script is provided to test all sample images:

```bash
./scripts/test_images.sh
```

The script will:
1. Discover all BMP files in `input/images/`.
2. Compress each image using `./compressor`.
3. Decompress each image.
4. Verify bit-exact pixel matching between original and reconstructed images.
5. Generate a comprehensive benchmark report at `results/compression_report.csv` and detailed logs in `results/logs/`.

### Verified Benchmark Results

| Image | Original (B) | Compressed (B) | Saved (%) | Ratio | Status |
| :--- | :---: | :---: | :---: | :---: | :---: |
| `graphic_solid_regions.bmp` | 3,686,538 | 1,031,674 | **72.02%** | **3.573** | **PASS** |
| `large_photo_1920x1280.bmp` | 7,372,938 | 6,536,986 | **11.34%** | **1.128** | **PASS** |
| `medium_photo_1280x853.bmp` | 3,275,658 | 3,049,780 | **6.90%** | **1.074** | **PASS** |
| `stress_test_4k_3840x2160.bmp` | 24,883,338 | 23,169,386 | **6.89%** | **1.074** | **PASS** |
| `small_photo_640x426.bmp` | 818,058 | 767,295 | **6.21%** | **1.066** | **PASS** |
| `sample_1920×1280.bmp` | 7,372,938 | 7,263,516 | **1.48%** | **1.015** | **PASS** |
| `test.bmp` | 818,058 | 808,001 | **1.23%** | **1.012** | **PASS** |
| `colorful_textures_1920x1280.bmp` | 7,372,938 | 7,315,811 | **0.77%** | **1.008** | **PASS** |

*All 8 images pass complete round-trip lossless verification.*

---

## 6. Backend / Server Integration Guide (For Bhavesh & Node.js Server)

To integrate this engine with the Node.js / TypeScript REST server (`server/`):

1. **Compilation**: The server build step can compile the binary using `make -C image_compression`.
2. **Process Execution**: The server can spawn `./image_compression/compressor` using `child_process.spawn` or `execFile`.
3. **Menu Automation**: Input parameters can be piped via stdin:
   ```bash
   # Compression
   printf "1\n<input_bmp_path>\n<output_compressed_path>\n0\n" | ./image_compression/compressor

   # Decompression
   printf "2\n<compressed_path>\n<output_decompressed_path>\n0\n" | ./image_compression/compressor
   ```
4. **Validation**: Check for exit code `0` and grep for `Operation completed successfully.` in the process output.
