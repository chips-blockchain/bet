# Identity keys reference

`bet` stores all between-actor game state on Verus identities. Each
piece of state lives at a specific key on a specific identity, written
by one specific actor and read by the others. This document is the
reference for every key the current build uses — what it's called,
where it lives, what it carries, who writes it, and when.

For the architectural rationale behind the per-identity layout (why
each actor writes only to its own identity, why some keys exist twice
in cashier-side and table-side variants, what the 5872-byte size
limit means), see [`verus-overview.md`](../explanation/verus-overview.md). For how
each identity is created in the first place see
[`id_creation_process.md`](../explanation/identity-tree.md). For the game
state machine that drives these writes see
[`game_state.md`](game-states.md).

The macro names below are defined in `poker/include/vdxf.h`. The
literal namespace string each macro expands to is given in
parentheses; it is hashed to a 32-byte VDXF id at runtime
(`get_vdxf_id` in `vdxf.c`) before being placed on-chain.

---

## Conventions

**Namespace prefix.** Every key uses `chips.vrsc::poker.` as
its readable prefix, defined once as the `VDXF_POKER_KEYS_PREFIX`
macro in `vdxf.h` and composed by every `*_KEY` macro listed below.
The prefix is the same on VRSCTEST and on production builds — it is
decoupled from the chain name on purpose so the same key macros stay
portable when the deployment moves chains. Renaming it to match the
VRSCTEST parent is cosmetic and would require an on-chain migration;
see `docs/TODO.md`.

**`<game_id>` suffix.** Keys whose comment description says they are
*per-game* are written under `<base_key>.<game_id>` rather than the
base key alone. `<game_id>` is the 32-byte hex string in the table
identity's `T_GAME_ID_KEY`. The helper
`get_key_data_vdxf_id(BASE_KEY, game_id_str)` (in `vdxf.c`) builds
the suffixed form. Keys without this suffix are written once per
identity lifecycle (e.g. an aggregator listing) or carry their own
internal game_id field (e.g. `P_JOIN_REQUEST_KEY`).

**Value type.** The CMM stores hex-encoded values tagged by VDXF
data type id. Three types are in use:

```c
#define STRING_VDXF_ID     "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c"   // vrsc::data.type.string
#define BYTE_VDXF_ID       "iBXUHbh4iacbeZnzDRxishvBSrYk2S2k7t"   // vrsc::data.type.byte
#define BYTEVECTOR_VDXF_ID "iKMhRLX1JHQihVZx2t2pAWW2uzmK6AzwW3"   // vrsc::data.type.bytevector
```

Almost every key in `bet` uses `bytevector` carrying a JSON blob; the
schema column below shows the JSON shape, not the wire bytes.

**Single-writer-per-identity.** No two actors write to the same
identity. The "Written by" column below names exactly one actor per
row, which is what makes the writes serialize without coordination.

---

## Aggregator identities

`cashiers.sg777z.VRSCTEST@` and `dealers.sg777z.VRSCTEST@` are list
identities. Their contentmultimap holds one key each, naming the
short identifiers of registered cashiers and dealers respectively.
Neither identity holds funds; both are pure discovery surfaces.

### `CASHIERS_KEY` *(`chips.vrsc::poker.cashiers`)*

- **Identity:** `cashiers.sg777z.VRSCTEST@`
- **Written by:** operator (manual `updateidentity`) or `bet`'s
  cashier-registration helper
- **Read by:** dealer (cashier discovery), `./bet list cashiers`
- **Payload:**
  ```json
  { "cashiers": ["<cashier_short_name>", ...] }
  ```
- **Suffixed:** no

### `DEALERS_KEY` *(`chips.vrsc::poker.dealers`)*

- **Identity:** `dealers.sg777z.VRSCTEST@`
- **Written by:** `register_dealer` /`deregister_dealer` (see
  `poker/src/dealer_registration.c`)
- **Read by:** player (dealer discovery), `./bet list dealers`,
  `poker_list_dealers`
- **Payload:**
  ```json
  { "dealers": ["<dealer_short_name>", ...] }
  ```
- **Suffixed:** no

---

## Dealer identity keys

A dealer identity (`dealer.sg777z.VRSCTEST@` or `d1.…`, etc.) holds
the dealer's table-template advertisement and its registration
record.

### `T_TABLE_INFO_KEY` *(`chips.vrsc::poker.t_table_info`)*

When written on a *dealer* identity (unsuffixed), this key advertises
the dealer's table parameters from `poker/config/dealer.ini`. The
same macro is reused on the *table* identity (suffixed by `<game_id>`)
to record the parameters of the specific game in progress — see the
Table identity section below.

- **Identity:** dealer identity (e.g. `d1.sg777z.VRSCTEST@`)
- **Written by:** dealer at startup
- **Read by:** players inspecting the dealer's offer
- **Payload:**
  ```json
  {
    "max_players": 2,
    "big_blind":   0.001,
    "min_stake":   20,
    "max_stake":   100,
    "table_id":    "t1.sg777z.VRSCTEST@",
    "dealer_id":   "d1.sg777z.VRSCTEST@",
    "cashier_id":  "cashier.sg777z.VRSCTEST@",
    "start_block": <block height>
  }
  ```
  The numeric `big_blind`/`min_stake`/`max_stake` values are
  serialized through `struct float_num` (see
  `id_creation_process.md`) and the whole `struct table` is what
  ends up as bytes on-chain.
- **Suffixed:** no (on dealer id) / yes (on table id)

### `"registration_info"` *(literal key, not vdxfid-hashed)*

- **Identity:** dealer identity
- **Written by:** `register_dealer` after a successful registration
  `sendcurrency` (see `dealer_registration.c:75`)
- **Payload:**
  ```json
  {
    "tx_data": {
      "dealer_id":   "<dealer_id>",
      "type":        "dealer_registration",
      "amount":      <DEALER_REGISTRATION_FEE>,
      "destination": "dealers"
    },
    "tx_id": "<registration txid>"
  }
  ```
- **Read by:** dealer itself (auditability) and dispute paths.

---

## Table identity keys

The table identity (`t1.sg777z.VRSCTEST@`, etc.) is the most
key-heavy identity in the deployment. Every key here is dealer-owned
under the single-writer rule. Most keys are suffixed by `<game_id>`
so multiple hands on the same table identity don't collide.

### `T_GAME_ID_KEY` *(`chips.vrsc::poker.t_game_ids`)*

- **Written by:** dealer at game start
- **Payload:** the 32-byte game id as a hex string, packaged as
  `{ "hexdata": "<32-byte hex>" }`.
- **Suffixed:** no — this is the bootstrap key that tells readers
  which `<game_id>` to use when reading the rest.

### `T_TABLE_INFO_KEY.<game_id>` *(`chips.vrsc::poker.t_table_info.<game_id>`)*

Same payload shape as the dealer-id version above, but written on
the table identity and suffixed by the current game id. Acts as the
authoritative table-parameter snapshot for the game.

### `T_PLAYER_INFO_KEY.<game_id>` *(`chips.vrsc::poker.t_player_info.<game_id>`)*

- **Written by:** dealer (after admitting a player from
  `P_JOIN_REQUEST_KEY`)
- **Read by:** players (to confirm they were admitted), dealer
  (turn order)
- **Payload:**
  ```json
  {
    "num_players": <int>,
    "player_info": ["<verus_pid>_<payin_tx_short>_<player_idx>", ...]
  }
  ```
  Each entry combines the player's verus pid, a short prefix of the
  payin tx, and the assigned seat index.

### `T_BETTING_STATE_KEY.<game_id>` *(`chips.vrsc::poker.t_betting_state`)*

- **Written by:** dealer (every time the action moves to a new
  player)
- **Read by:** all players (to know whose turn it is and what the
  pot/min-amount looks like)
- **Payload:**
  ```json
  {
    "current_turn":  <player index>,
    "round":         <0..3 — preflop / flop / turn / river>,
    "pot":           <total pot in CHIPS>,
    "action":        "small_blind" | "big_blind" | "round_betting",
    "min_amount":    <minimum to call/bet>,
    "possibilities": [<allowed action codes>],
    "bet_amounts":   [<per-player current-round bet>],
    "player_funds":  [<per-player remaining stake>],
    "last_action_str":    "<last action name>",   // optional
    "last_action_amount": <last action amount>,   // optional
    "last_action_player": <player index>          // optional
  }
  ```
  The three `last_action_*` fields are populated so the dealer's
  view of "what just happened" propagates to other players' GUIs
  even when they aren't the one who acted.

### `T_BOARD_CARDS_KEY.<game_id>` *(`chips.vrsc::poker.t_board_cards`)*

- **Written by:** dealer (after consensus that all players decoded
  the relevant card)
- **Read by:** players (community card state, rejoin)
- **Payload:**
  ```json
  {
    "flop":         [<c1>, <c2>, <c3>],   // or [-1,-1,-1]
    "turn":         <c4>,                 // or -1
    "river":        <c5>,                 // or -1
    "last_card_id": <card index just processed>
  }
  ```

### `T_GAME_INFO_KEY.<game_id>` *(`chips.vrsc::poker.t_game_info`)*

This is the canonical home of the game-state machine value
documented in [`game_state.md`](game-states.md).

- **Written by:** dealer (every state transition)
- **Read by:** every actor that polls game state via
  `get_game_state` / `get_game_state_info`
- **Payload:**
  ```json
  {
    "game_state":      <int from enum G_* in game.h>,
    "game_state_info": <optional metadata for this state>
  }
  ```
  *(Note: the docstring on this macro in `vdxf.h:249-253` names the
  fields `t_game_ids` and `game_info`; the code reads `game_state`
  / `game_state_info`. The docstring is wrong — see
  `docs/_code_suggestions.md` item 3.)*

### `T_SETTLEMENT_INFO_KEY.<game_id>` *(`chips.vrsc::poker.t_settlement_info`)*

- **Written by:** dealer (writes the settlement *order*); later the
  dealer canonicalizes the cashier's response back onto the same
  key after observing `C_SETTLEMENT_RESULT_KEY`.
- **Read by:** cashier (settlement order), players (post-hand)
- **Payload:**
  ```json
  {
    "game_id":        "<game_id>",
    "status":         "pending" | "completed" | "failed",
    "winners":        [<player index>, ...],
    "settle_amounts": [<CHIPS per player>, ...],
    "player_ids":     ["<verus_pid>", ...],
    "payin_txs":      ["<txid>", ...],
    "pot":            <total pot>,
    "payout_txs":     ["<txid>", ...]   // filled in after cashier completes
  }
  ```

### Dealer-shuffled deck family

The dealer writes its shuffled+blinded output for each player seat
into one of these per-player keys. See
[`deck_shuffling.md`](../explanation/deck-shuffling.md) for the cryptographic
pipeline.

- `T_D_DECK_KEY` *(`chips.vrsc::poker.t_d_deck`)* — aggregate
  dealer-side deck record (when used).
- `T_D_P1_DECK_KEY` … `T_D_P9_DECK_KEY`
  *(`chips.vrsc::poker.t_d_p<n>_deck`)* — dealer's
  shuffled+blinded output for seat `<n>`.
- All suffixed by `<game_id>`.

### Cashier-canonicalized deck family

The cashier writes its reblinded output to its own identity
(`C_B_P<n>_DECK_KEY`, below). The dealer reads from there and
canonicalizes onto the table identity under the corresponding
`T_B_*` key, giving players a single canonical place to read the
fully-blinded deck. The split is the single-writer migration in
action — `docs/TODO.md` items 1.1 and 1.2 reference it.

- `T_B_DECK_KEY` *(`chips.vrsc::poker.t_b_deck`)*
- `T_B_P1_DECK_KEY` … `T_B_P9_DECK_KEY`
  *(`chips.vrsc::poker.t_b_p<n>_deck`)*
- All suffixed by `<game_id>`.

### `T_CARD_BV_KEY.<game_id>` *(`chips.vrsc::poker.card_bv`)*

- **Status:** legacy table-side card-blinding-values key.
- **Replaced by:** `C_CARD_BV_KEY` on the cashier identity (see
  below). Retained for transitional compatibility; new readers
  should target `C_CARD_BV_KEY`.
- **Payload:** `{ "player_id": <int>, "card_id": <int>, "bv": [...] }`.

---

## Cashier identity keys

The cashier identity (`cashier.sg777z.VRSCTEST@`) holds the cashier's
work products and the response records for settlement and disputes.

### Cashier-shuffled deck family

Same shape as the `T_B_*` family above, but on the cashier identity.
This is where the cashier's output is *originally* written; the
dealer later canonicalizes it onto the table.

- `C_B_DECK_KEY` *(`chips.vrsc::poker.c_b_deck`)*
- `C_B_P1_DECK_KEY` … `C_B_P9_DECK_KEY`
  *(`chips.vrsc::poker.c_b_p<n>_deck`)*
- All suffixed by `<game_id>`.

### `C_CARD_BV_KEY.<game_id>` *(`chips.vrsc::poker.c_card_bv`)*

- **Written by:** cashier (one entry per (player_id, card_id) reveal
  request)
- **Read by:** player (resolves cashier id from `T_TABLE_INFO_KEY`,
  reads bv directly — no dealer canonicalization step needed)
- **Payload:**
  ```json
  { "player_id": <int>, "card_id": <int>, "bv": [<curve25519 point>, ...] }
  ```
- **Suffixed:** by `<game_id>`.

### `C_SETTLEMENT_RESULT_KEY.<game_id>` *(`chips.vrsc::poker.c_settlement_result`)*

- **Written by:** cashier (after `cashier_process_settlement`
  finishes all payouts)
- **Read by:** dealer (canonicalizes status + payout_txs onto
  `T_SETTLEMENT_INFO_KEY`)
- **Payload:**
  ```json
  { "status": "completed", "payout_txs": ["<txid>", ...] }
  ```
- **Also used for re-pay idempotency:** at the top of
  `cashier_process_settlement`, if this key already exists with
  `status: completed`, the cashier returns OK without re-issuing
  payouts.

### `C_DISPUTE_RESULT_KEY` *(`chips.vrsc::poker.c_dispute_result`)*

- **Status:** key layout defined; cashier-side flow not wired up.
- **Intended written-by:** cashier (when resolving a dispute)
- **Intended read-by:** disputing player (to claim refund/payout)
- **Payload:**
  ```json
  {
    "player_id":      "<verus pid>",
    "game_id":        "<game_id>",
    "status":         "refunded" | "paid" | "rejected",
    "payout_tx":      "<txid>",
    "reason":         "<text>",
    "resolved_block": <block height>
  }
  ```
- **See:** `docs/TODO.md` item 3.

---

## Player identity keys

Each player identity (`p1.sg777z.VRSCTEST@` through `p9.…`) is the
player's sole writeable surface. Per the single-writer rule the
player owns all writes to its own identity; the dealer and cashier
only read.

### `P_JOIN_REQUEST_KEY` *(`chips.vrsc::poker.p_join_request`)*

- **Written by:** player at join time (`vdxf.c:join_table`)
- **Read by:** dealer (admit decision, in
  `poker_poll_players_for_joins`)
- **Payload:**
  ```json
  {
    "dealer_id":  "<dealer to join>",
    "table_id":   "<table to join>",
    "cashier_id": "<cashier the payin landed at>",
    "payin_tx":   "<payin txid>"
  }
  ```
- **Suffixed:** no — the payload carries its own context.
- **See:** [`PLAYER_JOIN_FLOW.md`](player-join-flow.md).

### `P_BETTING_ACTION_KEY` *(`chips.vrsc::poker.p_betting_action`)*

- **Written by:** player after each betting decision
- **Read by:** dealer (`verus_poll_player_action`)
- **Payload:**
  ```json
  {
    "round":   <0..3>,
    "action":  "fold" | "call" | "raise" | "check" | "allin",
    "amount":  <CHIPS when raise/allin>,
    "card_id": <card turn this action is for>
  }
  ```
- **Suffixed:** no — only the latest action is read.

### `PLAYER_DECK_KEY.<game_id>` *(`chips.vrsc::poker.player_deck`)*

- **Written by:** player during `player_init_deck`
- **Read by:** dealer (input to its shuffling pass)
- **Payload:**
  ```json
  {
    "id":       <player_id>,
    "pubkey":   "<player keypair public point>",
    "cardinfo": ["<P1>", "<P2>", ..., "<P52>"]   // 52 Curve25519 public points
  }
  ```
- **Suffixed:** by `<game_id>`.

### `P_DECODED_CARD_KEY.<game_id>` *(`chips.vrsc::poker.p_decoded_card`)*

- **Written by:** player (cumulative snapshot — see the long
  docstring at `vdxf.h:189-218`)
- **Read by:** dealer (community-card consensus check)
- **Payload:**
  ```json
  {
    "game_id":       "<game_id>",
    "decoded_cards": [
      { "card_id": <int>, "card_type": <int>, "card_value": <0..51> },
      ...
    ]
  }
  ```
  Each write re-publishes the full array (rather than appending a
  single entry) so the latest CMM entry always contains the
  player's complete decoded-card view.

### `P_HOLECARDS_REVEAL_KEY.<game_id>` *(`chips.vrsc::poker.p_holecards_reveal`)*

- **Written by:** player exactly once per hand, at `G_SHOWDOWN`
- **Read by:** dealer (input to 7-card hand evaluation in
  `poker.c:seven_card_draw_score`)
- **Payload:**
  ```json
  { "game_id": "<game_id>", "hole_cards": [<v0>, <v1>] }
  ```
- Kept separate from `P_DECODED_CARD_KEY` so hole-card secrecy is
  preserved through the hand — see the docstring at `vdxf.h:221-246`
  for the rationale.

### `P_GAME_HISTORY_KEY` *(`chips.vrsc::poker.p_game_history`)*

- **Written by:** player at join time
- **Payload:**
  ```json
  {
    "payin_tx":   "<txid>",
    "table_id":   "<table>",
    "dealer_id":  "<dealer>",
    "game_id":    "<game_id>",
    "join_block": <block height>,
    "amount":     <CHIPS paid in>
  }
  ```
- **Suffixed:** no — one entry per join.

### `P_DISPUTE_REQUEST_KEY` *(`chips.vrsc::poker.p_dispute_request`)*

- **Status:** key layout defined; player-side `bet` command not yet
  wired up. See `docs/TODO.md` item 3.
- **Intended written-by:** player when raising a dispute
- **Intended read-by:** cashier (`cashier_poll_disputes`)
- **Payload:**
  ```json
  {
    "payin_tx":      "<txid>",
    "table_id":      "<table>",
    "game_id":       "<game_id>",
    "reason":        "no_payout" | "game_aborted" | "timeout",
    "request_block": <block height>
  }
  ```

`DISPUTE_TIMEOUT_BLOCKS` (currently 100, in `vdxf.h:332`) is the
window after which a pending game is considered aborted and the
dispute path becomes valid.

---

## Reading these keys from the CLI

[`cli-print.md`](cli-print.md) documents the `./bet print_id`,
`print`, `print_keys`, and `show` commands that decode and pretty-print
keys from any identity. Most of the keys above are covered by
`./bet print_id <id> table` (for the table identity), `dealer`,
`cashiers`, or `dealers` — see `cli-print.md` for the per-command key
list.

For programmatic reads, `poker_get_key_json` / `poker_get_key_str`
(declared in `poker/include/poker_vdxf.h`) take a base key macro and
optionally a `game_id_str` and return the parsed value; they handle
the namespace prefix and the vdxfid hashing internally.

---

## Related documentation

- [`verus-overview.md`](../explanation/verus-overview.md) — single-writer rule,
  CMM size limit, update mechanics.
- [`architecture.md`](../explanation/architecture.md) — the layered call path
  that turns these key reads into RPC calls.
- [`game_state.md`](game-states.md) — state machine driving the
  writes to `T_GAME_INFO_KEY`.
- [`deck_shuffling.md`](../explanation/deck-shuffling.md) — how
  `T_D_*`/`T_B_*`/`C_B_*` and `T_CARD_BV`/`C_CARD_BV` are used
  during the shuffle.
- [`player-rejoin.md`](../explanation/player-rejoin.md) — which keys a
  reconnecting player reads.
- [`PLAYER_JOIN_FLOW.md`](player-join-flow.md) — concrete walkthrough
  of `P_JOIN_REQUEST_KEY` end to end.
- [`cli-print.md`](cli-print.md) — CLI-side reads of every key
  above.
