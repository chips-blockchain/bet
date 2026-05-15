# VRSCTEST Local Regtest — Setup & Usage

This document describes the **local, offline-only `VRSCTEST` regtest** used as the blockchain backend for the `bet` poker app on this machine. It replaces the previous "local CHIPS PBaaS chain" approach (abandoned because patching `verusd` for an offline PBaaS launch was too invasive).

Everything below stays inside this single host (`10.29.48.33`). No external connectivity is required.

---

## 0. Agent operating rules (READ FIRST)

This guide is the canonical reference for **CLI auto test** / **auto test**. When the user says any of:

- "cli auto test"
- "auto test"
- "start the nodes" / "bring up the stack"

the agent **must follow this guide**, in particular §10 (start) and §11 (abort/teardown), and observe the hard rules below.

### Tmux sessions are pre-existing — never create or kill them

Long-lived tmux sessions (`dealer`, `cashier`, `p1`, `p2`, `blocknotify`) are already running on this host. The agent's job is to send commands into them, **not** to manage their lifecycle.

- **NEVER** run `tmux new-session` (the sessions already exist).
- **NEVER** run `tmux kill-session` (sessions stay up across tests).
- **NEVER** run `killall bet`, `pkill bet`, `pkill -f run_blocknotify`, or any process-killer that targets bet/daemon processes.
- **ONLY** use `tmux send-keys -t <session> ... Enter` to send commands, and `tmux send-keys -t <session> C-c` to interrupt the running process inside a session.

### What "abort" means

When the user says **"abort"** (or "stop", "cancel"), the agent must **only stop the `bet` processes running inside the tmux sessions** — by sending `C-c` to each session via `tmux send-keys`.

The agent must **NOT**:
- kill the tmux sessions themselves,
- kill any other process,
- stop the `verusd` daemon,
- run any cleanup, revert, or status command beyond the `C-c` interrupts.

After issuing the `C-c`s, stop and wait for the next instruction.

### Daemon, wallet, and identity actions need explicit approval

Per `interactive_testing_protocol.mdc` and `no_verus_id_creation.mdc`:

- Starting/stopping `verusd` (§4) → requires explicit user approval.
- Wiping chain data (§4 "Reset chain (nuclear)") → requires explicit user approval.
- Funding identities (§8), `sendcurrency`, `generate`, `updateidentity` (§9) → require explicit user approval.

The agent never runs any of the above unilaterally; this guide documents them only for the user's reference.

Note: the `--reset` flag in §10's dealer command is part of the canonical CLI auto setup and is kept intentionally — running §10 as documented is pre-approved.

---

## 1. Why VRSCTEST regtest

- VerusCoin already ships a `VRSCTEST` chain definition. Running it with regtest-style local settings (no DNS seeds, no peers) gives a fully isolated test chain.
- Verus IDs, `sendcurrency`, content multimaps, etc. all work natively — `bet` doesn't need to know it's not the production CHIPS PBaaS chain (a small `currency =` switch in `blockchain.ini` is the only `bet`-side knob that matters).
- We only had to add a single small `verusd` patch (`-offline` flag) so the daemon can boot without the public-net zk-SNARK params.

---

## 2. The `verusd` patch (single change)

`src/init.cpp` was patched to skip `ZC_LoadParams()` and `checkParams()` when `-offline` is passed. Snippet:

```c
// Skip Sapling parameter download/verification in fully offline test mode.
// Without these params shielded (z) transactions will not work; transparent-only.
// Intended for fully-local single-node test chains (e.g. -chain=VRSCTEST regtest).
bool fOfflineMode = GetBoolArg("-offline", false);
bool paramsVerified = false;
if (!fOfflineMode) {
    uiInterface.InitMessage(_("Verifying Params..."));
    initalizeMapParam();
    ...
}
```

That's the **only** code-side `verusd` change. No consensus changes, no chainparams changes.

The patched binary lives at:

- `/home/sarat/VerusCoin/src/verusd` — daemon
- `/home/sarat/VerusCoin/src/verus` — CLI wrapper

A symlink at `/usr/local/bin/verus` makes the CLI reachable from `bet`'s `popen()` subshells:

```bash
ln -sf /home/sarat/VerusCoin/src/verus /usr/local/bin/verus
```

**Do not remove this symlink** — without it, `bet` fails with `sh: 1: verus: not found` because subshells don't inherit interactive PATH.

---

## 3. Chain config

Config file: `/root/.komodo/vrsctest/vrsctest.conf`

```ini
# Local-only VRSCTEST regtest config (no external network)
server=1
rpcuser=vrsctest_user_local
rpcpassword=vrsctest_pass_b91a6c3df2e9e7d4a
rpcport=18843
rpcbind=127.0.0.1
rpchost=127.0.0.1
rpcallowip=127.0.0.1
rpcworkqueue=64

# P2P locked to localhost
listen=0
port=18844
dnsseed=0
maxconnections=0

# Mining
gen=1
genproclimit=1

# Indexing (handy for tests)
txindex=1
addressindex=1
spentindex=1

# Logging
debug=1
printtoconsole=0
```

Key points:
- `rpcbind` / `rpcallowip = 127.0.0.1` — RPC only on loopback.
- `dnsseed=0`, `maxconnections=0` — never tries to peer with anything.
- `gen=1 genproclimit=1` — mines blocks on a single thread continuously, so transactions confirm on their own without an external miner.
- `txindex/addressindex/spentindex` enabled so `getaddressbalance`, `getrawtransaction <txid> 1`, `getidentitycontent ... <height>` all work.

Data directory: `/root/.komodo/vrsctest/`. Notable files:
- `blocks/` — block chain
- `chainstate/` — UTXO db
- `wallet.dat` — wallet (holds keys for funded addresses + sub-IDs)
- `debug.log` — daemon log
- `verusd.pid` — running PID

---

## 4. Starting / stopping the daemon

### Start (offline)

```bash
cd /home/sarat/VerusCoin/src
./verusd -chain=VRSCTEST -offline -daemon
```

The `-offline` flag triggers the patched skip of zk-SNARK param loading.

### Wait until ready

```bash
verus -chain=VRSCTEST getblockcount
```

Returns a number once the daemon is up.

### Stop

> **Agent rule:** stopping the daemon requires explicit user approval. The agent must not issue these commands unilaterally.

```bash
verus -chain=VRSCTEST stop
```

Or hard stop:

```bash
kill $(cat /root/.komodo/vrsctest/verusd.pid)
```

### Reset chain (nuclear)

> **Agent rule:** wiping the chain destroys all identities and balances and requires explicit user approval. Never run this autonomously.

If you ever need a virgin chain (fresh genesis), stop the daemon then `rm -rf /root/.komodo/vrsctest/{blocks,chainstate,wallet.dat,fee_estimates.dat,peers.dat}`. **All identities and balances will be wiped — you'll need to redo §6 from scratch.** Don't do this without a strong reason; existing IDs and funds take a while to re-create.

---

## 5. Helper script: `run_blocknotify.sh`

`verusd -blocknotify=…` would normally invoke `bet newblock <hash>` after every confirmed block. Since we don't pass that flag (keeps the daemon minimal), a polling shim does the same job:

`/home/sarat/bet/poker/run_blocknotify.sh`

```bash
#!/bin/bash
set -u
cd /home/sarat/bet/poker
VC='verus -chain=VRSCTEST'
last=""
while true; do
  hash=$($VC getbestblockhash 2>/dev/null) || { sleep 2; continue; }
  if [ -n "$hash" ] && [ "$hash" != "$last" ]; then
    echo "[$(date +%T)] new block: $hash" >> /tmp/blocknotify.log
    ./bin/bet newblock "$hash" >> /tmp/blocknotify.log 2>&1 || true
    last="$hash"
  fi
  sleep 1
done
```

**Currently unused in CLI auto.** The only consumer was the cashier's `process_block` path, which `PLAYER_JOIN_FLOW.md §1.3` documents as dead code for joins (`sendcurrency` payins carry no embedded data, so `chips_extract_tx_data_in_JSON` always returns NULL). The cashier now self-discovers the active table via its own poll loop on player identities (`PLAYER_JOIN_FLOW.md §1.4`), so blocknotify is **not** part of the §10 CLI auto bring-up.

The script and `blocknotify` tmux session stay around for future use (e.g. dispute-resolution polling — `docs/TODO.md` item 2). The agent must still not create or kill the session.

---

## 6. Verus IDs registered on this chain

`sg777z.VRSCTEST@` is the parent / namespace. All `bet`-side IDs are sub-IDs of it.

| Name (FQN) | i-address | Role |
|---|---|---|
| `sg777z.VRSCTEST@` | `i6Q1xEn3MpbJA4vuy9aBdDuWwwXtTmD6hg` | parent namespace |
| `dealer.VRSCTEST@` | `i7yo8ErqbGPfJxa9xxkWLHTULk5Sjq6sMi` | dealer aggregator (CMM stores `dealers` list) |
| `d1.VRSCTEST@` | `iJ3zGH2sDxejSixEKND8TKQ53UmhC4yxoY` | dealer-1 instance |
| `t1.VRSCTEST@` | `i7fbC9ZC4qyKaCG6D4LBHsh1YZGs6N8DPC` | table-1 (game state lives here) |
| `cashier.VRSCTEST@` | `iJpM1EpZRCzCTqpUoqWy3LjJUeS8eMxtFC` | cashier aggregator + payin/payout custody |
| `p1.VRSCTEST@` | `i5dNbqvQHQ8Y4kHc8yZZ3ru8Ckh2SPTuqY` | player 1 |
| `p2.VRSCTEST@` | `i6N1Fy7p6vR9Dp4Fok9KAUEPX8Pt7vWemZ` | player 2 |

(Listed via `verus -chain=VRSCTEST listidentities`.)

To inspect an ID's full state:

```bash
verus -chain=VRSCTEST getidentity 't1.sg777z.VRSCTEST@'
verus -chain=VRSCTEST getidentitycontent 't1.sg777z.VRSCTEST@'
verus -chain=VRSCTEST getidentitycontent 't1.sg777z.VRSCTEST@' <start_block> -1   # CMM merge view from height
```

The `-1` end-height means "to chain tip".

---

## 7. `bet`-side configuration

Two ini files were updated to point at VRSCTEST instead of the abandoned local CHIPS chain.

### `poker/config/blockchain.ini`

```ini
[blockchain]
blockchain_cli = "verus -chain=VRSCTEST"   # Local VRSCTEST regtest
currency       = VRSCTEST                  # Native currency name used in sendcurrency (defaults to "chips" if missing)
new_block      = "Y"                       # Cashier processes payin_tx requests on every new block
```

`currency = VRSCTEST` is wired into `bet_get_currency()` (`poker/src/config.c`) and used by `verus_sendcurrency_data` so payins/payouts go in native VRSCTEST satoshis.

### `poker/config/keys.ini`

```ini
[identities]
cashier_id = cashier.sg777z.VRSCTEST@
dealers_id = dealers.sg777z.VRSCTEST@
```

Both fields must be fully-qualified Verus IDs; the parser rejects
bare short names. The parent is implicit in each FQN — there is no
separate `parent_id` field.

VDXF key names are not in `keys.ini` — the prefix
(`chips.vrsc::poker.`) and every key name are compile-time
macros in `poker/include/vdxf.h` (`VDXF_POKER_KEYS_PREFIX` and the
`*_KEY` family). The prefix is opaque to Verus and intentionally
unchanged for VRSCTEST so the same key macros stay portable when
deployment moves chains.

### Player ini files (e.g. `poker/config/p1.ini`)

```ini
[verus]
dealer_id   = d1.sg777z.VRSCTEST@
table_id    = t1.sg777z.VRSCTEST@
wallet_addr = *
player_id   = p1.sg777z.VRSCTEST@
ws_port     = 9001     # WebSocket port for GUI mode
```

All identity fields are fully-qualified Verus IDs. The parser rejects
short names with `ERR_INI_PARSING`.

---

## 8. Funding identities

`bet` requires each identity (cashier, dealer, p1, p2) to have spendable VRSCTEST satoshis on its primary address so it can sign `updateidentity` and `sendcurrency` transactions.

### Mining → coinbase

`gen=1` in `vrsctest.conf` already mines into a wallet address. The wallet currently holds:

```bash
$ verus -chain=VRSCTEST getbalance
50443046.00000000
```

(50.4 million VRSCTEST in the wallet — plenty for testing.)

### Funding a sub-ID

Identities can hold funds at their primary i-address. To top up:

```bash
verus -chain=VRSCTEST sendtoaddress <i-address> <amount>
```

Then mine/wait one block. Verify:

```bash
verus -chain=VRSCTEST getaddressbalance '{"addresses":["<i-address>"]}'
```

Current funding (sample, p1):

```bash
$ verus -chain=VRSCTEST getaddressbalance '{"addresses":["i5dNbqvQHQ8Y4kHc8yZZ3ru8Ckh2SPTuqY"]}'
{ "balance": 20500000000, ... }    # 205 VRSCTEST on p1's i-address
```

That's enough for hundreds of `updateidentity` rounds + payins.

---

## 9. Common operations cheat sheet

```bash
# Tip / sync
verus -chain=VRSCTEST getblockcount
verus -chain=VRSCTEST getbestblockhash
verus -chain=VRSCTEST getinfo

# Wallet / balances
verus -chain=VRSCTEST getbalance
verus -chain=VRSCTEST listunspent 1 99999

# Identities
verus -chain=VRSCTEST listidentities
verus -chain=VRSCTEST getidentity 'p1.sg777z.VRSCTEST@'
verus -chain=VRSCTEST getidentitycontent 't1.sg777z.VRSCTEST@'
verus -chain=VRSCTEST getidentitycontent 't1.sg777z.VRSCTEST@' <from_height> -1

# Transactions
verus -chain=VRSCTEST getrawtransaction <txid> 1     # verbose JSON
verus -chain=VRSCTEST z_getoperationstatus '["<opid>"]'
verus -chain=VRSCTEST sendcurrency '*' '[{"currency":"VRSCTEST","amount":0.5,"address":"cashier.sg777z.VRSCTEST@"}]' 1 0.0001

# Manual block generation (occasionally useful)
verus -chain=VRSCTEST generate 1

# RPC convenience alias for shell sessions
alias _vc='verus -chain=VRSCTEST'
```

---

## 10. Bringing up the full stack for a test (CLI auto)

Preconditions (the agent must verify, not perform, these steps unless explicitly approved):

1. **Daemon already running** — confirm with `verus -chain=VRSCTEST getblockcount` (returns a number). Starting the daemon requires explicit user approval.
2. **`bet` binary built** — if `bet` source was changed in this session, run `cd /home/sarat/bet/poker && make -j$(nproc)` (build is allowed; running the binary is not, until step 3).
3. **Tmux sessions up** — `tmux ls` must list `dealer`, `cashier`, `p1`, `p2`, `blocknotify`. The agent does **not** create them. If any is missing, stop and tell the user.

Then start the nodes by **sending commands into the existing sessions** (CLI auto: `--auto` for players). Order matters — dealer first, then cashier, then players. Blocknotify is **not** started — see §5; the cashier self-discovers the active table from `P_JOIN_REQUEST_KEY` writes by players (`PLAYER_JOIN_FLOW.md §1.4`).

```bash
# dealer (--reset kept intentionally as part of the canonical CLI auto setup)
tmux send-keys -t dealer "./bin/bet start dealer --config config/dealer.ini --cli --reset 2>&1 | tee /root/.bet/logs/dealer.log" Enter
sleep 5

# cashier — no --table_id argument; cashier idle-polls player IDs until the
# first matching join request arrives, then learns table_id and start_block
# from that request and the table's CMM (see PLAYER_JOIN_FLOW.md §1.4).
tmux send-keys -t cashier "./bin/bet start cashier --config config/cashier.ini --cli 2>&1 | tee /root/.bet/logs/cashier.log" Enter
sleep 4

# player 1 (CLI auto)
tmux send-keys -t p1 "./bin/bet start player --config config/p1.ini --auto 2>&1 | tee /root/.bet/logs/p1.log" Enter
sleep 3

# player 2 (CLI auto)
tmux send-keys -t p2 "./bin/bet start player --config config/p2.ini --auto 2>&1 | tee /root/.bet/logs/p2.log" Enter
```

Notes:

- Logs go to `/root/.bet/logs/{dealer,cashier,p1,p2}.log`. The agent reads these via the file tools, not by attaching to tmux.
- The user attaches to any session with `tmux attach -t <name>` and detaches with `Ctrl-b d`. The agent should not attach.

To inspect what a session is currently showing without attaching:

```bash
tmux capture-pane -t cashier -p           # last screen
tmux capture-pane -t cashier -p -S -200   # last 200 lines of scrollback
```

---

## 11. Abort / teardown

### "abort" (default — stop bet processes only)

When the user says "abort" / "stop" / "cancel", interrupt the `bet` process running inside each tmux session by sending `C-c`. Do **not** kill the tmux sessions and do **not** kill any other process.

```bash
tmux send-keys -t p2        C-c
tmux send-keys -t p1        C-c
tmux send-keys -t cashier   C-c
tmux send-keys -t dealer    C-c
```

(The `blocknotify` session is no longer started in CLI auto — see §5 — so it has nothing to interrupt. If you ever ran the polling shim by hand, send `C-c` to that session as well.)

After issuing the interrupts, stop and wait for the next instruction. No follow-up cleanup, no status reports, no daemon stop.

### Full teardown (only with explicit user approval)

The agent must not run these without an explicit ask:

```bash
# Stop daemon (requires approval per interactive_testing_protocol.mdc)
verus -chain=VRSCTEST stop
```

`tmux kill-session` and `pkill`/`killall` are **forbidden** for this project — sessions and processes are managed by the user, not the agent.

---

## 12. Files / paths summary

| What | Path |
|---|---|
| Patched `verusd` source | `/home/sarat/VerusCoin/src/init.cpp` (search `fOfflineMode`) |
| Patched daemon binary | `/home/sarat/VerusCoin/src/verusd` |
| CLI wrapper | `/home/sarat/VerusCoin/src/verus` (symlinked to `/usr/local/bin/verus`) |
| Daemon config | `/root/.komodo/vrsctest/vrsctest.conf` |
| Daemon data | `/root/.komodo/vrsctest/` |
| Daemon log | `/root/.komodo/vrsctest/debug.log` |
| Bet blockchain config | `/home/sarat/bet/poker/config/blockchain.ini` |
| Bet identity config | `/home/sarat/bet/poker/config/keys.ini` |
| Block-notify polling shim | `/home/sarat/bet/poker/run_blocknotify.sh` |
