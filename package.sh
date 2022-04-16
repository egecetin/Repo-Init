#!/bin/bash

sh makeself/makeself.sh --notemp --sha256 WorkDirectory/ "RepoInit.run" "Repo Initializer" sh scripts/init.sh
mkdir -p Dist
mv RepoInit.run Dist/
