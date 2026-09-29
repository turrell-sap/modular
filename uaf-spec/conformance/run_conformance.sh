#!/usr/bin/env bash
# Build and run the UAF-SPEC-001 v2.1 conformance suite.
set -euo pipefail
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null
cmake --build build -j"$(nproc)" >/dev/null
ctest --test-dir build --output-on-failure
