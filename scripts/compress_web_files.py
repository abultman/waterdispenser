#!/usr/bin/env python3
"""
Compress web files (HTML, CSS, JS) with gzip for efficient serving.
This script is run automatically during PlatformIO build.
"""

import os
import gzip
import shutil
from pathlib import Path

# Get the project root directory
Import("env")
project_dir = env.get("PROJECT_DIR")
data_dir = os.path.join(project_dir, "data")

# Files to compress
COMPRESS_EXTENSIONS = ['.html', '.css', '.js', '.json', '.svg']

def compress_file(file_path):
    """Compress a file with gzip"""
    gz_path = file_path + '.gz'

    # Read original file
    with open(file_path, 'rb') as f_in:
        file_data = f_in.read()

    # Write compressed file
    with gzip.open(gz_path, 'wb', compresslevel=9) as f_out:
        f_out.write(file_data)

    # Calculate compression ratio
    original_size = os.path.getsize(file_path)
    compressed_size = os.path.getsize(gz_path)
    ratio = (1 - compressed_size / original_size) * 100

    print(f"Compressed: {os.path.basename(file_path)}")
    print(f"  Original: {original_size} bytes")
    print(f"  Compressed: {compressed_size} bytes ({ratio:.1f}% reduction)")

    return compressed_size, original_size

def compress_web_files():
    """Compress all web files in the data directory"""
    if not os.path.exists(data_dir):
        print(f"Data directory not found: {data_dir}")
        return

    print("\n=== Compressing Web Files ===")
    total_original = 0
    total_compressed = 0
    files_compressed = 0

    # Walk through data directory
    for root, dirs, files in os.walk(data_dir):
        for file in files:
            file_path = os.path.join(root, file)
            ext = os.path.splitext(file)[1].lower()

            # Skip already compressed files
            if file.endswith('.gz'):
                continue

            # Compress if it's a web file
            if ext in COMPRESS_EXTENSIONS:
                try:
                    compressed, original = compress_file(file_path)
                    total_compressed += compressed
                    total_original += original
                    files_compressed += 1
                except Exception as e:
                    print(f"Error compressing {file}: {e}")

    if files_compressed > 0:
        total_ratio = (1 - total_compressed / total_original) * 100
        print(f"\n=== Compression Summary ===")
        print(f"Files compressed: {files_compressed}")
        print(f"Total original size: {total_original} bytes")
        print(f"Total compressed size: {total_compressed} bytes")
        print(f"Total savings: {total_ratio:.1f}%")
        print(f"Saved: {total_original - total_compressed} bytes\n")
    else:
        print("No files to compress\n")

# Run compression before building filesystem
compress_web_files()
