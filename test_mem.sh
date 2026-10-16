#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "Building target scheduler..."
make -B scheduler

if command -v valgrind > /dev/null 2>&1; then
    echo "Running Valgrind leak check..."
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./scheduler
    echo "Valgrind check passed with 0 leaks!"
else
    echo "Notice: 'valgrind' is not installed in the current environment."
    echo "Running './scheduler' directly to verify runtime behavior:"
    ./scheduler
fi
