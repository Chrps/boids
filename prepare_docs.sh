#!/bin/bash

# Clean up old docs
rm -rf docs

# Move build to docs
cp -r build docs

# Rename HTML file
mv docs/game.html docs/index.html

# Remove the native game binary (if it exists)
rm -f docs/game

echo "Docs folder prepared for GitHub Pages."