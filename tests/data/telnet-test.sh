spawn telnet localhost 23000
sleep 1

# Test message
send "Test Message\r"
sleep 1

send "Unknown Message\r"
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
