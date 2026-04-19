# Distributed P2P Battle Simulator

A highly concurrent, feature-rich peer-to-peer terminal strategy game written in C++ designed to run across distributed nodes. Players spend points to build specialized armies and pit them against each other or engage in massive multi-client team battles.

This project was developed for COP5570, specifically tailored to operate seamlessly on the `linprog` cluster using POSIX network sockets, `pthread` threading, and a custom synchronization protocol.

---

## 🏗 Build & Run Setup

To compile the codebase, simply run:
```bash
make
```
*(Optionally, use `make clean` to remove build artifacts).*

Because of firewalls on the `linprog` cluster, external network traffic is restricted. To test distributed interactions, **you and your friends should SSH into the same linprog node** (e.g., `linprog1.cs.fsu.edu`) and use `localhost` for testing, or rely on internal node-to-node routing if allowed by the campus net.

---

## 🎮 Game Modes & Usage

### 1. Standard 1v1 Battle
A classic 1v1 match utilizing localized P2P decentralized protocol syncing.
**Terminal 1 (Host):**
```bash
./battle --host --port 5000 --name P1_Host
```
**Terminal 2 (Client):**
```bash
./battle --join localhost 5000 --name P2_Join
```

### 2. Auto-Discovery Matchmaking
Instead of manually guessing the IP of a host, provide a list of known nodes (e.g., `nodes.txt`) and let the client automatically iterate through them until a successful TCP connection is found.
```bash
./battle --join-any nodes.txt 5000 --name P2_Join
```

### 3. Spectator Mode (Third-Node Observer)
Broadcast live battle events via UDP streams allowing a third remote node to observe without participating in the combat logic.

**Terminal 3 (The Spectator):** Start the listener first.
```bash
./battle --spectator-listen 9000
```
**Terminal 1 & 2 (The Players):** Connect the game, but feed the spectator node IP.
```bash
./battle --host --port 5000 --spectator localhost 9000
./battle --join localhost 5000 --spectator localhost 9000
```

### 4. Massive 15+ Player Team Battles
A newly implemented Star-Topology game mode designed for live classroom demos. Players all connect to a single central host. The host automatically aggregates everyone's 200pt modular armies, dynamically halves them into "Team A" and "Team B", and beams the massive battle-state to all clients where the battle completes simultaneously.
**Terminal 1 (The Relay Host):**
```bash
./battle --host-team --port 9050 --name "LobbyLeader"
```
**Terminals 2 through 15 (The Classmates):**
```bash
./battle --join-team localhost 9050 --name "Student"
```
*(Wait in the lobby on the Host to design their army. Once it's complete, the Massive Team Fight commences).*

---

## ⚔️ Game Mechanics

### Army Design
Upon connecting, players enter an interactive Terminal UI to spend their **200 Budget Points** on 6 diverse military units:

| Unit Name        | Cost | HP  | ATK | DEF | Special Abilities |
| ---------------- | ---- | --- | --- | --- | ----------------- |
| Infantry         | 10   | 50  | 15  | 5   | Basic ground troops. |
| Tank             | 30   | 120 | 40  | 20  | +50% ATK against enemy Infantry. |
| Artillery        | 35   | 60  | 55  | 3   | High priority targets; attacks fiercely. |
| Navy Destroyer   | 25   | 100 | 30  | 15  | Naval unit. Receives +50% DEF against Missiles. |
| Missile Launcher | 40   | 40  | 70  | 0   | Anti-bunker; completely ignores enemy DEF. |
| Shield Wall      | 20   | 150 | 0   | 30  | Meat shield. Absorbs damage for allies but cannot attack. |

*(Combat is turn-based, but heavily layered with complex multiplier roles and strategic priority targeting algorithms).*

---

## 🌪 Advanced Features

### 1. Dynamic Environmental Effects
Every game generates a randomized biome that drastically affects damage output:
*   **Weather Effects:** Clear, Rain, Fog (Creates 20% miss chance for ranged units), Storm, and Heatwave (buffs Infantry, debuffs Tanks).
*   **Map Effects:** Plains, Urban District (Infantry takes cover), Mountain Pass (Artillery gains high ground), Coastal Front (Naval dominance), Ancient Ruins (Shields weakened).

### 2. Chaotic World Events
Matches are rarely straightforward. Specific rounds may trigger world-breaking conditions:
*   **EMP Disruptions:** Occurs randomly mid-match. Disables all computerized units (Artillery & Missiles) for a full round. 
*   **Third-Party Drones:** Rogue forces enter the battlefield, dealing unavoidable raw damage to one random target on both sides.

### 3. Role-Based AI Targeting
Unlike typical random battle combat, units actively seek the best outcome. Artillery targets enemy Artillery/Missiles first; Missiles specifically hunt enemy Tanks. If no priority target exists, troops default to focusing down the lowest HP non-shield enemies to eliminate immediate threats.

### 4. Protocol Resiliency & Synchronization
The backbone of the application runs on a heavily fortified serialization protocol (`protocol.cpp`). 
*   **Name Exchange:** Opponents learn each other's custom handle dynamically (`MSG_NAME`).
*   **Rematch Negotiators:** Built-in protocol for players to request mutual rematches (`MSG_REMATCH`) without breaking TCP connections.
*   **Anti-Desync Hashes:** After a battle, both clients hash their entire combat log matrices layer by layer (`MSG_STATE_HASH`). If the hashes do not match, the game aggressively faults out, alerting the user to a desync event! 

---
*Built aggressively for performance, threading optimization, and tactical warfare terminal mastery!*
