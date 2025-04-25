#!/usr/bin/env bash

set -ev

if [[ -z $GITHUB_ACTION ]]; then
  python3 ./scripts/recurse_directories.py \
          -d apple_energy bindings tests examples \
          -e .cpp .h .hpp \
          -c clang-format -i -style=webkit
else
  python3 ./scripts/recurse_directories.py \
          -d apple_energy bindings tests examples \
          -e .cpp .h .hpp \
          -c clang-format --dry-run -Werror -style=webkit
fi
