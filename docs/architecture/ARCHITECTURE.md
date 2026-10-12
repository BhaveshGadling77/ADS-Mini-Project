# CODEC — System Architecture Documentation

## 1. Overview

**CODEC** is an Advanced Data Structures (ADS) Mini Project implementing data compression algorithms and custom data structures exposed through a modern web interface.

The application follows a modular, decoupled 3-tier architecture:

```text
                 USER
                  │
                  ▼
       ┌──────────────────────┐
       │     Web Frontend     │
       │                      │
       │ React + TypeScript   │
       │ Vite + Tailwind CSS  │
       │                      │
       │ • Upload text files  │
       │ • Select algorithm   │
       │ • Compress files     │
       │ • Decompress files   │
       │ • Benchmark / Compare│
       │ • Display metrics    │
       │ • Download outputs   │
       └──────────┬───────────┘
                  │
                  │ HTTP / REST API (Multipart file upload)
                  ▼
       ┌──────────────────────┐
       │    Backend Server    │
       │                      │
       │ Node.js + TypeScript │
       │ Fastify              │
       │                      │
       │ • Validate requests  │
       │ • Manage file storage│
       │ • Invoke C++ engine  │
       │ • Track job history  │
       │ • Return results     │
       └──────────┬───────────┘
                  │
                  │ Process Communication (CLI --json)
                  ▼
       ┌──────────────────────┐
       │   C++ Compression    │
       │        Engine        │
       │                      │
       │ Native C++17/20      │
       │ • LZW (Trie)         │
       │ • Huffman (Min-Heap) │
       │ • BWT + MTF          │
       │ • Compression & Dec. │
       └──────────┬───────────┘
                  │
                  ▼
       ┌──────────────────────┐
       │    File Storage      │
       │                      │
       │ • Uploaded files     │
       │ • Compressed (.adsc) │
       │ • Restored files     │
       │ • Metadata (meta.json│
       └──────────────────────┘
```

---

## 2. Component Details

### 2.1 Web Frontend (`web/`)
- **Technology:** React 18, TypeScript, Vite, Tailwind CSS, Lucide Icons.
- **Pages:**
  - `Workspace`: Drag-and-drop file upload, algorithm selector (LZW, Huffman, BWT), live compression statistics, decompression mode.
  - `Compare`: Multi-algorithm benchmark running all algorithms simultaneously on the same input file, with winners identified and interactive cards.
  - `History`: Historical job list showing timestamps, sizes, ratios, and direct download links.
  - `About`: Detailed breakdown of data structures, complexity analyses, and project team roles.
- **Production Delivery:** The built assets (`web/dist/`) are served statically by Fastify with SPA fallback.

### 2.2 Backend Server (`server/`)
- **Technology:** Node.js 22+, TypeScript, Fastify 5, `@fastify/multipart`, `@fastify/cors`, `@fastify/static`.
- **Responsibilities:**
  - Streaming multipart file uploads.
  - Isolated job directory allocation (`server/storage/jobs/<jobId>`).
  - Executing compiled C++ executable (`compression-engine`) via `child_process.execFile`.
  - Machine-readable JSON output parsing.
  - File download streaming with appropriate headers.

### 2.3 C++ Compression Engine (`compression-core/`)
- **Technology:** C++17/C++20, CMake 3.16+.
- **Data Structures Implemented from Scratch:**
  - **Trie (Prefix Tree)** for LZW dictionary matching.
  - **Min-Heap (Priority Queue)** for Huffman frequency tree construction.
  - **Binary Tree** for Huffman symbol decoding.
  - **Suffix Array / Cyclic Rotation Array** for Burrows-Wheeler Transform.
  - **Move-to-Front (MTF) Dynamic List** for symbol rank transformation.
  - **BitStream (`BitWriter`, `BitReader`)** for bit-level I/O.
- **Binaries:**
  - `bin/compression-engine`: CLI entrypoint supporting `compress`, `decompress`, `compare`, `info` with `--json` option.
  - `bin/test_runner`: Automated unit and roundtrip test suite.

---

## 3. Binary File Format Specification (`.adsc`)

All compressed files produced by the engine share a unified 16-byte binary header:

| Offset | Field | Type | Description |
|---|---|---|---|
| `0x00 - 0x03` | Magic | `char[4]` | `'A', 'D', 'S', 'C'` (0x41, 0x44, 0x53, 0x43) |
| `0x04` | Version | `uint8_t` | Version `0x01` |
| `0x05` | Algorithm | `uint8_t` | `1` = LZW, `2` = Static Huffman, `3` = BWT + MTF |
| `0x06` | Modality | `uint8_t` | `1` = Text |
| `0x07` | Flags | `uint8_t` | Reserved (0) |
| `0x08 - 0x0F` | Original Size | `uint64_t` | Uncompressed file size in bytes (little-endian) |

Following the header is the algorithm-specific payload:
- **LZW Payload:** Variable-width bitstream (9 to 16 bits per code).
- **Huffman Payload:** Symbol frequency table (`uint16_t` count + `(symbol, freq)` pairs) followed by variable-length Huffman bitstream.
- **BWT Payload:** Primary index (`uint32_t`, 4 bytes) followed by Zero-RLE MTF stream.
