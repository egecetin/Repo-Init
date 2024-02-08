#!/usr/bin/python3

import errno
import os
import sys
from optparse import OptionParser


if __name__ == "__main__":
    # Parse inputs
    parser = OptionParser()
    parser.usage = "Dump symbol files using Google breakpad dump_syms"
    parser.add_option("", "--dump-syms")
    parser.add_option("", "--binary")
    parser.add_option("", "--output-dir")
    (options, args) = parser.parse_args()

    dumpSymsExe = options.dump_syms
    binaryPath = options.binary
    outputDir = options.output_dir

    # Check all inputs
    if dumpSymsExe is None or binaryPath is None or outputDir is None:
        print("Provide all arguments")
        sys.exit(1)

    # Prepare command to dump symbols
    outFilePath = outputDir + os.path.basename(binaryPath) + ".sym"
    command = f"{dumpSymsExe} {binaryPath} > {outFilePath}"

    print(command)
    os.system(command)

    # Prepare desired folder structure
    symFile = open(outFilePath, "r")
    firstLine = symFile.readline().split(" ")

    symbolsDir = (
        outputDir + "/" + os.path.basename(binaryPath) + "/" + firstLine[-2] + "/"
    )

    try:
        os.makedirs(symbolsDir)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise
    os.rename(outFilePath, symbolsDir + os.path.basename(binaryPath) + ".sym")
    os.system(f"strip {binaryPath}")
