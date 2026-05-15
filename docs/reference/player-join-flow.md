# Player Join Flow

This document describes how a player joins a poker table on the `bet` system, and the proposed change to replace hardcoded player IDs with a configured list in `dealer.ini`.

---

## 1. Today's flow (as implemented)

The flow has three independent on-chain actors: **player**, **cashier identity**, and **dealer**. Each step below is a real on-chain action; intermediate state is held on Verus identities (CMM keys), not in any tx payload.

### 1.1 Player broadcasts the payin

`vdxf.c:join_table` does two things, in order:

1. **Send currency to the cashier i-address** via `verus_sendcurrency_data(cashier_fqn, payin_amount, NULL)`.
   - The third argument (`data`) is **deliberately ignored** inside `verus_sendcurrency_data` (see `vdxf.c:632` — `(void)data; // Data parameter currently unused — join info stored on player identity instead`).
   - Established by commit `8e5eefa9` (Jan 1, 2026) — *"Simplify sendcurrency to not embed data (identity addresses don't support data field)"*.
   - **Therefore the payin tx is a plain currency transfer.** It carries no `verus_pid`, no `table_id`, no `dealer_id`. Just `(amount, currency, address)`.

2. **Update the player's own Verus identity** with `P_JOIN_REQUEST_KEY` containing the join metadata:
   ```json
   {
     "dealer_id":  "d1.sg777z.VRSCTEST@",
     "table_id":   "t1.sg777z.VRSCTEST@",
     "cashier_id": "cashier.sg777z.VRSCTEST@",
     "payin_tx":   "<txid from step 1>"
   }
   ```
   All identity references are fully-qualified Verus IDs. This is the canonical record of intent to join.

### 1.2 Dealer discovery loop

`dealer.c:handle_game_state` (state `G_TABLE_STARTED`) calls `poker_poll_players_for_joins(cashier_id, table_id, dealer_id, start_block)`.

This function polls **player identities** for `P_JOIN_REQUEST_KEY`; the cashier address is queried exactly once per tick to act as a "did the money land?" lookup table.

(Historical note: this function used to be called `poker_poll_cashier_for_joins`, which misrepresented the polling target. Renamed to `poker_poll_players_for_joins` since the cashier is only a verifier here.)

`poker_vdxf.c:poker_poll_players_for_joins` does:

1. **One-shot fetch of cashier UTXOs / txids since `start_block`:**
   ```c
   txids = get_address_txids_range(cashier_address, start_block, 0);
   ```
   This is just an index used for verification — *not* the source of join metadata.

2. **Iterate a hardcoded list of player FQNs:**
   ```c
   const char *known_players[] = {
       "p1.sg777z.VRSCTEST@", "p2.sg777z.VRSCTEST@", ...,
       "p9.sg777z.VRSCTEST@", NULL
   };
   for (int i = 0; known_players[i] != NULL; i++) {
       check_player_join_request(known_players[i], table_id, dealer_id, txids, start_block);
   }
   ```

3. **For each candidate player, read `P_JOIN_REQUEST_KEY` from that player's identity** (`get_cJSON_from_id_key(player_id, P_JOIN_REQUEST_KEY)`):
   - If the key is absent → no intent to join, skip.
   - If `dealer_id`/`table_id` don't match this dealer → not for me, skip.
   - If `payin_tx` is **not** in the cashier's txid list → reject (the player claims a payin that never landed).
   - If `payin_tx`'s confirm height is **before** `start_block` → stale request from a previous game, skip.
   - If the player is already in `t_player_info` → already joined, skip.
   - Otherwise → call `process_player_join` → `process_payin_tx_data` → write `T_PLAYER_INFO_KEY.<game_id>` on the table id.

### 1.3 What blocknotify / `process_block` (cashier side) currently does

`vdxf.c:process_block` is wired up via `bet newblock <hash>` from the `run_blocknotify.sh` polling shim. It:
1. Confirms it's running on the cashier (`id_cansignfor(cashier_fqn)`).
2. Lists UTXOs at the cashier address in the new block.
3. For each UTXO at the new block height, calls `chips_extract_tx_data_in_JSON(txid)` to try to extract embedded JSON, then feed it into `process_payin_tx_data`.

**This path is dead code for joins.** Step 3 always returns NULL because §1.1 step 1 doesn't embed any data. So `process_block` logs `tx_id::…` lines and then drops every tx at `if (!payin_tx_data) continue;`. The dealer's polling loop and the cashier's polling loop (§1.4) are the working payin processors in the current codebase. Removing `process_block` outright is a separate cleanup.

### 1.4 Cashier-side discovery loop

The cashier mirrors the dealer's polling pattern (§1.2) but with a different filter and a different action. This is what lets the cashier learn the active `table_id` and seed `g_start_block` without being told either on the CLI.

`blinder.c:cashier_game_init` runs an idle-poll loop:

1. **While idle** (`cashier_active == 0`), every 2s call `cashier_poll_players_for_joins()`.
2. That iterates the same `known_players[]` FQN list as the dealer (currently duplicated in `blinder.c` — TODO.md item 2 collapses both into config-driven).
3. For each player FQN, `cashier_check_payin_join()`:
   - Reads `P_JOIN_REQUEST_KEY` from the player identity (cumulative-latest, no height filter).
   - Filters: `req.cashier_id == bet_get_cashiers_id_fqn()` (vs. the dealer's `dealer_id`/`table_id` filter).
   - Verifies cryptographically: `chips_get_balance_on_address_from_tx(get_vdxf_id(cashier_fqn), req.payin_tx) > 0`. The cashier owns the address, so this is the strongest-possible payin verification — independent of `g_start_block`, which is the exact seam that resolves the chicken-and-egg start_block problem.
4. **On the first verified match:**
   - Read `T_GAME_ID_KEY` from `req.table_id` (cumulative on the table id, plain `getidentity`).
   - Read `T_TABLE_INFO_KEY.<game_id>` from `req.table_id`, extract `start_block`.
   - Seed `g_start_block`, store `cashier_table_id`, set `cashier_active = 1`.
5. **Once active**, the loop falls through to `handle_game_state_cashier(cashier_table_id)` — the existing state machine (deck shuffle, BV reveal, settlement) — unchanged.
6. **On `G_SETTLEMENT_COMPLETE`**, the lifecycle resets: `cashier_active = 0`, `cashier_table_id[0] = '\0'`, `g_start_block = 0`. The next iteration re-enters the idle-poll phase, ready for the next game's first payin.

**No on-chain writes from this discovery path.** It only reads. The dealer is still the actor that writes `T_PLAYER_INFO_KEY` on the table id; the cashier just learns the topology so it can do its existing job (deck shuffle / BV / settlement) without an `--table_id` CLI argument.

**Single-table per cashier in this iteration.** `cashier_table_id` is a single static; the cashier serves one game at a time, then resets. Multi-table support would require per-table `g_start_block` and is tracked separately.

---

## 2. Problem with the current discovery list

`known_players[] = {"p1.sg777z.VRSCTEST@", ..., "p9.sg777z.VRSCTEST@", NULL}` is:

- **Hardcoded in C source** (`poker_vdxf.c:361` for join discovery, also in `blinder.c` for cashier-side join discovery, and `vdxf.c:cashier_poll_disputes` for dispute discovery).
- **Identical for every deployment that targets the same parent** — to add `p10`, you patch C and rebuild; to target a different parent (e.g. production), you maintain a separate binary.
- **Probed even when no such identity exists** — every poll tick does up to N `getidentity` RPCs against possibly-nonexistent IDs.
- **Hidden** — operators have no visible knob for "which players is this dealer accepting?".

There is no startup-time check that the player IDs actually resolve on-chain. If `p4.sg777z.VRSCTEST@` was never created, the dealer silently no-ops on every tick.

---

## 3. Proposed change — `dealer.ini` declares the player set

Make the player list a deployment configuration item, and verify it on dealer startup.

> **Status note.** Since the FQN hard-cutover (commit `8e2f1907`),
> `known_players[]` is hardcoded with full FQNs rather than short
> names; the rest of this proposal (config-driven discovery + startup
> validation) is still deferred — see `TODO.md` item 2.

### 3.1 `dealer.ini` schema extension

Add a `players` section listing the FQNs the dealer is willing to seat:

```ini
[table]
max_players = 2
big_blind   = 0.001
min_stake   = 20
max_stake   = 100
table_id    = t1.sg777z.VRSCTEST@

[verus]
dealer_id  = d1.sg777z.VRSCTEST@
cashier_id = cashier.sg777z.VRSCTEST@

[players]
# Comma-separated list of fully-qualified Verus IDs
ids = p1.sg777z.VRSCTEST@, p2.sg777z.VRSCTEST@
```

Every identity field is a full FQN — the existing INI parsers
already enforce this for `dealer_id` / `table_id` / `cashier_id`.

### 3.2 Dealer startup behaviour

When the dealer starts:

1. Parse `[players].ids` from `dealer.ini`. Trim whitespace. Reject empty list.
2. For each FQN `s`:
   a. Validate `s` contains `@` (consistent with the existing INI checks).
   b. Call `is_id_exists(s)` (already used elsewhere in the codebase).
   c. If any ID does not resolve on-chain → log the missing ID and **abort dealer startup with a non-zero exit code**. Do not enter `handle_game_state`.
3. If all IDs resolve → log the verified list (`Dealer accepting joins from: p1.sg777z.VRSCTEST@, p2.sg777z.VRSCTEST@`) and proceed to normal init.

This is a fail-fast precondition: the operator gets immediate feedback if a player identity is missing or misspelled, instead of a silently-no-op poll loop.

### 3.3 Replace the hardcoded `known_players[]`

Both call sites switch from a C string-array to a runtime list owned by config:

- `poker_poll_players_for_joins(...)` — iterate over `dealer_config.player_ids` instead of `known_players[]`.
- `cashier_poll_disputes(...)` — same source. Cashier already takes `known_players[]` as an arg, so the cashier just needs the same config injected (likely via `cashier.ini` mirroring the same `[players] ids = ...` field, since the cashier needs to know who can dispute).

### 3.4 Behavioural guarantees after this change

- **No more brute-force probing of `p3..p9` when only `p1, p2` are configured.** Each poll tick costs `O(num_configured_players)` `getidentity` calls instead of `O(9)`.
- **Misconfigured deployments don't start** — no mysterious "table never fills" because a player ID was off by one character.
- **Operators have a single source of truth** for the player set: `dealer.ini` (and, mirrored, `cashier.ini`).
- **No on-chain state changes** — this is purely a discovery / startup-validation change. The join protocol itself (player writes `P_JOIN_REQUEST_KEY`, dealer polls + verifies via cashier txid list, dealer writes `T_PLAYER_INFO_KEY`) is unchanged.

### 3.5 Things this proposal does **not** change

- Payin txs are still plain `sendcurrency` calls with no embedded data.
- Join intent still lives in `P_JOIN_REQUEST_KEY` on the player identity.
- `process_block` (cashier side) is still effectively dead for joins — that's a separate question (delete it, or repurpose it to mirror the dealer's polling logic) and out of scope for this doc.

---

## 4. Open questions

1. **Should the cashier's dispute polling list be derived from the same `[players]` block in `dealer.ini`**, or kept independently in `cashier.ini`? Independent is more decoupled but creates two sources of truth that must stay in sync.
2. **Dynamic player addition during a running game** — should adding a new FQN to `dealer.ini` and SIGHUP'ing the dealer be supported, or do we require a full restart between games? Restart-only is simpler and matches the current `--reset` lifecycle.
3. **Player aggregator identity** — is there value in maintaining an on-chain aggregator (à la the existing `dealers`/`cashiers` aggregator IDs) listing approved players, so dealers and cashiers don't need duplicate config? Probably yes long-term, but out of scope for this change.
