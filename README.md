# Battle Simulator

Simple peer-to-peer terminal battle game in C++.

## Build

```bash
make
```

This creates the executable: `battle`.

## Run

Open **two terminals**:

1. Start host:

```bash
./battle --host --port 5000 --name HostPlayer
```

2. Join from another terminal (same machine example):

```bash
./battle --join 127.0.0.1 5000 --name JoinPlayer
```

Use `Ctrl+C` to disconnect.

## New Distributed Features

- Player name exchange is now synced over protocol (`NAME` message).
- Round-by-round state hashes are exchanged after battle (`STATEHASH`) for desync detection.
- Rematch negotiation is fully wired (`REMATCH`).
- Target selection uses role-based AI priorities (not purely random).
- Battles now include environment complexity:
  - Random weather each match (Clear/Rain/Fog/Storm/Heatwave)
  - Random map pool each match (Plains/Urban/Mountains/Coastal/Ruins)
  - Dynamic world events (EMP disruption + third-party drone intervention)

### Auto-Discovery Join

Use a node list file (`nodes.txt`) to try multiple hosts automatically:

```bash
./battle --join-any nodes.txt 5000 --name JoinPlayer
```

### 3rd-Node Spectator Mode

On a third node:

```bash
./battle --spectator-listen 9000
```

On host/join nodes, stream battle events to the spectator:

```bash
./battle --host --port 5000 --name HostPlayer --spectator linprog3 9000
./battle --join linprog1 5000 --name JoinPlayer --spectator linprog3 9000
```

### Evaluation Helpers

- `evaluation_template.md`: report-ready metrics table.
- `tools_sync_summary.py`: summarize sync-pass/desync counts from captured logs.

## Clean

```bash
make clean
```
