#!/bin/bash

read -p "Enter new name: " replace

if [[ $replace != "" ]]; then
sed -i 's/XXX/$replace/' *.*
sed -i 's/XXX/$replace/' **/*
else
echo "Error"
fi

