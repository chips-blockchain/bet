# Glossary

Terminology used across the `bet` codebase and the rest of this
documentation tree. Verus-specific terms (identity, contentmultimap,
vdxfid) are kept close to how Verus itself uses them; poker-specific
terms (DCV, BVV) reflect what they're called in the C source even
when the public-facing name is plainer.

---

**Aggregator identity.** A Verus identity whose contentmultimap holds
a discovery list rather than per-game state. The two aggregators in
`bet` are `dealers.<parent>@` (list of registered dealer short names)
and `cashiers.<parent>@` (list of registered cashier short names).
Distinct from the operational identities of the same names
(`dealer.<parent>@`, `cashier.<parent>@`) which actually sign and
hold funds. See [`id_creation_process.md`](../explanation/identity-tree.md).

**BVV.** Blinding Value Vendor — the historical name in the
codebase for the actor that produces card blinding values during
shuffling. The cashier plays this role today; the BVV/cashier
distinction is preserved in some struct names (`bvv_state`,
`dcv_bvv_*`) but is otherwise dormant since the nanomsg-era
multi-cashier flow was retired.

**Cashier.** The operational entity that holds payin funds during a
hand and issues payouts at settlement. Named on-chain as the
`cashier.<parent>@` identity in the dev deployment; can be extended
to multiple cashier identities in production. See
[`adhoc-multisig.md`](../explanation/adhoc-multisig.md) for the
signing semantics.

**CHIPS.** The chain ticker `bet` was originally written for. On the
local dev regtest the chain is `VRSCTEST`, but the readable key
namespace (`chips.vrsc::poker.`) still uses the legacy chain
name — it's an opaque string at the protocol level and renaming it
is cosmetic.

**ContentMultiMap (CMM).** Verus identity feature that stores a
dictionary of `vdxfid → [{type_vdxfid → value}]` on the identity
record. `bet`'s on-chain state lives almost entirely in
contentmultimaps. Updating an identity rewrites its entire CMM in
one transaction — see [`verus-overview.md`](../explanation/verus-overview.md).

**DCV.** Deck Creating Vendor — the codebase's name for the dealer
in deck-shuffling contexts. The `dcv_vars` global struct
(`game.c`) tracks the dealer's per-hand state. The name persists for
historical continuity; "dealer" is the equivalent term everywhere
else.

**game_id.** The 32-byte random identifier `bet` generates at the
start of each hand. Many per-game CMM keys are suffixed with this
value so multiple hands on the same table identity don't collide.

**i-address.** A Verus identity's on-chain address, derived from
the identity's name. Format `i...`. Where currency is sent to an
identity (e.g. payin to `cashier.<parent>@`), the daemon resolves the
name to its current i-address.

**Identity (Verus identity).** A first-class on-chain object that
combines an i-address, a contentmultimap, primary addresses,
signature threshold, and revoke/recovery authorities. Verus
identities replace what would otherwise be plain addresses + an
off-chain registry — `bet` uses one identity per actor (dealer,
cashier, each player) plus one per table.

**minimumsignatures.** The threshold field on a Verus identity
saying how many of its `primaryaddresses` must sign for the identity
to update or spend. Equivalent of `m` in Bitcoin's `m-of-n` multisig,
but reconfigurable without changing the identity name. Defaults to
`1` on the dev regtest.

**Notary node.** Older project terminology for what is now called a
*cashier*. The two are interchangeable in legacy docs.

**Operational identity.** An identity whose private keys are held
by a running `bet` node and used to sign updates. Distinguished
from aggregator identities (above) and per-table identities
(below). The operational identities in `bet` are the parent, the
dealer, the cashier, and the per-player identities.

**Payin.** The transaction by which a player commits funds to a
hand. Today this is a plain `sendcurrency` from the player to the
cashier identity's i-address. See
[`PLAYER_JOIN_FLOW.md`](player-join-flow.md).

**primaryaddresses.** The list of addresses (R-addresses or other
identities) whose signatures are accepted as authority over a Verus
identity. Equivalent of `n` in `m-of-n` multisig.

**Per-table identity.** A Verus identity (`t1.<parent>@`,
`t2.<parent>@`, etc.) created by the dealer for each table it hosts.
The contentmultimap of a per-table identity is where game state
lives. Operationally transient — once a hand ends, that identity's
keys are no longer updated, though the identity itself persists
on-chain.

**Recovery authority.** Verus identity field naming the identity
allowed to restore a revoked identity. Set at registration time,
separately from the day-to-day signing set. See
[`revoke-recovery.md`](../tutorials/revoke-recovery.md).

**Revocation authority.** Verus identity field naming the identity
allowed to mark another identity as revoked. Same registration flow
as recovery authority.

**Single-writer-per-identity.** Architectural rule in `bet`:
exactly one actor is allowed to write to any given identity's
contentmultimap. This avoids `bad-txns-inputs-spent` contention on
identity UTXOs and is the reason cashier-owned data was split off
from the table identity onto the cashier identity in the May 2025
migration. See [`verus-overview.md`](../explanation/verus-overview.md).

**T_/P_/C_ key prefix conventions.** Macro prefixes in
`poker/include/vdxf.h` that tag each CMM key by the identity it
lives on: `T_*` keys live on the table identity, `P_*` keys on a
player identity, `C_*` keys on the cashier identity. Full reference
in [`ids_keys_data.md`](vdxf-keys.md).

**Table identity.** Synonym for per-table identity (see above).

**vdxfid.** A 32-byte hash of a human-readable key name (e.g.
`chips.vrsc::poker.t_table_info`). All CMM keys are stored
on-chain by their vdxfid; `get_vdxfid` in `vdxf.c` is the wrapper
that resolves the readable form. The Verus CLI also has a
`getvdxfid` RPC for inspecting the mapping.

**VDXF.** Verus Data eXchange Format — the underlying namespace and
hashing scheme that maps human-readable strings to 32-byte ids.
`bet` uses VDXF for both key names and a small set of typed value
markers (`STRING_VDXF_ID`, `BYTEVECTOR_VDXF_ID`).
