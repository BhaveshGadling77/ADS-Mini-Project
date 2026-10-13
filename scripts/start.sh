#!/usr/bin/env bash
set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Starting CODEC Server (Fastify + Native C++ Engine + React Frontend)..."
cd "$ROOT_DIR/server"
npm start
