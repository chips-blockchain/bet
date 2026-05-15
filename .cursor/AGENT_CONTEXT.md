# Agent Context — Pangea-Bet Testing

## CRITICAL: Read This First

This is an interactive session. Rules are non-negotiable:
1. **Never touch the blockchain daemon** (`verusd` — user manages it in tmux `chipsd`)
2. **No system commands** — only project-relevant commands
3. **Never create or modify Verus IDs** — no `updateidentity`, `registernamecommitment`, `reset_id`
4. **Plan first, wait for approval** — present plan, get explicit go-ahead, then act one step at a time
5. **One phase at a time** — test → find issue → identify root cause → report to user → wait for fix approval → fix → retest
6. **Never fix everything at once** — each fix requires user permission

---

## Blockchain Setup

- **Daemon:** `verusd` at `/usr/local/bin/verusd` — Verus 1.2.9-5
- **Chain:** CHIPS PBaaS chain under Verus VRSC
- **Start command (user runs this):** `verusd -chain=CHIPS -daemon`
- **Data dir:** `/root/.verus/pbaas/f315367528394674d45277e369629605a1c3ce9f/`
- **RPC port:** 22778, P2P port: 22777
- **CLI:** `verus -chain=chips <command>` (alias `_vc` available on this machine)
- **Credentials:** `/root/bet/poker/.rpccredentials` (user=user1172159772)
- **Block time:** ~10 seconds

---

## Verus Identities (All exist under `sg777z.chips.vrsc@`)

| Short ID | Full ID | Role |
|----------|---------|------|
| `d1` | `d1.sg777z.chips.vrsc@` | Dealer node |
| `t1` | `t1.sg777z.chips.vrsc@` | Table (game state lives here) |
| `p1` | `p1.sg777z.chips.vrsc@` | Player 1 |
| `p2` | `p2.sg777z.chips.vrsc@` | Player 2 |
| `cashier` | `cashier.sg777z.chips.vrsc@` | Cashier |

**These are live on-chain with real CHIPS. Never modify.**

---

## Backend Repo: `/root/bet`

### Binary
```
/root/bet/poker/bin/bet        # Main binary (compiled, ready)
```

### Config files (`/root/bet/poker/config/`)
- `dealer.ini` — max_players=2, big_blind=0.001, min_stake=20 BB, table_id=t1, dealer_id=d1, cashier_id=cashier
- `p1.ini` — player_id=p1, dealer_id=d1, table_id=t1, ws_port=9001
- `p2.ini` — player_id=p2, dealer_id=d1, table_id=t1, ws_port=9002
- `keys.ini` — parent_id=sg777z.chips.vrsc@ (VDXF key names are compile-time macros in `poker/include/vdxf.h`)
- `.rpccredentials` — RPC credentials for CHIPS daemon

### Node start commands
```bash
cd /root/bet/poker

# Dealer (fresh game):
./bin/bet start dealer -c config/dealer.ini --reset

# Dealer (rejoin):
./bin/bet start dealer -c config/dealer.ini

# Cashier:
./bin/bet start cashier -t t1

# Player 1 — CLI (interactive betting, agent types commands):
./bin/bet start player -c config/p1.ini --cli

# Player 2 — AUTO (auto check/call, no input needed):
./bin/bet start player -c config/p2.ini --auto

# Player — GUI mode (WebSocket on port 9001):
./bin/bet start player -c config/p1.ini --gui
```

### Betting modes
| Flag | Behaviour |
|------|-----------|
| `--auto` | Auto check/call — no human input |
| `--cli` | Reads from stdin — agent types: `fold`/`f`, `check`/`x`, `call`/`c`, `raise`/`r`, `bet`/`b`, `allin`/`a` |
| `--gui` | WebSocket port 9001 — waits for GUI or simulator commands |

### Game state machine (in order, every step = blockchain write ~10s)
```
G_ZEROIZED_STATE      → table not initialized
G_TABLE_STARTED       → dealer reset, new game_id on t1
G_PLAYERS_JOINED      → both players paid in and joined
G_DECK_SHUFFLING_P    → players uploaded deck keys
G_DECK_SHUFFLING_D    → dealer shuffled all decks
G_DECK_SHUFFLING_B    → cashier (BVV) did final shuffle
G_REVEAL_CARD         → a card being dealt
G_ROUND_BETTING       → betting round (pre-flop/flop/turn/river × 4 rounds)
G_SHOWDOWN            → winner determined
G_SETTLEMENT_PENDING  → cashier processing payouts
G_SETTLEMENT_COMPLETE → game finished
```

### Startup order (mandatory)
1. Dealer first (`--reset` for fresh game)
2. Wait ~30s for blockchain
3. Player 1
4. Wait ~30s
5. Player 2
6. Cashier (can start anytime)

### Key source files
| File | What it does |
|------|-------------|
| `src/bet.c` | Entry point, command parsing |
| `src/dealer.c` | Dealer state machine, shuffle, settlement trigger |
| `src/player.c` | Player join, deck init, CLI/GUI betting handler |
| `src/cashier.c` | Payin processing, BVV shuffle, payout |
| `src/game.c` | Game state strings, state transitions, card dealing |
| `src/states.c` | Betting state machine (turns, rounds) |
| `src/vdxf.c` | All blockchain read/write (`updateidentity`, `getidentitycontent`) |
| `src/gui.c` | GUI message builders (Backend → GUI) |
| `src/client.c` | WebSocket server (GUI → Backend handlers) |
| `src/commands.c` | RPC wrappers (`verus -chain=chips ...`) |
| `include/common.h` | All constants |

### Constants
- `CARDS_MAXCARDS=14` (2-player optimized deck)
- `BET_TURN_TIMEOUT_SECS=60`, `BET_TURN_TIMEOUT_BLOCKS=6`
- `default_bb_in_chips=0.02`, `default_sb_in_chips=0.01`
- `default_min_stake=0.5` CHIPS payin

### Log files
`/root/bet/poker/logs/` — one file per node (by ID)

---

## GUI Repo: `/root/pangea-poker`

### Stack
- React 16 + TypeScript, Parcel bundler, Electron (optional)
- WebSocket: `react-use-websocket` library
- State: React Context + custom store/actions

### Run GUI
```bash
cd /root/pangea-poker
npm start            # Opens at http://localhost:1234
```

### Default IP config
`/root/pangea-poker/src/config/development.json` — default IP: `159.69.23.31`
Change this to `127.0.0.1` for local testing.

### WebSocket ports (backend-side)
| Node | Port |
|------|------|
| Dealer | 9000 |
| Player | 9001 |
| Cashier | 9002 |

### Key GUI source files
| File | What it does |
|------|-------------|
| `src/components/Game/onMessage.ts` | **All incoming WebSocket message handlers** |
| `src/components/Game/WebSocket.ts` | WebSocket connection management |
| `src/components/FindTableButton/FindTableButton.tsx` | "Find Table" button → sends `table_info` request |
| `src/config/development.json` | Default backend IPs |
| `src/store/actions.ts` | All state actions dispatched from messages |

### GUI message flow (GUI → Backend, triggers)
1. GUI connects to `ws://host:9001`
2. GUI receives `backend_status` → shows "Find Table" button
3. User clicks "Find Table" → GUI sends `{"method": "table_info"}`
4. Backend responds with `table_info` message → GUI renders seat map
5. User clicks empty seat → GUI sends `{"method": "join_table", "seat": N}`
6. Backend responds with `join_ack` → GUI shows "Joining..."
7. Backend sends `player_init_state` updates as joining progresses
8. Game starts → GUI receives `deal`, `betting`, `finalInfo` messages

### GUI message handlers (what `onMessage.ts` handles)
| Message from backend | GUI action |
|---------------------|------------|
| `backend_status` | Updates backendStatus, shows Find Table button |
| `table_info` | Populates seat map, balance, table config |
| `player_init_state` | Shows join progress (states 1–7) |
| `join_ack` | Confirms join approved |
| `blindsInfo` | Sets blind amounts |
| `dealer` | Sets dealer button position |
| `seats` | Updates seat occupancy |
| `deal` | Renders hole cards / board cards |
| `betting` (round_betting) | Shows betting controls to active player |
| `betting` (check/call/raise/fold/allin) | Shows other player's action |
| `finalInfo` | Runs showdown animation, shows winner |
| `warning` | Shows error notice |
| `error` | Shows error notice |

---

## Known Gaps Between Backend and GUI

These are documented gaps. **Do not fix without user approval.**

### Gap 1 — CRITICAL: Backend sends `walletInfo`, GUI expects `table_info`
- **GUI** sends `{"method": "table_info"}` (from Find Table button)
- **GUI** handles `case "table_info"` with `occupied_seats`, `balance`, `table_id`, etc.
- **Backend** (`gui.c`) builds and sends `walletInfo` (NOT `table_info`)
- `walletInfo` falls to `default` in GUI → unknown method warning, table never loads
- **Impact:** GUI cannot display the table or let the player join

### Gap 2 — `table_info` missing `occupied_seats` field
- GUI handler expects `occupied_seats: [{seat, player_id, stack}]` to show who is sitting
- Backend's `gui_build_wallet_info()` has no such field
- **Impact:** Even if method name is fixed, seat occupancy won't show

### Gap 3 — `finalInfo` crash if `showInfo` is missing
- GUI does `message.showInfo.boardCardInfo` — crashes with TypeError if `showInfo` is null
- Backend only populates `showInfo` when `board_cards` is non-null
- **Impact:** Showdown may crash the GUI

### Gap 4 — `player_init_state` message format needs verification
- GUI expects: `{"method":"player_init_state","state":5,"state_name":"...","player_id":"p1","payin_amount":0.5}`
- Backend sends `send_init_state_to_gui()` — need to verify exact fields match

### Gap 5 — Card string format unverified
- Backend `gui.c`: ranks=`23456789TJQKA`, suits=`cdhs` → e.g. `"Ah"`, `"Tc"`
- GUI uses `lowerCaseLastLetter()` on cards in `finalInfo` — expects e.g. `"Ah"` → `"Ah"` ✓
- Needs live test to confirm no rendering issues

### Gap 6 — `finalInfo` hardcoded for 2 players
- `gui_build_final_info()` iterates `for p < 2` — breaks for 3+ players

### Gap 7 — No reconnect/resync
- No mechanism to re-send current game state if GUI reconnects mid-game

---

## Tmux Session Convention

| Session | Node | Who manages |
|---------|------|-------------|
| `chipsd` | CHIPS blockchain daemon | **User only — never touch** |
| `dealer` | Dealer node | Agent (with approval) |
| `cashier` | Cashier node | Agent (with approval) |
| `p1` | Player 1 | Agent (with approval) |
| `p2` | Player 2 | Agent (with approval) |

---

## GUI Simulator (for GUI mode testing without browser)

```bash
cd /root/bet/tools
node gui_simulator.js <port> <mode>
# mode: check | call | random | fold
# Example: node gui_simulator.js 9001 call
```

Connects as a fake GUI, auto-responds to betting prompts. Use for GUI mode testing without launching pangea-poker.

---

## Phase-by-Phase Testing Plan

**Rule: Complete one phase, document result, report to user, get approval before next.**

---

### PHASE 0 — Prerequisites
**Goal:** Confirm system is ready before touching any node.

Steps:
1. Verify blockchain daemon is synced (`verus -chain=chips getblockcount`)
2. Verify all 5 IDs exist and are readable on-chain
3. Check wallet balance

**Pass criteria:** All 5 IDs found, block height > 0, daemon responding.

---

### PHASE 1 — Dealer Initialization
**Goal:** Dealer resets table, writes fresh game_id to chain.

Command: `./bin/bet start dealer -c config/dealer.ini --reset`

Watch for in logs:
- `New game_id: <hash>`
- `New start_block: <height>`
- `Table CMM updated successfully`
- `Waiting for players to join...`

**Pass criteria:** Game state on `t1` = `G_TABLE_STARTED`, new game_id written.
**Fail → stop, report, wait.**

---

### PHASE 2 — Cashier Start
**Goal:** Cashier comes up and polls for joins.

Command: `./bin/bet start cashier -t t1`

Watch for: cashier running, no immediate errors.

**Pass criteria:** Cashier running, no crash on startup.

---

### PHASE 3 — Player 1 Joins
**Goal:** Player 1 connects, finds table, payin confirmed, joins.

Command: `./bin/bet start player -c config/p1.ini --cli`

Watch for in logs:
- `Table found`
- `Table Joined`
- `Player ID: 0`
- `player_init_deck` started

**Pass criteria:** p1 joined, game state = `G_TABLE_STARTED` still (waiting for p2).
**Fail → stop, report, wait.**

---

### PHASE 4 — Player 2 Joins
**Goal:** Player 2 joins, table becomes full, game moves forward.

Command: `./bin/bet start player -c config/p2.ini --auto`

Watch for:
- `Player ID: 1`
- Dealer log: `G_PLAYERS_JOINED`

**Pass criteria:** Both players joined, game state = `G_PLAYERS_JOINED`.
**Fail → stop, report, wait.**

---

### PHASE 5 — Deck Shuffling (3 phases)
**Goal:** All 3 shuffle phases complete.

Watch for (in order):
- Players: `G_DECK_SHUFFLING_P` — players upload deck keys
- Dealer: `G_DECK_SHUFFLING_D` — dealer shuffles
- Cashier: `G_DECK_SHUFFLING_B` — cashier BVV shuffle done

**Pass criteria:** Game state = `G_DECK_SHUFFLING_B` complete, `Its time for game`.
**Fail → stop, report, wait.**

---

### PHASE 6 — Card Dealing (Hole Cards)
**Goal:** Both players receive hole cards.

Watch for:
- `G_REVEAL_CARD` repeated for each card
- Player logs: `Card 1: <name>`, `Card 2: <name>`

**Pass criteria:** Both players have 2 hole cards decoded.

---

### PHASE 7 — Pre-flop Betting (CLI)
**Goal:** Player 1 acts via CLI, Player 2 auto-responds.

Player 1 CLI prompt appears, agent types action (e.g. `call`).
Player 2 auto-calls.

Watch for:
- `G_ROUND_BETTING`
- Both players act
- Round advances to flop

**Pass criteria:** Pre-flop round complete, pot updated.

---

### PHASE 8 — Flop, Turn, River (Repeat)
**Goal:** Board cards dealt, 3 more betting rounds complete.

Repeat Phase 6+7 pattern for flop (3 cards), turn (1 card), river (1 card).

---

### PHASE 9 — Showdown & Settlement
**Goal:** Winner determined, cashier pays out.

Watch for:
- `G_SHOWDOWN`
- `G_SETTLEMENT_PENDING`
- `G_SETTLEMENT_COMPLETE`
- Cashier: payout transaction sent

**Pass criteria:** Game completes, funds moved correctly.

---

### PHASE 10 — GUI Testing (after CLI test complete)
**Goal:** Replay the game flow with Player 1 in `--gui` mode, connected to pangea-poker.

Setup:
- Player 2: `--auto`
- Player 1: `--gui` (port 9001)
- GUI: `cd /root/pangea-poker && npm start` → browser at `http://localhost:1234`

Validate each phase from CLI test against GUI:
- Gap 1 (table_info) will likely fail first — document it, report, fix with approval
- Work through gaps one at a time

---

## Current Status

| Item | Status |
|------|--------|
| Blockchain daemon | User manages in `chipsd` tmux — DO NOT TOUCH |
| All Verus IDs | Confirmed exist under `sg777z.chips.vrsc@` |
| CLI testing | NOT started — waiting for chain to be ready |
| GUI gap analysis | Documented above — 7 gaps identified |
| Any fixes | NONE — waiting for user approval |
