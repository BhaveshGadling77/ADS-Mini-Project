# ==============================================================================
# Bitshrink - Standalone C++ Compression Core Engine & CLI
# Multi-stage Dockerfile for C++ CLI Engine & Unit Tests
# ==============================================================================

# ------------------------------------------------------------------------------
# Stage 1: Build & Test Engine
# ------------------------------------------------------------------------------
FROM debian:bookworm-slim AS builder

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

# Run internal C++ algorithm test suite
RUN ./bin/test_runner

# ------------------------------------------------------------------------------
# Stage 2: Minimal CLI Runtime
# ------------------------------------------------------------------------------
FROM debian:bookworm-slim AS runner

RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY --from=builder /workspace/compression-core/build/bin/compression-engine /usr/local/bin/compression-engine
COPY --from=builder /workspace/compression-core/build/bin/test_runner /usr/local/bin/test_runner

ENTRYPOINT ["compression-engine"]
CMD ["info"]
