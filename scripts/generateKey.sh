#!/bin/bash

openssl genrsa -out server.key 4096
openssl req -new -key server.key -out server.csr
openssl x509 -req -days 3650 -in server.csr -signkey server.key -out server.crt
openssl dhparam -out dh2048.pem 2048
