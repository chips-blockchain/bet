# Identity creation for `bet`

A working `bet` deployment depends on a small fixed family of Verus
identities. They divide into two lifecycles: **permanent**, created
once at deployment time and used for the life of the system, and
**per-table**, created on demand by a dealer and discarded after the
game is over (the identities themselves live forever on-chain, but the
deployment stops using them). This document covers what identities
exist, who creates each one, what their `contentmultimap` is used for,
and how `bet`'s built-in commands fit into the picture.

The mechanics of `registernamecommitment` + `registeridentity` and the
choice of revoke/recovery authorities are documented separately in
[`revoke-recovery.md`](../tutorials/revoke-recovery.md). This document assumes
those authorities already exist.

---

## The identity tree

The deployment is parented at a single identity. On the local VRSCTEST
regtest:

```
sg777z.VRSCTEST@                       (parent, permanent)
├── dealers.sg777z.VRSCTEST@           (dealer aggregator, permanent)
├── cashiers.sg777z.VRSCTEST@          (cashier aggregator, permanent)
├── dealer.sg777z.VRSCTEST@            (operational dealer, permanent)
├── cashier.sg777z.VRSCTEST@           (operational cashier, permanent)
├── p1.sg777z.VRSCTEST@ … p9.sg777z.VRSCTEST@   (player slots, permanent)
└── t1.sg777z.VRSCTEST@ … tN.sg777z.VRSCTEST@   (per-table, transient)
```

The parent name (`sg777z`) is an example operator. Replace it with
whichever VRSCTEST identity owns the deployment; the `bet` binary
reads the name from `poker/config/keys.ini` and doesn't hardcode it.

A subtle distinction worth pinning down up front: the **aggregator**
identities (`dealers.…`, `cashiers.…`) are *not* the addresses funds
go to. They are list identities whose `contentmultimap` holds the
fully-qualified Verus IDs of registered dealers and cashiers
respectively. The
**operational** identities (`dealer.…`, `cashier.…`) are the actual
signing-and-funding entities. `bet` reads the aggregator to discover
who's online, then talks to the operational identities directly. The
old documentation treated `cashiers.…` as the multisig vault holding
funds; that is no longer accurate, and conflating the two will lead
you down the wrong code paths.

---

## Permanent identities

All five categories of permanent identities are created out-of-band
via the Verus CLI before any `bet` process starts. The two-step
`registernamecommitment` + `registeridentity` flow from
[`revoke-recovery.md`](../tutorials/revoke-recovery.md) is used for each one,
with the parent set to `sg777z.VRSCTEST@` and the revoke/recovery
authorities set to the deployment-wide pair created in that
document.

### Parent identity

The root of the namespace. Its `contentmultimap` is unused by `bet` —
the parent exists purely so its children inherit a namespace and so
sub-IDs can be created under it. Whoever holds the parent's primary
addresses can mint new sub-IDs, which is the protocol's natural
gatekeeper for who can register a dealer or a cashier.

### Cashier aggregator and operational cashier

Two separate identities, with different jobs:

- `cashiers.sg777z.VRSCTEST@` (aggregator) — its
  `contentmultimap` holds a single key (`chips.vrsc::poker.cashiers`)
  whose value is a JSON array of the fully-qualified Verus IDs of
  currently registered cashiers. The dealer reads this to discover
  which operational cashier identity to direct payins to.
  `bet list cashiers` prints this list.
- `cashier.sg777z.VRSCTEST@` (operational) — the identity that
  payments land on. Players send their payins to this identity's
  i-address via `verus_sendcurrency_data` (see
  [`adhoc-multisig.md`](adhoc-multisig.md) for the signing
  semantics). The cashier process holds the primary-address private
  keys for this identity and can sign payouts out of its UTXOs at
  settlement.

A multi-cashier deployment can run more operational cashier
identities (e.g. `cashier2.sg777z.VRSCTEST@`) and add their FQNs to
the aggregator's CMM. The cashier identity itself becomes
the multisig vault when its `primaryaddresses` are extended and
`minimumsignatures` is raised — but the aggregator stays a plain
"who's registered" list.

### Dealer aggregator and operational dealer

Same shape as the cashier pair. `dealers.sg777z.VRSCTEST@` is the
list identity; `dealer.sg777z.VRSCTEST@` is the operational dealer
that hosts tables and writes per-game state. The aggregator's
`contentmultimap` value under `chips.vrsc::poker.dealers` is
the JSON array of registered dealer FQNs. `bet list dealers` reads
it.

The dealer registration flow is more interesting than the cashier
one because `bet` automates it. The dealer-side command
`./bet register_dealer <dealer_id>` does the following (see
`poker/src/dealer_registration.c:75`):

1. Verify that `<dealer_id>` is a real on-chain identity
   (`is_id_exists`).
2. Verify the dealer is not already in the aggregator's list
   (`is_dealer_registered`).
3. Check the local wallet has at least `DEALER_REGISTRATION_FEE`
   CHIPS.
4. Submit a `sendcurrency` transaction to the dealer aggregator
   identity carrying a JSON payload:
   ```json
   {
     "dealer_id":   "<dealer_id>",
     "type":        "dealer_registration",
     "amount":      <DEALER_REGISTRATION_FEE>,
     "destination": "dealers"
   }
   ```
5. Record the registration on the dealer's own identity under a
   `"registration_info"` key, capturing the tx_data and txid so the
   registration is auditable from the dealer side later.

Notice the symmetry with the join flow described in
[`PLAYER_JOIN_FLOW.md`](../reference/player-join-flow.md): the on-chain
operations are deliberately structured the same way (fee transfer to
an aggregator + metadata commit on the actor's own identity), so the
dispute and rejoin paths can use the same primitives. The matching
deregistration command (`./bet deregister_dealer`) is implemented in
the same file and reverses these steps.

There is no separate "Registration Authority" actor in the code. The
older documentation talked about an RA approving dealer requests; in
the current build, the gatekeeping is implicit — only an actor
holding `primaryaddresses` of the parent can mint a new dealer
sub-ID in the first place, so the parent-keyholder *is* the
registration authority.

### Player identities

`p1.sg777z.VRSCTEST@` through `p9.sg777z.VRSCTEST@` are permanent,
one per seat at a maximum-sized table. They are created out-of-band
like the others. Each player process holds the primary-address
private key for its own player identity and writes to its own
`P_JOIN_REQUEST_KEY`, `P_BETTING_ACTION_KEY`,
`P_DECODED_CARD_KEY`, etc. (see
[`verus-overview.md`](verus-overview.md) for the single-writer
rule).

The player-discovery list in
`poker_vdxf.c:poker_poll_players_for_joins` is currently hardcoded
to `{"p1", …, "p9"}`. Replacing it with a configurable list is
tracked as `docs/TODO.md` item 2.

---

## Per-table identities

The dealer creates a separate sub-identity for each table it hosts.
On the dev regtest these are named `t1.sg777z.VRSCTEST@`,
`t2.sg777z.VRSCTEST@`, etc., with the parent set to the same
namespace as the dealer. The `contentmultimap` of a table identity is
where the per-game state actually lives: `T_GAME_ID_KEY`,
`T_TABLE_INFO_KEY`, `T_PLAYER_INFO_KEY`, `T_BETTING_STATE_KEY`,
`T_BOARD_CARDS_KEY`, `T_SETTLEMENT_INFO_KEY`, the dealer-shuffled
deck keys, and the canonicalized cashier blinded-deck keys. The key
reference for all of these lives in
[`ids_keys_data.md`](../reference/vdxf-keys.md).

Table identities are *protocol-permanent* (the identity record never
goes away on-chain) but *operationally transient* (once the game is
over, the deployment stops writing to that table's keys). A dealer
who shuts down a table won't reuse the same `tN` identity for the
next table; instead it creates a fresh one. There is no garbage
collection — old table identities accumulate in the namespace as a
historical record.

---

## Dealer configuration and on-chain commit

When a dealer process starts, it reads
`poker/config/dealer.ini` (NOT `verus_dealer.ini` — that string
survives only in one stale error message; see
`docs/_code_suggestions.md` item 2):

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
```

Every identity field is a fully-qualified Verus ID — the parser
rejects bare short names. The numeric `table` parameters
(`big_blind`, `min_stake`, `max_stake`) are stored on-chain in a
compact binary form using the `struct float_num` defined in
`poker/include/bet.h:292`:

```c
struct float_num {
    uint32_t mantisa  : 23;
    uint32_t exponent : 8;
    uint32_t sign     : 1;
};

struct table {
    uint8_t           max_players;
    struct float_num  min_stake;
    struct float_num  max_stake;
    struct float_num  big_blind;
    char              table_id[16];
};
```

`float_to_uint32_s` / `uint32_s_to_float` (`poker/src/misc.c:91-107`)
encode and decode the float values. The encoded `struct table` is
serialized to a hex string and written to the dealer's identity
under the dealer-info key, where players and cashiers can read it.
Compact binary packing makes the on-chain entry small (the table
template is one of the more frequently-read keys); the trade-off is
that the value is not human-readable unless decoded with the matching
struct. `./bet print_id d1 dealer` shows the decoded form (see
[`cli-print.md`](../reference/cli-print.md)).

---

## Related documentation

- [`revoke-recovery.md`](../tutorials/revoke-recovery.md) — one-time setup of
  the deployment-wide revoke/recovery authority pair, and the
  two-step `registernamecommitment` + `registeridentity` flow.
- [`adhoc-multisig.md`](adhoc-multisig.md) — how `primaryaddresses`
  and `minimumsignatures` on the operational cashier identity
  control fund movement.
- [`verus-overview.md`](verus-overview.md) — the
  single-writer-per-identity rule that motivates the per-actor
  identity layout.
- [`PLAYER_JOIN_FLOW.md`](../reference/player-join-flow.md) — the live player
  join flow, mirror of the dealer registration flow described above.
- [`ids_keys_data.md`](../reference/vdxf-keys.md) — full key reference for
  table, dealer, cashier, and player identities.
