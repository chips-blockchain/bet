# Reading identity history with `getidentitycontent`

Verus exposes two RPCs for inspecting a contentmultimap. `getidentity`
returns the **current** state: whatever values the latest UTXO encodes,
exactly as it was written. `getidentitycontent` returns the **history**:
every update applied to the identity within a given block range,
combined into a single cumulative view of the contentmultimap. `bet`
uses both — and the difference between them matters for any key that
gets written more than once during a hand.

The distinction is structural, not just operational. Every successful
`updateidentity` transaction **replaces** the contentmultimap of the
identity wholesale: the new UTXO encodes the CMM the caller submitted,
and any key not present in that submission is implicitly gone from the
latest view. The comment in `poker/src/vdxf.c:252` calls this out
plainly: *"Each updateidentity REPLACES the CMM, but getidentitycontent
COMBINES all updates from height_start."*

This is the reason `bet` cannot rely on `getidentity` alone. A player
who writes their preflop betting action under `P_BETTING_ACTION_KEY`
will overwrite that key when they write their flop action; `getidentity`
on the player identity after the river will return only the river
action. To reconstruct the hand, the dealer needs every value the key
has held since the start block. That is exactly what
`getidentitycontent` provides.

---

## The RPC call

```
verus -chain=VRSCTEST getidentitycontent <id> <heightstart> <heightend>
```

`heightstart` is the first block to include; `heightend` is the last,
with `-1` meaning "include the mempool" so unconfirmed updates are
folded into the returned view. `bet`'s wrapper
(`vdxf.c:get_cmm_from_height`) hardcodes `heightend = -1` for exactly
this reason — it wants the most current possible reconstruction without
waiting for the next block.

The response carries a `contentmultimap` field — either at the top
level or nested inside an `identity` object, depending on daemon
version; the wrapper handles both shapes. Each key in the returned CMM
holds the merged value across every update between `heightstart` and
`heightend`, in the order the updates were applied.

---

## How `bet` calls it

Every read in `bet` that depends on multi-update history goes through
one of two wrappers in `poker/src/vdxf.c`:

- `get_cmm_from_height(id, height_start)` — fetches the full CMM and
  lets the caller pick keys.
- `get_cmm_key_data_from_height(id, key, height_start)` — thin
  wrapper around the above that returns one key.

`id` is always a fully-qualified Verus ID; the helpers no longer
take a separate short-name/FQN flag.

Higher-level helpers (`get_cJSON_from_id_key_vdxfid_from_height`,
`get_str_from_id_key_from_height`) take a `height_start` argument and
fan out through the same RPC. The dealer's poll loops and the player's
event loop pass the table's start block as `height_start` so each read
naturally bounds itself to the current hand and skips noise from
previous games on the same identity.

`getidentity`, by contrast, is used for keys that are written exactly
once and read once (the table template, the initial join request),
where the history view would be unnecessary overhead.
