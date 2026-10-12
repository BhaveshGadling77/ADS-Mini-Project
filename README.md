# Bitshrink — Multi-Modal Data Compression System

A high-performance cross-platform data compression system built as an **Advanced Data Structures (ADS) Mini Project**.

Bitshrink implements fundamental lossless data compression algorithms and custom **advanced data structures** in native C++17/20, exposed through a Fastify REST API backend, and presented through an interactive React + TypeScript web application with multi-modality support.

---

## Team Task Assignment & Modules

| Modality | Algorithm / Strategy | Primary Data Structure | Team Member | Status | Implementation Guide |
|---|---|---|---|---|---|
| **Text** | LZW, Static Huffman, BWT + MTF | **Trie**, **Min-Heap**, **Suffix Array / List** | **Bhavesh** | **Completed & Tested** | Full C++ & Backend Implemented |
| **Image** | Spatial Quadtree, Color Quantization, 2D Wavelets | **Quadtree**, **KD-Tree**, Wavelet Matrix | **Jibran** | *Stubs & Architecture Ready* | [Teammate Guide](docs/development/TEAM_IMPLEMENTATION_GUIDE.md#1-image-compression-module-jibran) |
| **Audio** | 1D Wavelet Tree, Delta + RLE, LPC + Huffman | **Wavelet Tree**, Delta-RLE, Predictor Heap | **Parth** | *Stubs & Architecture Ready* | [Teammate Guide](docs/development/TEAM_IMPLEMENTATION_GUIDE.md#2-audio-compression-module-parth) |
| **Video** | Inter-frame Quadtree Delta, 3D Octree | **Quadtree**, **3D Octree** | **Sangram** | *Stubs & Architecture Ready* | [Teammate Guide](docs/development/TEAM_IMPLEMENTATION_GUIDE.md#3-video-compression-module-sangram) |

> 📖 **Teammates:** For detailed instructions on function signatures to implement (`compress` & `decompress`), file paths, data structures, and CLI test commands, see **[`docs/development/TEAM_IMPLEMENTATION_GUIDE.md`](docs/development/TEAM_IMPLEMENTATION_GUIDE.md)**.

---

## 1. System Architecture

```text
                                USER
                                 │
                                 ▼
                     ┌───────────────────────┐
                     │     Web Frontend      │
                     │                       │
                     │  React + TypeScript   │
                     │  Vite + Tailwind CSS  │
                     │                       │
                     │ • Modality Switcher   │
                     │   (Text/Image/Audio/  │
                     │    Video)             │
                     │ • Drag-and-drop file  │
                     │ • Compress / Restore  │
                     │ • Side-by-side Bench  │
                     │ • Metrics & Downloads │
                     └───────────┬───────────┘
                                 │
                                 │ HTTP / REST API (Multipart uploads up to 1 GB)
                                 ▼
                     ┌───────────────────────┐
                     │    Backend Server     │
                     │                       │
                     │  Node.js + TypeScript │
                     │  Fastify              │
                     │                       │
                     │ • Validate requests   │
                     │ • Disk stream storage │
                     │ • Invokes C++ engine  │
                     │ • Track job history   │
                     │ • Serves Web SPA      │
                     └───────────┬───────────┘
                                 │
                                 │ Process communication (CLI --json)
                                 ▼
                     ┌───────────────────────┐
                     │   C++ Compression     │
                     │        Engine         │
                     │                       │
                     │  Native C++17/20      │
                     │  • Text: LZW, Huffman,│
                     │    BWT+MTF (Bhavesh)  │
                     │  • Image stubs:       │
                     │    Quadtree (Jibran)  │
                     │  • Audio stubs:       │
                     │    Wavelet (Parth)    │
                     │  • Video stubs:       │
                     │    Interframe(Sangram)│
                     └───────────┬───────────┘
                                 │
                                 ▼
                     ┌───────────────────────┐
                     │     File Storage      │
                     │                       │
                     │ • Uploaded files      │
                     │ • Compressed (.adsc)  │
                     │ • Restored files      │
                     │ • Metadata (meta.json)│
                     └───────────────────────┘
```

---

## 2. Technology Stack

| Component | Technology | Purpose |
|---|---|---|
| **Web Frontend** | React 18 + TypeScript + Vite | Interactive multi-modal dashboard & benchmarking UI |
| **Styling** | Tailwind CSS + Lucide Icons | Modern dark-theme UI with responsive status badges |
| **Backend Server** | Node.js + TypeScript + Fastify | High-throughput streaming REST API & orchestration |
| **File Uploads** | `@fastify/multipart` (Streaming) | Direct stream-to-disk up to 1 GB without RAM exhaustion |
| **Static Delivery** | `@fastify/static` | Unified single-port deployment of frontend on port 3000 |
| **Compression Engine** | Native C++17/20 | Custom data structures, bit-level I/O, modular architecture |
| **Build System** | CMake 3.16+ | C++ library, test suite, and CLI executable |
| **Containerization** | Docker & Docker Compose | Multi-stage production container build & zero-config run |

---

## 3. Running with Docker (Recommended)

Bitshrink provides flexible Docker configurations:
- **Modular Multi-Container Architecture (Docker Compose)**: Separates the backend API server (`docker/server/Dockerfile`) and React frontend (`docker/web/Dockerfile`) reverse-proxied with Nginx.
- **Unified Single-Port Container (`Dockerfile`)**: Builds and serves both the Fastify backend and the compiled React frontend together on port **3000**.
- **Standalone C++ Core Engine (`docker/compression-core/Dockerfile`)**: Build and run the native CLI engine independently.

### Quick Start with Docker Compose (Frontend + Server)
```bash
# Build and run Bitshrink (Frontend on :80, API Server on :3000)
docker compose up --build
```
Open **[http://localhost](http://localhost)** in your browser.

### Building Services Individually
```bash
# 1. Build and run the Backend Server (Port 3000)
docker build -t bitshrink-server:latest -f docker/server/Dockerfile .
docker run -p 3000:3000 bitshrink-server:latest

# 2. Build and run the Frontend Web UI (Port 80)
docker build -t bitshrink-web:latest -f docker/web/Dockerfile .
docker run -p 80:80 -e BACKEND_HOST=host.docker.internal bitshrink-web:latest

# 3. Or build and run the Unified All-in-One Container (Port 3000)
docker build -t bitshrink:latest -f Dockerfile .
docker run -p 3000:3000 bitshrink:latest
```


---

## 4. Local Development Setup

### Prerequisites
- **GCC / G++** with C++20 support (GCC 11+)
- **CMake** (3.16+)
- **Node.js** (v20+) and **npm**

### Build All Modules
```bash
./scripts/build.sh
```

### Run Full Test Suite
```bash
./scripts/test.sh
```
Runs both:
1. **C++ Engine Unit Tests**: Trie, Min-Heap, LZW roundtrip, Huffman roundtrip, BWT roundtrip.
2. **Fastify Backend Integration Tests**: Health check, compression, decompression, roundtrip hash verification, algorithm comparison, job history, and file download.

### Start the Application Locally
```bash
./scripts/start.sh
```
Open **[http://localhost:3000](http://localhost:3000)** to view the web dashboard.

---

## 5. Standalone C++ Engine CLI

The C++ engine can also be executed directly from the command line:

```bash
# Text Compression (LZW)
./compression-core/build/bin/compression-engine compress -m text -a lzw -i input.txt -o compressed.adsc --json

# Text Decompression (auto-detects algorithm from header)
./compression-core/build/bin/compression-engine decompress -i compressed.adsc -o restored.txt --json

# Text Algorithm Comparison (Benchmarks LZW, Huffman, and BWT)
./compression-core/build/bin/compression-engine compare -i input.txt --json

# Image Compression Stub (Module for Jibran)
./compression-core/build/bin/compression-engine compress -m image -a image_quadtree -i photo.ppm -o photo.adsc --json

# Audio Compression Stub (Module for Parth)
./compression-core/build/bin/compression-engine compress -m audio -a audio_wavelet -i audio.raw -o audio.adsc --json

# Video Compression Stub (Module for Sangram)
./compression-core/build/bin/compression-engine compress -m video -a video_interframe -i video.yuv -o video.adsc --json
```

---

## 6. Project Structure

```text
ADS-Mini-Project/
│
├── Dockerfile                         # Unified multi-stage production container
├── docker-compose.yml                 # Multi-container orchestrator (server + frontend)
├── .dockerignore                      # Docker ignore rules (optimized build context)
│
├── docker/                            # Modular Docker container configurations
│   ├── server/                        # Fastify API Server + C++ Engine Dockerfile
│   ├── web/                           # React + Vite Web Frontend Nginx Dockerfile
│   ├── frontend/                      # Frontend alias Dockerfile
│   └── compression-core/              # Standalone C++ Engine CLI Dockerfile
│
├── web/                               # React + TypeScript Web Frontend
│   ├── src/
│   │   ├── components/                # FileDropzone, AlgorithmSelector, CompressionStats, Navbar
│   │   ├── pages/                     # Workspace, Compare, History, About
│   │   ├── services/                  # compressionApi.ts (REST client)
│   │   ├── types/                     # TypeScript interfaces
│   │   ├── App.tsx
│   │   └── main.tsx
│   ├── package.json
│   └── vite.config.ts
│
├── server/                            # Fastify + TypeScript Backend
│   ├── src/
│   │   ├── controllers/               # CompressionController.ts (1 GB streaming pipeline)
│   │   ├── routes/                    # apiRoutes.ts
│   │   ├── services/                  # storageService.ts, compressionEngineService.ts
│   │   ├── config/                    # Config and env vars
│   │   ├── types/                     # Request/response interfaces
│   │   └── index.ts                   # Fastify server & static web server
│   ├── storage/jobs/                  # Storage directory for jobs
│   ├── tests/                         # server.test.ts (integration tests)
│   └── package.json
│
├── compression-core/                  # Native C++17/20 Engine
│   ├── include/
│   │   ├── core/                      # Types.hpp, ICompressor.hpp, FileHeader.hpp, Engine.hpp
│   │   ├── datastructures/            # Trie.hpp, MinHeap.hpp, BinaryTree.hpp
│   │   ├── algorithms/
│   │   │   ├── text/                  # LZWCompressor.hpp, HuffmanCompressor.hpp, BWTCompressor.hpp
│   │   │   ├── image/                 # ImageCompressor.hpp (Jibran)
│   │   │   ├── audio/                 # AudioCompressor.hpp (Parth)
│   │   │   └── video/                 # VideoCompressor.hpp (Sangram)
│   │   └── io/                        # BitStream.hpp (BitWriter, BitReader)
│   ├── src/                           # Implementations of headers
│   ├── tests/                         # test_text_compression.cpp (unit & roundtrip tests)
│   ├── cli/                           # main.cpp (CLI executable with -m modality & --json)
│   └── CMakeLists.txt
│
├── shared/
│   └── types/                         # Shared type definitions (compression.ts)
│
├── docs/
│   ├── development/
│   │   └── TEAM_IMPLEMENTATION_GUIDE.md  # Detailed guide for Jibran, Parth, and Sangram
│   ├── architecture/                  # ARCHITECTURE.md
│   ├── algorithms/                    # TEXT_COMPRESSION.md
│   └── api/                           # API_SPEC.md
│
└── scripts/
    ├── build.sh                       # Builds C++ core, server, and web frontend
    ├── start.sh                       # Starts the fullstack server
    └── test.sh                        # Runs both C++ and server test suites
```
