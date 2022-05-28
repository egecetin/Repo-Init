#!/bin/bash

read -p "Enter new name: " replace

sed -i "s/XXX/${replace}/" *.*
sed -i "s/XXX_Lib/${replace}_Lib/" *.*
sed -i "s/XXX/${replace}/" include/**
sed -i "s/XXX/${replace}/" src/**
