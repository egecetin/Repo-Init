#!/bin/bash

read -p "Enter new name: " replace

sed -i "s/XXX/${replace}/" CMakeLists.txt
sed -i "s/XXX/${replace}/" scripts/data/branding/branding.css
