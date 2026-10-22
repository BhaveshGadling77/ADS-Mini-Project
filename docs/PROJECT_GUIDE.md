# Bitshrink Project Guide

This guide describes the algorithms in this repository and distinguishes working implementations from planned interfaces. The integrated web application invokes only the text algorithms in `compression-core`. The image, audio, and video modules under `compression-core/modules/` are separate prototypes and are not currently called by the web API.

## System structure

```text
Browser (React/Vite) → Fastify API → C++ compression-engine CLI → job files
       Nginx (Compose) proxies API requests to Fastify and serves the UI
```

- `web/`: React + TypeScript UI, built to static assets.
- `server/`: Fastify upload, job, history, and download API. It starts the native CLI as a child process.
- `compression-core/`: C++17 engine, `.adsc` container handling, CLI, algorithms, custom data structures, and modality modules.
- `compression-core/modules/image/`: standalone BMP reader/writer and image Huffman implementation with a spatial predictor.
- `compression-core/modules/audio/`: standalone MP3-to-PCM, Huffman, PCM-to-MP3 demonstration; FFmpeg is required.
- `compression-core/modules/video/`: standalone video prototype sources.
- `experiments/expoPractice/`: independent React Native practice files.
- `docker/` and root `Dockerfile`: container definitions; `docker-compose.yml` runs the split web/API arrangement.
- `docs/`: algorithm, API, architecture, and team docs.

## Algorithms in the integrated engine

All three integrated algorithms are lossless and process input as bytes. They share the `.adsc` header, which records format identification, algorithm identifier, modality, and original byte length. The decoder uses this metadata to select the inverse algorithm and determine the output length.

### LZW (Lempel–Ziv–Welch)

LZW begins with dictionary entries for all 256 byte values. It scans the input while extending the current phrase as long as that phrase exists in the dictionary. When the next byte would make an unknown phrase, it emits the current phrase's integer code and adds the extended phrase. Codes are packed into a bitstream with a width that grows from 9 up to 16 bits as the dictionary grows; dictionary growth stops at its configured maximum.

The decoder rebuilds the same dictionary while reading codes. For each code it emits the corresponding phrase and adds the previous phrase plus the first byte of the current phrase. It also handles the LZW special case where the next code is the next dictionary entry being defined.

- **Data structures:** trie for encoder phrase extension; vector-backed dictionary for decoder; bit writer/reader.
- **Typical time:** linear in input bytes, assuming bounded phrase lookup; dictionary growth and bit packing are amortized.
- **Space:** up to the configured dictionary limit, plus input and output buffers.
- **Trade-off:** repeated phrases compress well; dictionary and header overhead can make small or high-entropy files larger.

### Shared Huffman codec for text and images

The reusable implementation in `compression-core/include/algorithms/huffman/HuffmanCodec.hpp` counts byte frequencies, inserts non-zero-frequency symbols into the project's custom min-heap, and repeatedly combines the two least frequent nodes. Both the text compressor and BMP module use this same tree builder and code-table generator. The resulting binary tree assigns shorter prefix-free codes to common bytes and longer codes to rare bytes. Each modality still owns its own file header and bitstream reader/writer: text writes the ADSC format, while the standalone image module writes HUF2. A single-symbol input receives a one-bit code so decoding remains well-defined.

- **Data structures:** 256-entry frequency table; custom min-heap for tree construction; binary tree; bit writer/reader.
- **Time:** frequency count and encode/decode are `O(n)`; tree construction is `O(k log k)`, where `k ≤ 256` is the number of distinct bytes.
- **Space:** `O(k)` tree/table plus bitstream and buffers.
- **Trade-off:** frequencies adapted to one file can be efficient, but the stored table is overhead and does not help data with near-uniform byte frequencies.

### BWT + MTF + zero-run encoding

The Burrows–Wheeler Transform (BWT) sorts the input's cyclic rotations, writes the final byte of each sorted rotation, and records the row corresponding to the original input. This transform is reversible and tends to group similar contexts; it does not itself reduce data size. Move-to-Front (MTF) maintains an ordered list of all byte values: each transformed byte is replaced with its current list index and moved to the front. Repeated nearby symbols often become zeroes. The final stage writes zero runs as a marker and run length, while non-zero values are copied directly. The archive stores the BWT primary index and encoded stream.

Decoding expands zero runs, reverses MTF, and uses LF mapping (cumulative symbol counts plus occurrence ranks) to reconstruct the original bytes from the BWT last column and primary index.

- **Data structures:** cyclic-rotation index array; 256-byte MTF list; symbol counts, ranks, and LF mapping arrays.
- **Current BWT cost:** this implementation compares rotations directly during sorting, so worst-case time is `O(n² log n)` and space is `O(n)`. LF reconstruction is `O(n + 256)`.
- **MTF cost:** list search and shifting are `O(256n)`, effectively linear for a fixed byte alphabet; space is `O(n + 256)`.
- **Trade-off:** useful on inputs with repeated local contexts; the BWT/MTF stages and primary-index/header overhead can expand some files. Rotation sorting is time-intensive for large inputs.

## Standalone algorithms

### BMP image prototype

`compression-core/modules/image/` reads 24-bit uncompressed BMP, normalizes scanlines and row padding to RGB pixels, then applies left-neighbor spatial prediction per color channel. The predicted byte stream is Huffman-coded using a frequency table and a custom `HUF2` header carrying dimensions and bitstream details. Decompression reverses Huffman coding and the predictor, then writes a BMP.

- **Spatial predictor:** stores channel differences from a neighboring pixel; modulo-byte arithmetic allows exact inversion. It is a decorrelation transform, not a lossy operation.
- **Huffman stage:** same min-frequency merge strategy described above, applied to residual bytes.
- **Data structures:** BMP pixel buffer, 256-symbol frequencies, priority queue/tree, code table, bit-level I/O.
- **Complexity:** predictor and coding passes are `O(n)`; tree construction is `O(k log k)` for at most 256 symbols. Memory is `O(n + k)`.
- **GUI support:** the API invokes this standalone CLI for BMP compression and decompression; the integrated CMake engine's image interface remains a stub.

### Audio prototype

`compression-core/modules/audio/` uses FFmpeg to decode MP3 into fixed-format 44.1 kHz stereo signed 16-bit PCM, then applies byte-wise static Huffman coding. The `HUF1` archive stores the PCM byte count, 256 64-bit frequencies, and encoded bits. Decompression restores PCM before asking FFmpeg to encode an MP3.

Huffman decoding can reproduce the intermediate PCM bytes exactly. MP3 re-encoding is lossy, so the resulting MP3 is not expected to match the input MP3 byte-for-byte. Since MP3 is already compressed and PCM byte frequencies may be near-uniform, the Huffman archive can be larger than the PCM input.

### GUI audio and video prototypes

The web API dispatches image, audio, and video jobs to the standalone module binaries under `compression-core/modules/`. The integrated CMake engine's image/audio/video classes remain stubs and are not used by these API routes. Video is an experimental lossy raw-YUV420 codec; it needs OpenCV 4 and even frame dimensions. Audio needs FFmpeg with MP3 support. See [TEAM_IMPLEMENTATION_GUIDE.md](development/TEAM_IMPLEMENTATION_GUIDE.md) for the integrated engine's extension points.

## Data structures highlighted by the project

| Structure | Role |
|---|---|
| Trie | Stores LZW phrases and supports prefix extension. |
| Min-heap | Selects the two least-frequent Huffman nodes efficiently. |
| Binary tree | Stores Huffman prefix codes and supports bitwise decoding. |
| Rotation/index array | Orders cyclic rotations for BWT and locates the original row. |
| MTF list | Maps symbols to recency ranks and updates recency after each symbol. |
| Bit stream reader/writer | Packs codes into bytes and reads variable-width bit fields. |

## Docker usage

Run commands from the repository root. Docker Engine and the Docker Compose plugin must be installed.

### Full application with Compose

```bash
docker compose up --build
```

Compose builds `docker/server/Dockerfile` (Node API plus C++ build) and `docker/web/Dockerfile` (Vite assets plus Nginx). Browse to <http://localhost>. Nginx serves the frontend on port 80 and proxies API traffic to the `server` service on port 3000. Uploaded job data persists in the `bitshrink-storage` Docker volume.

Useful operations:

```bash
docker compose up --build -d       # start in background
docker compose logs -f             # follow both services
docker compose logs -f server      # follow API logs only
docker compose down                # stop and remove containers/network
docker compose down -v             # also remove persisted job data
```

Compose maps port 80 for the UI and port 3000 for direct API access. If port 80 is occupied, change the host side of `"80:80"` in `docker-compose.yml` (for example, `"8080:80"`) and use <http://localhost:8080>.

### Unified container

The root `Dockerfile` builds the C++ binary, web assets, and backend into one Node runtime. Fastify serves both the API and SPA on port 3000.

```bash
docker build -t bitshrink:latest -f Dockerfile .
docker run --rm -p 3000:3000 bitshrink:latest
```

Open <http://localhost:3000>. To keep job files beyond the container lifetime, bind-mount a host directory to `/app/storage/jobs`, for example `-v "$PWD/storage/jobs:/app/storage/jobs"`.

### API-only and engine-only images

```bash
# API only (includes the C++ engine); uses the port declared in the image
docker build -t bitshrink-server -f docker/server/Dockerfile .
docker run --rm -p 3000:3000 bitshrink-server

# Standalone CLI; default command prints engine info
docker build -t bitshrink-engine -f docker/compression-core/Dockerfile .
docker run --rm bitshrink-engine info
```

For an input on the host, mount its directory and use the mount path inside the container:

```bash
docker run --rm -v "$PWD:/data" bitshrink-engine \
  compress -m text -a huffman -i /data/input.txt -o /data/output.adsc --json
docker run --rm -v "$PWD:/data" bitshrink-engine \
  decompress -i /data/output.adsc -o /data/restored.txt --json
```

The standalone engine image runs its C++ test runner while building. Docker Compose and the root application image include the BMP, audio, and video module binaries; the integrated engine's separate image/audio/video interfaces remain stubs.

## Local developer commands

```bash
./scripts/build.sh      # build integrated engine, modality modules, API, and UI
./scripts/test.sh       # run configured C++ and backend checks
./scripts/start.sh      # launch the API/SPA at http://localhost:3000
```

For standalone codec details, consult the READMEs under `compression-core/modules/`. The web API invokes these tools directly; they remain separate from the integrated CMake engine.
