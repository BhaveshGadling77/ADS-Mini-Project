#!/usr/bin/env bash
set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo ""
echo "Building CODEC — ADS Compression Engine"
echo ""

echo "[1/3] Building C++ Compression Core..."
cmake -B "$ROOT_DIR/compression-core/build" -S "$ROOT_DIR/compression-core"
cmake --build "$ROOT_DIR/compression-core/build"

echo "[2/3] Building Fastify TypeScript Backend..."
cd "$ROOT_DIR/server"
npm install --silent
npm run build

echo "[3/3] Building React + Vite Web Frontend..."
cd "$ROOT_DIR/web"
npm install --silent
npm run build

echo ""
echo "✓ Full system build completed successfully!"
echo ""
