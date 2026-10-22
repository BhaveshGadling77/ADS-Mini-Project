#!/usr/bin/env bash
set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo ""
echo "Building CODEC — ADS Compression Engine"
echo ""

echo "[1/6] Building integrated C++ Compression Core..."
cmake -B "$ROOT_DIR/compression-core/build" -S "$ROOT_DIR/compression-core"
cmake --build "$ROOT_DIR/compression-core/build"

echo "[2/6] Building standalone BMP image module..."
make -C "$ROOT_DIR/compression-core/modules/image"

echo "[3/6] Building standalone audio module..."
cd "$ROOT_DIR/compression-core/modules/audio"
g++ -std=c++17 -Wall -Wextra audioCompression.cpp -o audioCompression
g++ -std=c++17 -Wall -Wextra audioDecompression.cpp -o audioDecompression

echo "[4/6] Building standalone video module (OpenCV 4 required)..."
g++ -std=c++17 "$ROOT_DIR/compression-core/modules/video/standalone/videoCompress.cpp" $(pkg-config --cflags opencv4) -lopencv_imgproc -lopencv_core -o "$ROOT_DIR/compression-core/modules/video/videoCompress"
g++ -std=c++17 "$ROOT_DIR/compression-core/modules/video/standalone/videoDecompress.cpp" $(pkg-config --cflags opencv4) -lopencv_imgproc -lopencv_core -o "$ROOT_DIR/compression-core/modules/video/videoDecompress"

echo "[5/6] Building Fastify TypeScript Backend..."
cd "$ROOT_DIR/server"
npm install --silent
npm run build

echo "[6/6] Building React + Vite Web Frontend..."
cd "$ROOT_DIR/web"
npm install --silent
npm run build

echo ""
echo "✓ Full system build completed successfully!"
echo ""
