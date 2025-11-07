#!/usr/bin/env python3
"""
Manually compress web files (HTML, CSS, JS) with gzip.
Run this script directly without PlatformIO build process.
"""

import os
import gzip
import shutil
from pathlib import Path

# Get script directory and project root
script_dir = Path(__file__).parent
project_dir = script_dir.parent
data_dir = project_dir / "data"

# Files to compress
COMPRESS_EXTENSIONS = ['.html', '.css', '.js', '.json', '.svg']

def compress_file(file_path):
    """Compress a file with gzip"""
    gz_path = str(file_path) + '.gz'

    # Read original file
    with open(file_path, 'rb') as f_in:
        file_data = f_in.read()

    # Write compressed file with maximum compression
    with gzip.open(gz_path, 'wb', compresslevel=9) as f_out:
        f_out.write(file_data)

    # Calculate compression ratio
    original_size = os.path.getsize(file_path)
    compressed_size = os.path.getsize(gz_path)
    ratio = (1 - compressed_size / original_size) * 100

    print(f"✓ {file_path.name}")
    print(f"  {original_size:,} → {compressed_size:,} bytes ({ratio:.1f}% smaller)")

    return compressed_size, original_size

def compress_web_files():
    """Compress all web files in the data directory"""
    if not data_dir.exists():
        print(f"❌ Data directory not found: {data_dir}")
        return

    print("\n" + "="*50)
    print("   Compressing Web Files")
    print("="*50 + "\n")

    total_original = 0
    total_compressed = 0
    files_compressed = 0

    # Walk through data directory
    for file_path in data_dir.rglob('*'):
        if not file_path.is_file():
            continue

        # Skip already compressed files
        if file_path.suffix == '.gz':
            continue

        # Compress if it's a web file
        if file_path.suffix.lower() in COMPRESS_EXTENSIONS:
            try:
                compressed, original = compress_file(file_path)
                total_compressed += compressed
                total_original += original
                files_compressed += 1
                print()  # Empty line between files
            except Exception as e:
                print(f"❌ Error compressing {file_path.name}: {e}\n")

    if files_compressed > 0:
        total_ratio = (1 - total_compressed / total_original) * 100
        print("="*50)
        print("   Compression Summary")
        print("="*50)
        print(f"Files compressed:     {files_compressed}")
        print(f"Original size:        {total_original:,} bytes")
        print(f"Compressed size:      {total_compressed:,} bytes")
        print(f"Total reduction:      {total_ratio:.1f}%")
        print(f"Bytes saved:          {total_original - total_compressed:,}")
        print("="*50 + "\n")
    else:
        print("\n⚠️  No files found to compress\n")

if __name__ == "__main__":
    compress_web_files()
