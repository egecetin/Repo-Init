spawn telnet localhost 23000
sleep 1

foreach entry [list "Test Message\r" "Unknown Message\r" "help\r" "\b" "\t" "he\t\r" "enable log v\r" "enable log vv\r" \
                    "clear\r" "enable log vvv\r" "disable log\r" "disable log all\r" "version\r" "\x1b\x5b\x41\r" "\x1b\x5b\x42\r" "\r" "quit\r"] {
    send $entry
    sleep 1
}

sleep 1
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
sleep 3
