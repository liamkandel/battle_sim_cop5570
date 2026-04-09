#!/usr/bin/env python3
import sys

def main() -> int:
    if len(sys.argv) < 2:
        print("Usage: python3 tools_sync_summary.py <log1> [log2 ...]")
        return 1

    matched = 0
    desync = 0
    rounds = []

    for path in sys.argv[1:]:
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            data = f.read()
        if "[SYNC CHECK] Round hashes matched" in data:
            matched += 1
        if "[SYNC CHECK] Desync detected at round" in data:
            desync += 1
            marker = "[SYNC CHECK] Desync detected at round "
            idx = data.find(marker)
            if idx >= 0:
                tail = data[idx + len(marker):].split(".", 1)[0].strip()
                if tail.isdigit():
                    rounds.append(int(tail))

    total = len(sys.argv) - 1
    print(f"Total logs: {total}")
    print(f"Matched:    {matched}")
    print(f"Desync:     {desync}")
    if rounds:
        print(f"Avg first mismatch round: {sum(rounds)/len(rounds):.2f}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
