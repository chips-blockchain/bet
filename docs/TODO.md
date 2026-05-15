# Deferred work

Items that are designed/discussed but intentionally **not** part of the current iteration. The current code paths these would replace are working and we don't want to disturb them while we stabilise the payin / join flow on a fresh chain.

---

## Backlog

### 2. Dealer reads player IDs from `dealer.ini`

**Status:** deferred. Hardcoded `known_players[] = {"p1.sg777z.VRSCTEST@" ... "p9.sg777z.VRSCTEST@", NULL}` is duplicated in three places and stays as the discovery list for now:
- `poker/src/poker_vdxf.c:poker_poll_players_for_joins` (dealer)
- `poker/src/blinder.c:known_players` (cashier — added with the cashier-side join discovery in `PLAYER_JOIN_FLOW.md` §1.4)
- `poker/src/vdxf.c:cashier_poll_disputes` (cashier dispute polling, currently dormant — see item 3 below)

Since commit `8e2f1907` the entries are full FQNs rather than short names, so the array is tied to a specific parent at compile time (e.g. a VRSCTEST binary and a production binary will diverge on this constant).

**What's already documented:** full design is in [`PLAYER_JOIN_FLOW.md`](reference/player-join-flow.md) §3 — `[players] ids = ...` block in `dealer.ini` (FQNs), `is_id_exists` validation at startup, fail-fast on missing IDs, replacement of both `known_players[]` call sites.

**Why deferred:** the hardcoded list is functionally correct (just inefficient — up to 9 `getidentity` RPCs per 2-second poll tick instead of 2). We want to land payin / join correctness first on a clean chain before touching the discovery layer.

**Touched when picked up:**
- `poker/config/dealer.ini` (new `[players]` section)
- `poker/include/bet.h`, `poker/src/bet.c` (globals: `dealer_player_ids[][]`, `dealer_player_count`)
- `poker/include/err.h`, `poker/src/err.c` (`ERR_NO_PLAYERS_CONFIGURED`, `ERR_PLAYER_ID_NOT_FOUND`)
- `poker/src/config.c` (`bet_parse_dealer_config_ini_file` — parse `players:ids`)
- `poker/src/dealer.c` (`dealer_init`, `dealer_init_with_reset` — startup `is_id_exists` loop)
- `poker/src/poker_vdxf.c` (`poker_poll_players_for_joins` — drop `known_players[]`)
- `poker/src/blinder.c` (`cashier_poll_players_for_joins` — drop `known_players[]`)
- `poker/src/vdxf.c` (`cashier_poll_disputes` — drop `known_players[]`, takes config-driven list once dispute polling is wired up)

---

### 3. Graceful dispute resolution

**Status:** deferred. The functions exist in `poker/src/vdxf.c` (`cashier_poll_disputes`, `cashier_resolve_dispute`) and the on-chain key layout is defined (`P_DISPUTE_REQUEST_KEY` on the player ID, `C_DISPUTE_RESULT_KEY` on the cashier ID). The cashier's main loop (`handle_game_state_cashier` in `blinder.c`) **does not call `cashier_poll_disputes`** today — the path is dormant.

**What needs to be answered before we wire it up:**
1. **When is it safe for the cashier to resolve a dispute?** A dispute can only be resolved when no game referencing the disputed `payin_tx` is active. Today there is no `is_game_active(game_id)` check — `cashier_resolve_dispute` would happily process a dispute against a payin in a game still in progress.
2. **Single-threaded vs. dedicated worker.** The cashier today is single-threaded — `handle_game_state_cashier` runs the game loop on the main thread. Dispute polling on the same thread blocks the game loop; on a separate thread it has to coordinate with the game-state machine for the "is the game active?" guard. Pick one explicitly.
3. **Refund correctness.** `cashier_resolve_dispute` writes `C_DISPUTE_RESULT_KEY` and (presumably) issues a refund `sendcurrency`. Need to confirm idempotency: re-running on the same dispute must not double-refund.
4. **Discovery list.** Same `known_players[]` problem as item 2 — once we move to config-driven players, dispute polling iterates the same list.

**Touched when picked up:**
- `poker/src/blinder.c` (`handle_game_state_cashier` — invoke `cashier_poll_disputes` at a defined cadence/state)
- `poker/src/vdxf.c` (`cashier_resolve_dispute` — add active-game guard, audit refund idempotency)
- Possibly a new `poker/src/game.c` helper `is_game_active(game_id)` reading `T_GAME_STATE_KEY` / `T_GAME_RESULT_KEY` from the table id.

---

### 4. Persist `cashier_r` for crash recovery across the shuffle / BV-reveal window

**Status:** deferred. The current in-process idempotency gate (`blinder.c` `G_DECK_SHUFFLING_D`, added with item 1.1's direct-read migration) prevents the same cashier process from re-entering `cashier_init_deck` and regenerating `b_deck_info.cashier_r` while the table state is still propagating — this fixes the hot path. It does **not** survive a cashier process crash mid-game.

**Failure mode the gate doesn't cover:**

1. Cashier shuffles, writes `C_B_P*_DECK_KEY.<gid>` on its own id, signals `G_DECK_SHUFFLING_B`. `b_deck_info.cashier_r` lives only in process memory.
2. Cashier crashes (any reason — OOM, SIGSEGV, host reboot).
3. Cashier restarts. The on-chain deck is still there → idempotency gate skips the shuffle → `b_deck_info.cashier_r` is **never reinitialized**.
4. At reveal time `reveal_bv` reads `b_deck_info.cashier_r[player_id][card_id].priv` — zeroes / garbage — publishes garbage BV, decode_card returns -1 forever.

**What needs to happen:**

Persist `cashier_r` keyed by `<game_id>` so a restarted cashier can recover the same blinding values it used when it wrote the on-chain deck. Two storage candidates:

- **Cashier id (on-chain, encrypted to cashier's own key):** new `C_CASHIER_R_KEY.<game_id>` carrying the per-player `cashier_r[i][0..CARDS_MAXCARDS-1].priv` bytes, encrypted with the cashier's identity private key (or a derived symmetric key) so other parties can't read it. Pro: identity-rooted, naturally garbage-collected when the game completes via the same lifecycle reset. Con: encryption infra needed; on-chain bytes per game.
- **Local DB (`/root/.bet/db/pangea.db`):** new `cashier_blinding_values` table keyed by `(cashier_id, game_id)`. Pro: no on-chain bytes, simpler — plaintext is fine since the file is local to the cashier host. Con: lost if the disk is wiped / a new cashier host comes up; tied to a specific machine.

**Recommendation when picked up:** start with the local DB option (simpler, sufficient for crash recovery on the same host) and only escalate to encrypted-on-chain if multi-host cashier failover becomes a requirement.

**Recovery flow that needs writing:**

1. On cashier startup, load `cashier_r` from store keyed by `<game_id>` of the active table (if any).
2. In `cashier_init_deck`, if `cashier_r` for `<game_id>` is already loaded, skip the `gen_deck` calls (don't overwrite).
3. On `G_SETTLEMENT_COMPLETE` lifecycle reset, delete the entry.

**Touched when picked up:**
- `poker/src/blinder.c` (`cashier_init_deck` — load-or-generate; `G_SETTLEMENT_COMPLETE` lifecycle reset — delete)
- `poker/src/storage.c` + new schema migration (if local-DB path is chosen)
- `poker/include/vdxf.h` + `poker/src/vdxf.c` (if on-chain path is chosen — new `C_CASHIER_R_KEY` macro + encryption helpers)
- `docs/TODO.md` — close this item

---

### 5. Side-pot generalization for hand evaluation (all-in scenarios)

**Status:** deferred. The new `dealer_evaluate_showdown` (in `poker/src/dealer.c`) splits `vars->pot` evenly among tied top-score winners — which is correct for the no-all-in case but wrong when one or more players go all-in for less than the full pot.

The legacy code in `poker/src/host.c` (`det_dcv_pot_split`) implements the proper side-pot algorithm: walk players in order of `funds_spent`, peel off sub-pots, distribute each sub-pot only among winners eligible for that level. It depends on a `card_values[CARDS_MAXPLAYERS][hand_size]` global that is **no longer populated** in the Verus CMM flow (the legacy nanomsg `bet_receive_card` was the writer) — so we cannot just call it as-is.

**What needs to happen when picked up:**
1. Replace the `card_values[][]` global with values sourced from the same on-chain reads that `dealer_evaluate_showdown` already does (hole cards from `P_HOLECARDS_REVEAL_KEY` on each player id, board from `T_BOARD_CARDS_KEY` on table id) — populate a local 2-D buffer with the same shape as `card_values[][]`.
2. Adapt `det_dcv_pot_split` to take the buffer as a parameter (drop the global) and `vars` for `funds_spent`/`bet_actions`/`pot`.
3. Drive the loop from `vars->bet_actions[i][round] == allin` markers (already set today by `game.c::verus_handle_round_betting` when a player goes all-in).
4. Replace the simple even-split block at the bottom of `dealer_evaluate_showdown` with a call to the adapted side-pot logic. Keep the simple path as a fast fallback for `no all-ins this hand`.

**Touched when picked up:**
- `poker/src/dealer.c` (`dealer_evaluate_showdown` — switch from even-split to side-pot)
- `poker/src/host.c` (adapt `det_dcv_pot_split` signature; drop `card_values[][]` global dependency)
- `poker/include/host.h` (export the adapted signature, if needed)

---

## Out of scope here

- Replacing or deleting `process_block` (cashier-side blocknotify path). It's currently dead code for joins (see `PLAYER_JOIN_FLOW.md` §1.3) but the decision of whether to delete vs. repurpose stays open until joins are stable.
