#!/bin/bash
cd ~/term_project

# Kill any leftover battle processes
pkill -u $USER -f './battle' 2>/dev/null
sleep 1

PORT=19877

echo "=== Starting Team Host ==="
# Host designs an army (1 Infantry x5) then waits. When done, presses '0' and enter.
mkfifo /tmp/host_team_pipe_$$ 2>/dev/null
# Sleep 15 will give enough time for clients to join. Then host presses Enter to start.
# Then host designs army: 1 (Infantry), 5 (qty), 0 (done).
(sleep 15; echo ""; sleep 2; echo "1"; sleep 1; echo "5"; sleep 1; echo "0"; sleep 30) | timeout 60 ./battle --host-team --port $PORT --name TeamHost > /tmp/host_team_out_$$ 2>&1 &
HOST_PID=$!

sleep 3

echo "=== Starting 5 Team Clients ==="
CLIENT_PIDS=""
for i in {1..5}; do
    # Each client buys 2 Tanks (2, 2, 0)
    (sleep $((2 + i)); echo "2"; sleep 1; echo "2"; sleep 1; echo "0"; sleep 30) | timeout 60 ./battle --join-team localhost $PORT --name Client$i > /tmp/client_${i}_out_$$ 2>&1 &
    CLIENT_PIDS="$CLIENT_PIDS $!"
done

echo "Waiting for game to complete (up to 60s)..."
wait $HOST_PID 2>/dev/null
for pid in $CLIENT_PIDS; do
    wait $pid 2>/dev/null
done

echo ""
echo "========================================="
echo "HOST OUTPUT:"
echo "========================================="
cat /tmp/host_team_out_$$ | tail -n 40

echo ""
echo "========================================="
echo "CLIENT 1 OUTPUT:"
echo "========================================="
cat /tmp/client_1_out_$$ | tail -n 40

# Cleanup
rm -f /tmp/host_team_out_$$ /tmp/client_*_out_$$ /tmp/host_team_pipe_$$
