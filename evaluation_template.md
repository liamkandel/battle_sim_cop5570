# Evaluation Template

Use this table in your report/demo notes to document distributed correctness and performance.

| Test ID | Nodes Used | Network Setup | Matches | Sync Pass Rate | Avg Rounds | Avg Runtime (s) | Disconnect Recovery | Notes |
|---|---|---|---:|---:|---:|---:|---|---|
| E1 | linprog1/linprog2 | normal | 20 |  |  |  |  |  |
| E2 | linprog1/linprog2 | delayed/loaded | 20 |  |  |  |  |  |
| E3 | linprog1/linprog2/linprog3 | spectator enabled | 20 |  |  |  |  |  |
| E4 | discovery mode | nodes.txt | 20 |  |  |  |  |  |

## Minimum report metrics

- Number of matches with `SYNC CHECK matched`
- Number of desyncs and first mismatch round
- Time to connect (manual and discovery mode)
- Behavior when one node disconnects mid-session
