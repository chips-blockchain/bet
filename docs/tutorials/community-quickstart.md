# Community Testing Guide

This is the short path from "fresh clone" to "two players finishing a
hand of poker on your own Verus regtest". For the full operator
playbook (including the `verusd` patch, identity registration, and
reset/recovery procedures), see
[`VRSCTEST_LOCAL_SETUP.md`](cli-auto-vrsctest.md).

---

## 1. What you'll be running

Four `bet` processes on the same machine, talking to each other only
through Verus identity contentmultimap (CMM) updates on a local
`VRSCTEST` regtest:

| Role     | Identity (dev)        | Purpose                                                      |
|----------|-----------------------|--------------------------------------------------------------|
| Dealer   | `d1.<parent>@`        | Drives the per-hand state machine.                           |
| Cashier  | `cashier.<parent>@`   | Custodies payins and disburses payouts.                      |
| Player 1 | `p1.<parent>@`        | Places bets, signs deck-shuffle steps.                       |
| Player 2 | `p2.<parent>@`        | Same.                                                        |

A single `bet` binary plays all roles, selected by the first argument
to `./bin/bet start`.

---

## 2. Prerequisites

* Linux (Ubuntu 20.04 or 22.04 recommended; macOS works too).
* A built Verus daemon and CLI, with the small `-offline` patch
  for local-only operation. The exact patch and where to put the
  binaries are in [`VRSCTEST_LOCAL_SETUP.md § 2`](cli-auto-vrsctest.md).
* The Verus daemon running with `-chain=VRSCTEST -offline -daemon`.
* `verus` on `PATH` (typically a symlink into `/usr/local/bin/`).
* Registered identities for the parent, dealer, cashier, and each
  player on your dev chain. The registration flow is in
  [`docs/explanation/identity-tree.md`](../explanation/identity-tree.md);
  the dev regtest identities listed in
  [`VRSCTEST_LOCAL_SETUP.md § 6`](cli-auto-vrsctest.md) are an
  example you can mimic.

Sanity check before continuing:

```bash
verus -chain=VRSCTEST getblockcount        # returns a number
verus -chain=VRSCTEST listidentities | head # lists your registered IDs
```

---

## 3. Build

```bash
git clone --recurse-submodules <fork-url> bet
cd bet
make -j$(nproc)
```

Produces `poker/bin/bet`. For a full list of system packages, see
[`docs/guides/build-from-source.md`](../guides/build-from-source.md).

---

## 4. Configuration

All four nodes share the same `poker/config/` directory:

* `blockchain.ini` — `blockchain_cli = "verus -chain=VRSCTEST"`,
  `currency = VRSCTEST`. See
  [`dealer_configuration.md`](../reference/dealer-config.md).
* `keys.ini` — aggregator identity FQNs (`cashier_id`, `dealers_id`).
  The deployment parent (`sg777z.VRSCTEST@` in the dev layout) is
  implicit in every FQN.
* `.rpccredentials` — `rpcuser=` / `rpcpassword=` matching
  `~/.komodo/vrsctest/vrsctest.conf`.
* `dealer.ini`, `cashier.ini`, `p1.ini`, `p2.ini` — per-node knobs
  (table parameters for the dealer, identity FQNs for the players,
  GUI port).

The shipped examples in `poker/config/` are wired for the dev
layout; replace the identity names if you registered your own.

---

## 5. Bring up the stack (CLI auto)

In four terminals (or four tmux panes), from `poker/`:

```bash
# 1. Dealer (the --reset is part of the canonical bring-up; it
#    clears prior in-memory state for a clean hand)
./bin/bet start dealer --config config/dealer.ini --cli --reset

# 2. Cashier (no table arg — it polls player identities and learns
#    which table is active from the first join request)
./bin/bet start cashier --config config/cashier.ini --cli

# 3. Player 1 (--auto = automated betting, useful for smoke testing)
./bin/bet start player --config config/p1.ini --auto

# 4. Player 2
./bin/bet start player --config config/p2.ini --auto
```

Wait a few seconds between starts so each node has a chance to
publish its initial CMM updates.

A successful hand goes through the 14 game states documented in
[`docs/reference/game-states.md`](../reference/game-states.md);
expect roughly one minute end-to-end on the regtest.

### Betting modes (per player)

| Flag      | What it does                                                                       |
|-----------|------------------------------------------------------------------------------------|
| `--auto`  | Bot betting (call/check based on simple rules). Default for unattended testing.    |
| `--cli`   | Prompts the operator on the terminal for each action.                              |
| `--gui`   | Opens a WebSocket on `verus:ws_port` for the GUI front-end. Default for a player. |

---

## 6. Watching what happens

### Logs

Each node writes `/root/.bet/logs/<role>.log`. Tail them with:

```bash
tail -F /root/.bet/logs/dealer.log
tail -F /root/.bet/logs/cashier.log
tail -F /root/.bet/logs/p1.log
tail -F /root/.bet/logs/p2.log
```

### On-chain state

The full hand history is on the table identity's CMM. Replay it from
a known starting block with:

```bash
verus -chain=VRSCTEST getidentitycontent 't1.<parent>@' <from_block> -1
```

`bet` ships CLI shortcuts for this — see
[`cli-print.md`](../reference/cli-print.md):

```bash
./bin/bet print_id t1
./bin/bet print_keys t1 <from_block>
```

### Wallet balances

```bash
# Cashier (during the hand, it temporarily holds both payins)
verus -chain=VRSCTEST getaddressbalance '{"addresses":["<cashier-i-addr>"]}'

# Players (after payout, the winner's identity holds the pot)
./bin/bet balance
```

---

## 7. Aborting and recovering

To stop the running nodes cleanly, send `Ctrl-C` to each in
reverse order (players → cashier → dealer). Do **not** kill the
`verusd` daemon — it can stay up across many test runs.

If a node crashes mid-hand, the rejoin paths described in
[`player-rejoin.md`](../explanation/player-rejoin.md) let it
re-enter the hand by re-reading the chain state. Dealer rejoin uses
the local SQLite (`pangea.db`) for deck keys; player rejoin reads
its own `P_*` CMM entries.

A full reset (fresh chain, lost identities) is documented in
[`VRSCTEST_LOCAL_SETUP.md § 4`](cli-auto-vrsctest.md) under
"Reset chain (nuclear)" — only use it if you really want to start
from scratch.

---

## 8. Reporting issues

When filing an issue, the most useful attachments are:

1. Output of `./bin/bet --version` from the build under test.
2. The relevant log files from `/root/.bet/logs/`.
3. The block range the hand happened in:
   `verus -chain=VRSCTEST getblockcount` before and after, and the
   table identity FQN. Anyone with the chain can replay the hand
   from `getidentitycontent`.
4. A sentence saying which game state (`G_*`) the hand was in when
   the bug occurred, if you can find it in `dealer.log`.

The list of currently-known deferred items is in
[`TODO.md`](../TODO.md).

---

## 9. Differences from the legacy CHIPS guide

For readers familiar with the previous CHIPS-PBaaS-era guide:

* There is **no Lightning Network** in the current build. `bet`
  does not start `lightningd`, doesn't read `lightning-cli`, and
  doesn't expose any LN-related GUI methods.
* There is **no `chipsd` daemon**. The chain is plain Verus
  (`VRSCTEST` regtest for dev).
* `payin` is now a plain `sendcurrency` from the player's wallet to
  `cashier.<parent>@`. There is no embedded data part on the
  transaction; the cashier correlates payins to join intents by
  reading the player's CMM.
* Player and dealer per-turn timeouts are 120 seconds plus 12 blocks
  (not the 60 s / 6 blocks of the legacy guide). On auto-timeout
  the dealer folds the silent player and continues.
* The deck is 52 cards (not the 14-card 2-player-optimized deck of
  earlier prototypes). Hand evaluation uses the full 7-card poker
  ranker.
