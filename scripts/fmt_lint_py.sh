#!/usr/bin/env bash

set -ev

if [[ -z $GITHUB_ACTION ]]; then
  ruff format bindings tests
else
  ruff format --check bindings tests
fi

ruff check bindings
pyright bindings
