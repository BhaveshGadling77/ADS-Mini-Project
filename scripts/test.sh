#!/usr/bin/env bash
set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Running CODEC Automated Test Suites"

echo "[1/2] Running C++ Engine Test Suite..."
"$ROOT_DIR/compression-core/build/bin/test_runner"

echo "[2/2] Running Backend Fastify Integration Tests..."
cd "$ROOT_DIR/server"
npx tsx tests/server.test.ts

echo "✓ All test suites passed successfully!"
