# How `bet` uses Verus identities

`bet` stores every piece of game state that needs to be shared between
nodes — payin transactions, deck shuffling output, betting rounds, card
reveals, settlement — on the Verus blockchain. Each actor (dealer,
cashier, players) writes only to the keys on its own identity and reads
from the others. This replaced an earlier nanomsg pub-sub flow that has
been removed from the codebase; every `nanomsg/pub-sub no longer used`
comment in the source tree marks where the old flow used to be.

This document is the architectural overview of how that works today. For
the player-join flow specifically see `docs/reference/player-join-flow.md`; for the
CLI tooling that inspects on-chain state see `docs/reference/cli-print.md`;
for the per-key reference see `docs/reference/vdxf-keys.md`.

---

## Identities and parent namespace

The deployment is rooted at a single parent identity registered on the
chain. On the local VRSCTEST regtest used for development, that's
`sg777z.VRSCTEST@` — configured in `poker/config/keys.ini` under
`[ids] parent_id` and resolved at startup. Whichever VRSCTEST identity
the operator controls can be substituted; the `bet` binary doesn't
hardcode `sg777z`.

Under the parent live the actor sub-identities:

- `dealer.sg777z.VRSCTEST@` — the dealer node identity.
- `t1.sg777z.VRSCTEST@` — a table identity owned by the dealer. The
  dealer can register multiple table identities under the same parent
  and host multiple tables off the same daemon.
- `cashier.sg777z.VRSCTEST@` — the cashier that receives payin
  transfers and holds the table's funds across a hand.
- `p1.sg777z.VRSCTEST@` … `p9.sg777z.VRSCTEST@` — per-player
  identities. The current discovery list (`known_players[]` in
  `poker_vdxf.c` and `blinder.c`) hardcodes nine slots; replacing it
  with a configured list is `docs/TODO.md` item 2.
- `dealers.sg777z.VRSCTEST@` and `cashiers.sg777z.VRSCTEST@` —
  aggregator identities whose contentmultimap entries list the
  registered dealers and cashiers respectively. The `./bet list dealers`
  command and `poker_list_dealers()` read directly from this aggregator.

Each of these identities is independent at the protocol level — a
distinct UTXO secures each one, and updates to one don't interact with
updates to another.

---

## The contentmultimap as a key-value store

Every Verus identity owns a contentmultimap (CMM): an on-chain
dictionary keyed by VDXF id hash, valued by a hex-encoded blob.
`bet` stores poker state by writing well-known keys with JSON values
into this dictionary on the appropriate identity.

The CMM key namespace `bet` uses is human-readable and prefixed with
`chips.vrsc::poker.sg777z.` in the current build — defined once as
the `VDXF_POKER_KEYS_PREFIX` macro in `poker/include/vdxf.h` and
reused by every `*_KEY` macro in the same header. The prefix is
hashed to a VDXF id when actually placed on-chain; the readable form
exists only so a developer can grep `vdxf.h` and see what
`chips.vrsc::poker.sg777z.t_betting_state` maps to. Renaming the prefix
to match the new VRSCTEST parent (`vrsctest::poker.sg777z.` or similar)
is a cosmetic cleanup that doesn't change on-chain behaviour; it's
tracked separately.

### The single-writer-per-identity rule

Each key in the CMM is updated by exactly one actor. The dealer owns
all writes to the table identity (`T_TABLE_INFO_KEY`,
`T_PLAYER_INFO_KEY`, `T_BETTING_STATE_KEY`, `T_BOARD_CARDS_KEY`,
`T_SETTLEMENT_INFO_KEY`, the dealer-side `t_d_p*_deck` keys, etc.). The
cashier owns all writes to its own identity (`C_B_P*_DECK_KEY`,
`C_CARD_BV_KEY`, `C_DISPUTE_RESULT_KEY`). Each player owns all writes
to its own player identity (`P_JOIN_REQUEST_KEY`,
`P_BETTING_ACTION_KEY`, `P_DECODED_CARD_KEY`,
`P_DISPUTE_REQUEST_KEY`). No identity has two concurrent writers.

This rule is the architectural reason updates serialize without
coordination logic — there's never a "who wrote first?" race, because
only one actor is even trying. It's also why two of the keys that
historically lived on the table id (`T_B_P*_DECK_KEY` and
`T_CARD_BV_KEY`) were moved to the cashier id: those are cashier-owned
writes, and keeping them on the table id forced the dealer and cashier
to share a writer. `docs/TODO.md` items 1.1 and 1.2 are the closing
write-ups of that migration.

---

## How updates work, and why concurrent writers would be a problem

Updating an identity in Verus is a regular transaction. It spends the
identity's current UTXO and produces a new UTXO carrying the updated
contentmultimap state. Two updates targeting the same identity
therefore both try to spend the same UTXO — only one can win per block.
The other lands a `bad-txns-inputs-spent` error from the daemon.

If two writers ever did end up on the same identity, the loser has two
options:

1. Wait for the next block, observe the new UTXO produced by the
   winner, and retry against that.
2. Walk the mempool, find the in-flight spend, and chain its own spend
   on top of it before either lands. This is the path Verus exposes
   through mempool-aware reads (`heightend = -1` passed to
   `getidentitycontent`), but `bet` does not currently take it.

`bet` uses option 1 for simplicity, via `update_with_retry` in
`poker/src/vdxf.c`. Each `updateidentity` call is followed by
`wait_for_a_blocktime()` and a `check_if_tx_exists` confirmation. On
VRSCTEST this costs roughly one blocktime per write per identity
(typically two to three seconds in the current test setup).

A hardcoded 3-second sleep that used to follow each successful update
was removed in May 2026 after empirical verification on VRSCTEST that
`wait_for_a_blocktime()` + `check_if_tx_exists` is sufficient settle
time on its own. That change cut roughly two and a half seconds of
wall-clock latency per identity update. Comments at
`poker/src/vdxf.c:update_with_retry` record the rationale.

---

## Reading: `getidentity` vs `getidentitycontent`

`getidentity` returns only the most recently written value for each
contentmultimap key — whatever appeared in the latest UTXO spend. That
is correct for keys written exactly once per game (the table template,
the initial dealer deck), but it would mask history for any key that
gets updated multiple times per hand: the betting state turns over once
per action, decoded-card snapshots cumulate per round, and the
settlement info evolves through the showdown.

`getidentitycontent` returns the full update history for an identity
within a block range. Each player and the dealer event loop use this
to reconstruct the round-by-round state of any key whose semantics are
cumulative, by passing a `heightstart` close to the game's start block.
The semantics and rationale for adding this RPC to Verus are described
in `docs/explanation/getidentitycontent.md`.

---

## On-chain size limit, and what it implies for deck shuffling

A single `updateidentity` transaction's serialized identity object is
capped by the Verus daemon at `MAX_SCRIPT_ELEMENT_SIZE_PBAAS` (6000
bytes) minus 128 bytes of overhead — 5872 bytes for the identity
payload itself. This is enforced inside `CIdentity::IsValid` on the
daemon side. We verified this limit empirically in May 2026 while
investigating whether the deck shuffle phase could batch all per-player
deck writes into one transaction.

For a 52-card deck encoded as the JSON value `bet` writes today, one
full per-player deck fits in a single update and two do not. That's
why deck shuffling fans out into N sequential `updateidentity` calls —
one per player deck — rather than batching all decks into a single
transaction. The cashier walks the same fan-out when committing the
per-player blinded deck back onto its own identity. Optimization
options that don't require batching (binary packing, cross-actor
pipelining, mempool-aware UTXO chaining) are listed in
`docs/TODO.md`.

---

## What's still on the local SQLite DB

`bet_sqlite3_init` in `poker/src/storage.c` still creates
`~/.bet/db/pangea.db` with 14 tables on every node startup. Most of
those tables (`dcv_tx_mapping`, `player_tx_mapping`,
`cashier_tx_mapping`, `*_game_state`, `*_deck_info`) are pre-Verus
leftovers — they are created on startup but never read by the live
game loop in the current build. The schema is preserved so old
databases continue to open without migration errors.

The one table that the current build actively writes and reads is
`player_local_state`, which records the player's payin tx, decoded
cards, last decoded card id, and last-known game state. This is what
lets a player rejoin after a crash without re-deriving everything from
the chain. `docs/explanation/player-rejoin.md` describes how
that data is used during reconnect.

---

## Cost and latency profile

A typical two-player hand on VRSCTEST issues on the order of twenty to
thirty `updateidentity` calls in total across all actors: deck commits
(five or six writes once shuffling is in flight), reveal blinding
values per round (one per card revealed by the cashier), per-round
betting actions per player, and a single settlement write at the end.
Each costs one transaction fee on the chain and one blocktime of
latency on the wire.

On a local regtest both are negligible. On a public chain the per-game
cost is the sum of those fees, and the wall-clock latency is roughly
*N × blocktime* where *N* is the longest serial chain of writes for a
single identity. The biggest contributor to wall-clock time is the
deck shuffle phase (player decks committed by the dealer, then
re-committed by the cashier, each in serial). Reducing that latency is
what `docs/TODO.md` tracks as a future pipelining target now that the
batch-into-one-tx approach has been ruled out by the script element
size limit.

---

## What this overview deliberately does not implement

Two concepts that show up in adjacent literature are not part of the
current build and need to be called out so readers don't assume they
exist:

**A Verus-native heartbeat protocol.** The legacy nanomsg heartbeat
code survives in `poker/src/heartbeat.c` as compile-time dead code —
the thread that would run `bet_dcv_heartbeat_loop` is launched only
under `#ifdef LIVE_THREAD`, which is not defined in the current build
(`bet.c:45` keeps the corresponding `#define` commented out). There is
no equivalent on-chain heartbeat yet. Player liveness today is
inferred from how long it's been since a player wrote to their
`P_BETTING_ACTION_KEY` after the dealer prompted them; a configured
timeout there is treated as an implicit fold. That works for game
progress but it does not yet detect a player who has dropped between
hands.

**Dispute resolution by the cashier.** The functions
`cashier_poll_disputes` and `cashier_resolve_dispute` exist in
`poker/src/vdxf.c`, and the on-chain key layout is defined
(`P_DISPUTE_REQUEST_KEY` on the player identity,
`C_DISPUTE_RESULT_KEY` on the cashier). The cashier's main loop does
not currently invoke them — the dispute path is dormant until the
open questions in `docs/TODO.md` item 3 are settled.
