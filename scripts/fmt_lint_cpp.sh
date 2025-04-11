#!/usr/bin/env bash

set -ev

if [[ -z $GITHUB_ACTION ]]; then
  python3 ./scripts/recurse_directories.py \
          -d include bindings \
          -e .cpp .h .hpp \
          -c clang-format -i -style=LLVM
else
  python3 ./scripts/recurse_directories.py \
          -d include bindings \
          -e .cpp .h .hpp \
          -c clang-format --dry-run -Werror -style=LLVM
fi
