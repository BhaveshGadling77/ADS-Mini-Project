
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
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY compression-core/ ./compression-core/

WORKDIR /workspace/compression-core/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --parallel $(nproc)

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
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

ENV NODE_ENV=production \
    PORT=3000 \
    HOST=0.0.0.0 \
    ENGINE_BINARY_PATH=/app/bin/compression-engine \
    STORAGE_DIR=/app/storage/jobs

WORKDIR /app

# Copy C++ engine binary
COPY --from=cpp-builder /workspace/compression-core/build/bin/compression-engine /app/bin/compression-engine
RUN chmod +x /app/bin/compression-engine

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
