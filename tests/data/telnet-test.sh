spawn telnet localhost 23000
sleep 1

# Test message
send "Test Message\r"
sleep 1

send "Unknown Message\r"
sleep 1

send "help\r"
sleep 1

send "he\t\r"
sleep 1

send "enable log v\r"
sleep 1

send "enable log vv\r"
sleep 1

send "enable log vvv\r"
sleep 1

send "disable log\r"
sleep 1

send "disable log all\r"
sleep 1

send "quit\r"
sleep 1

sleep 1
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
spawn telnet localhost 23000
sleep 3
