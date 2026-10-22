
# Bitshrink - Backend Server & C++ Compression Core Engine
# Multi-stage Dockerfile for the Fastify API Backend


# 
# Stage 1: Build C++ Core Compression Engine
# 
FROM debian:bookworm-slim AS cpp-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    make \
    pkg-config \
    libopencv-dev \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY compression-core/CMakeLists.txt ./compression-core/CMakeLists.txt
COPY compression-core/include/ ./compression-core/include/
COPY compression-core/src/ ./compression-core/src/
COPY compression-core/cli/ ./compression-core/cli/
COPY compression-core/tests/ ./compression-core/tests/
COPY compression-core/modules/ ./compression-core/modules/

WORKDIR /workspace/compression-core/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . --parallel 2
RUN make -C /workspace/compression-core/modules/image
RUN g++ -std=c++17 /workspace/compression-core/modules/audio/audioCompression.cpp -o /workspace/compression-core/modules/audio/audioCompression
RUN g++ -std=c++17 /workspace/compression-core/modules/audio/audioDecompression.cpp -o /workspace/compression-core/modules/audio/audioDecompression
RUN pkg-config --exists opencv4 && \
    g++ -std=c++17 /workspace/compression-core/modules/video/standalone/videoCompress.cpp $(pkg-config --cflags opencv4) -lopencv_imgproc -lopencv_core -o /workspace/compression-core/modules/video/videoCompress
RUN pkg-config --exists opencv4 && \
    g++ -std=c++17 /workspace/compression-core/modules/video/standalone/videoDecompress.cpp $(pkg-config --cflags opencv4) -lopencv_imgproc -lopencv_core -o /workspace/compression-core/modules/video/videoDecompress

# 
# Stage 2: Build Server Backend (TypeScript -> JavaScript)
# 
FROM node:22-bookworm-slim AS server-builder

WORKDIR /workspace/shared
COPY shared/ ./

WORKDIR /workspace/server
COPY server/package*.json ./
RUN npm ci

COPY server/ ./
RUN npm run build

# 
# Stage 3: Minimal Production Runtime
# 
FROM node:22-bookworm-slim AS runner

RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    libopencv-core406 \
    libopencv-imgproc406 \
    ffmpeg \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

ENV NODE_ENV=production \
    PORT=3000 \
    HOST=0.0.0.0 \
    ENGINE_BINARY_PATH=/app/bin/compression-engine \
    IMAGE_COMPRESSOR_PATH=/app/bin/modules/image/compressor \
    AUDIO_COMPRESSOR_PATH=/app/bin/modules/audio/audioCompression \
    AUDIO_DECOMPRESSOR_PATH=/app/bin/modules/audio/audioDecompression \
    VIDEO_COMPRESSOR_PATH=/app/bin/modules/video/videoCompress \
    VIDEO_DECOMPRESSOR_PATH=/app/bin/modules/video/videoDecompress \
    STORAGE_DIR=/app/storage/jobs

WORKDIR /app

# Copy C++ engine binary
COPY --from=cpp-builder /workspace/compression-core/build/bin/compression-engine /app/bin/compression-engine
COPY --from=cpp-builder /workspace/compression-core/modules/image/compressor /app/bin/modules/image/compressor
COPY --from=cpp-builder /workspace/compression-core/modules/audio/audioCompression /app/bin/modules/audio/audioCompression
COPY --from=cpp-builder /workspace/compression-core/modules/audio/audioDecompression /app/bin/modules/audio/audioDecompression
COPY --from=cpp-builder /workspace/compression-core/modules/video/videoCompress /app/bin/modules/video/videoCompress
COPY --from=cpp-builder /workspace/compression-core/modules/video/videoDecompress /app/bin/modules/video/videoDecompress
RUN chmod +x /app/bin/compression-engine /app/bin/modules/image/compressor /app/bin/modules/audio/audioCompression /app/bin/modules/audio/audioDecompression /app/bin/modules/video/videoCompress /app/bin/modules/video/videoDecompress

# Copy shared types
COPY --from=server-builder /workspace/shared /app/shared

# Copy built server & install production dependencies
WORKDIR /app/server
COPY --from=server-builder /workspace/server/package*.json ./
RUN npm ci --omit=dev

COPY --from=server-builder /workspace/server/dist ./dist

# Create storage directory for uploaded, compressed, and restored files
RUN mkdir -p /app/storage/jobs

EXPOSE 3000

HEALTHCHECK --interval=15s --timeout=5s --start-period=5s --retries=3 \
  CMD node -e "fetch('http://localhost:3000/health').then(r => r.ok ? process.exit(0) : process.exit(1)).catch(() => process.exit(1))"

CMD ["node", "dist/index.js"]
