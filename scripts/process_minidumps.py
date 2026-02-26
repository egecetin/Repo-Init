#!/usr/bin/env python3
"""
Script to process minidump files using minidump_stackwalk and symbol files.
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path


def find_minidump_files(dump_dir):
    """Find all minidump files in the specified directory."""
    dump_path = Path(dump_dir)
    if not dump_path.exists():
        print(f"Error: Dump directory '{dump_dir}' does not exist")
        return []

    # Find all .dmp files
    minidump_files = list(dump_path.glob("*.dmp"))
    return minidump_files


def process_minidump(minidump_stackwalk_path, minidump_file, symbol_dir):
    """Process a single minidump file and print the stackwalk."""
    print(f"\n{'='*80}")
    print(f"Processing: {minidump_file.name}")
    print(f"{'='*80}\n")

    try:
        # Run minidump_stackwalk with the minidump file and symbol directory
        result = subprocess.run(
            [minidump_stackwalk_path, str(minidump_file), symbol_dir],
            capture_output=True,
            text=True,
            timeout=30,
        )

        if result.returncode == 0:
            print(result.stdout)
        else:
            print(f"Error processing {minidump_file.name}:")
            print(result.stderr)

        return result.returncode == 0

    except subprocess.TimeoutExpired:
        print(f"Timeout processing {minidump_file.name}")
        return False
    except Exception as e:
        print(f"Exception processing {minidump_file.name}: {e}")
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Process minidump files with minidump_stackwalk"
    )
    parser.add_argument(
        "--minidump-stackwalk",
        required=True,
        help="Path to minidump_stackwalk executable",
    )
    parser.add_argument("--dump-dir", help="Directory containing minidump files")
    parser.add_argument(
        "--symbol-dir", required=True, help="Directory containing symbol files"
    )
    parser.add_argument(
        "--file",
        help="Process a specific minidump file instead of all files in dump-dir",
    )

    args = parser.parse_args()

    # Either --dump-dir or --file must be provided
    if not args.dump_dir and not args.file:
        parser.error("Either --dump-dir or --file must be provided")

    if args.dump_dir and args.file:
        parser.error("Cannot specify both --dump-dir and --file")

    # Verify minidump_stackwalk exists
    stackwalk_path = Path(args.minidump_stackwalk)
    if not stackwalk_path.exists():
        print(f"Error: minidump_stackwalk not found at '{args.minidump_stackwalk}'")
        return 1

    # Verify symbol directory exists
    symbol_path = Path(args.symbol_dir)
    if not symbol_path.exists():
        print(f"Error: Symbol directory '{args.symbol_dir}' does not exist")
        return 1

    # Get minidump files to process
    if args.file:
        minidump_files = [Path(args.file)]
        if not minidump_files[0].exists():
            print(f"Error: Minidump file '{args.file}' does not exist")
            return 1
    else:
        minidump_files = find_minidump_files(args.dump_dir)

    if not minidump_files:
        print(f"No minidump files found in '{args.dump_dir}'")
        return 0

    print(f"Found {len(minidump_files)} minidump file(s) to process")
    print(f"Using symbol directory: {args.symbol_dir}")
    print(f"Using minidump_stackwalk: {args.minidump_stackwalk}")

    # Process each minidump file
    success_count = 0
    for minidump_file in minidump_files:
        if process_minidump(str(stackwalk_path), minidump_file, args.symbol_dir):
            success_count += 1

    print(f"\n{'='*80}")
    print(
        f"Processed {success_count}/{len(minidump_files)} minidump files successfully"
    )
    print(f"{'='*80}")

    return 0 if success_count == len(minidump_files) else 1


if __name__ == "__main__":
    sys.exit(main())
