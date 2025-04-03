#!/usr/bin/env bash

set -ev

if [[ -z $GITHUB_ACTION ]]; then
  black bindings tests
else
  black --check bindings tests
fi

ruff check bindings
pyright bindings
