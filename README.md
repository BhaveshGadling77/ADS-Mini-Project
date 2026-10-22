# Bitshrink — ADS Data Compression Project

Bitshrink is an Advanced Data Structures mini project with a React web interface, a Fastify API, and native C++ compression modules. The GUI supports text, BMP image, MP3 audio, and raw YUV420 video compression and decompression. Text uses LZW, Huffman coding, or BWT + MTF; image and audio use Huffman coding; video uses an experimental lossy inter-frame quadtree codec.

## Project layout

```text
.
├── compression-core/       C++ engine, CLI, data structures, tests
│   └── modules/
│       ├── image/          BMP prediction + Huffman CLI
│       ├── audio/          PCM Huffman prototype
│       └── video/          OpenCV prototype sources
├── server/                 Fastify API and job storage
├── web/                    React + TypeScript UI
├── shared/                 Shared application type definitions
├── experiments/            Independent UI experiments
├── docker/                 Service Dockerfiles and Nginx templates
├── docs/                   Algorithm, API, architecture, and team docs
└── scripts/                Local build, test, and start helpers
```

See [docs/PROJECT_GUIDE.md](docs/PROJECT_GUIDE.md) for the algorithms, data structures, project boundaries, and detailed Docker commands.

## GUI file formats

- **Text:** regular text files; output `.adsc`.
- **Image:** 24-bit uncompressed BMP only; output `.huf2`; decompression restores BMP exactly.
- **Audio:** MP3 input; output `.huf`. Huffman coding preserves decoded PCM, while the MP3 output is re-encoded and can differ from the original. FFmpeg with MP3 support is required.
- **Video:** raw YUV420 `.yuv` input; enter the original even width and height; output `.cmp`. The prototype is lossy and decompression restores raw `.yuv`.

Choose the matching modality before decompressing so the GUI accepts the corresponding `.adsc`, `.huf2`, `.huf`, or `.cmp` archive.

## Run with Docker Compose

This starts the complete project: the API/C++ server and the web frontend. The script builds both images, runs the containers in the foreground, and removes the containers and network when you press `Ctrl+C` or the Compose process exits. Uploaded job data remains in the named volume.

```bash
./scripts/docker-run.sh
```

Open <http://localhost> (frontend on port 80; API on port 3000). The build is limited to one concurrent Compose build and two C++ compile jobs to reduce system load.

### Docker Hub DNS or network errors

If the build fails with `lookup registry-1.docker.io ... server misbehaving`, Docker cannot resolve or reach Docker Hub to download the base images. This happens before the project build begins. Check DNS and retry the base-image pulls:

```bash
getent hosts registry-1.docker.io
docker pull debian:bookworm-slim
docker pull node:22-bookworm-slim
```

If the hostname lookup fails, restore the machine's network/DNS connection first. On systemd-based Linux, you can inspect DNS with `resolvectl status` and restart the resolver with `sudo systemctl restart systemd-resolved`. Once both image pulls succeed, rerun `./scripts/docker-run.sh`. On managed or campus networks, use the DNS server or proxy configured for that network.

If host port 3000 is already allocated, check whether the Compose server is using it:

```bash
docker compose ps
```

To run the whole application as one image instead, stop Compose with `Ctrl+C` and let the script remove its containers, then use:

```bash
docker build --tag bitshrink:latest --file Dockerfile .
docker run --rm --detach --name bitshrink --publish 3000:3000 bitshrink:latest
```

The `--rm` option removes this container automatically when it stops. Open <http://localhost:3000>. If port 3000 is already in use, map host port 3001 instead:

```bash
docker run --rm --detach --name bitshrink --publish 3001:3000 bitshrink:latest
```

Then open <http://localhost:3001>. To stop the detached container, run `docker stop bitshrink`; Docker removes it automatically.

The backend-only service image can also be run with `docker compose up --build server`; it exposes the API on port 3000. The standalone CLI image accepts the engine's regular command-line arguments; for example:

```bash
docker build -t bitshrink-engine -f docker/compression-core/Dockerfile .
docker run --rm bitshrink-engine info
```

To pass a local input file to that CLI, mount its containing directory and refer to the in-container path, e.g. `docker run --rm -v "$PWD:/data" bitshrink-engine compress -m text -a lzw -i /data/input.txt -o /data/output.adsc --json`.

## Run locally without Docker

Install C++17, CMake 3.16+, GNU Make, Node.js 20+, npm, FFmpeg with MP3 support, and OpenCV 4 development files (including `pkg-config`) for video. Run these commands from the repository root. Build the native codecs, server, and frontend first:

```bash
./scripts/build.sh
```

After the build completes, start the backend in Terminal 1:

```bash
cd server
npm start
```

From the repository root in a second terminal, start the frontend dev server:

```bash
cd web
npm run dev
```

Open <http://localhost:5173>. The frontend proxies API requests to the backend at <http://localhost:3000>. Stop each process with `Ctrl+C` in its terminal.

The build helper performs the dependency installation and builds the integrated C++ engine, image/audio/video modules, server, and frontend. To rebuild after changing code, rerun `./scripts/build.sh`.

For direct engine use:

```bash
./compression-core/build/bin/compression-engine info
./compression-core/build/bin/compression-engine compress -m text -a lzw -i input.txt -o output.adsc --json
./compression-core/build/bin/compression-engine decompress -i output.adsc -o restored.txt --json
```

## Standalone modality projects

- BMP prototype: see [compression-core/modules/image/README.md](compression-core/modules/image/README.md); build with `make -C compression-core/modules/image`.
- Audio prototype: see [compression-core/modules/audio/README.md](compression-core/modules/audio/README.md); requires FFmpeg at runtime.
- Video prototype: [compression-core/modules/video/README.md](compression-core/modules/video/README.md); the GUI accepts raw YUV420 data and requires frame dimensions.
- Integrated engine scope and planned modules: see [docs/development/TEAM_IMPLEMENTATION_GUIDE.md](docs/development/TEAM_IMPLEMENTATION_GUIDE.md).
