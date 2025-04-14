#!/usr/bin/env bash

set -ev

if [[ -z $GITHUB_ACTION ]]; then
  ruff format zeus_apple_silicon tests
else
  ruff format --check zeus_apple_silicon tests
fi

ruff check zeus_apple_silicon
pyright zeus_apple_silicon
