#!/bin/bash
cd ~/term_project

# Kill any leftover battle processes
pkill -u $USER -f './battle' 2>/dev/null
sleep 1

PORT=19876

echo "=== Starting Host ==="
# Host gets: buy 1 infantry (5 of them), then 0 to finish, then 'n' for no rematch
# Using a named pipe for interactive input
mkfifo /tmp/host_pipe_$$ 2>/dev/null
mkfifo /tmp/join_pipe_$$ 2>/dev/null

# Start host in background, feed it piped input
(sleep 8; echo "1"; sleep 1; echo "5"; sleep 1; echo "0"; sleep 15; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo "n") | timeout 120 ./battle --host --port $PORT --name TestHost > /tmp/host_out_$$ 2>&1 &
HOST_PID=$!

sleep 3

echo "=== Starting Join ==="
# Join gets: buy 2 tanks (2 of them), then 0 to finish, then 'n' for no rematch
(sleep 5; echo "2"; sleep 1; echo "2"; sleep 1; echo "0"; sleep 15; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo ""; sleep 2; echo "n") | timeout 120 ./battle --join localhost $PORT --name TestJoin > /tmp/join_out_$$ 2>&1 &
JOIN_PID=$!

echo "Waiting for game to complete (up to 120s)..."
wait $HOST_PID 2>/dev/null
HOST_EXIT=$?
wait $JOIN_PID 2>/dev/null
JOIN_EXIT=$?

echo ""
echo "========================================="
echo "HOST OUTPUT (exit=$HOST_EXIT):"
echo "========================================="
cat /tmp/host_out_$$

echo ""
echo "========================================="
echo "JOIN OUTPUT (exit=$JOIN_EXIT):"
echo "========================================="
cat /tmp/join_out_$$

# Cleanup
rm -f /tmp/host_out_$$ /tmp/join_out_$$ /tmp/host_pipe_$$ /tmp/join_pipe_$$
