# Node Communication

The legacy `bet` was a "talking nodes" design: dealer published on
nng port `7797`, players pushed on `7798`, cashier-to-dealer messages
went over a third nng channel. The current Verus build replaces all
of that with **identity-content-multimap (CMM) writes on the
shared Verus chain**.

| Concern                          | Legacy (CHIPS era)                        | Current (Verus era)                                  |
|----------------------------------|-------------------------------------------|------------------------------------------------------|
| Dealer → players (broadcast)     | nng pub/sub on TCP `7797`                 | `updateidentity` on `t<N>.<parent>@` CMM             |
| Player → dealer (action)         | nng push/pull on TCP `7798`               | `updateidentity` on `p<N>.<parent>@` CMM             |
| Cashier ↔ dealer                 | nng inproc channel                        | `updateidentity` on `cashier.<parent>@` + reads of `t<N>` |
| Heartbeat                        | nng beacon                                | None (compile-time stub only)                        |
| GUI ↔ backend                    | local WebSocket on `:9000`                | local WebSocket per node (see §GUI section below)    |

All of `bet`'s state-machine progress is therefore a sequence of
on-chain `updateidentity` calls. Every node reads the relevant
identities with `getidentitycontent` (typically a windowed read
from a known block height); there is no direct socket between any
two `bet` nodes.

The nanomsg/nng plumbing is still compiled in (the
`external/nng` submodule still builds, and several handlers in
`host.c` and `client.c` reference the old method names like
`init_d`, `bvv_join`, `add_bvv`) but is unreachable at runtime in
this build — every `nn_send`/`nn_recv` call site has been
explicitly disabled, marked with `// Nanomsg removed` comments
in the source.

---

## How a hand actually flows on the wire

1. **Discovery.** The dealer publishes its FQN into the
   `dealers.<parent>@` aggregator CMM. The cashier discovers
   dealers by reading that same identity. The dealer reads
   `cashiers.<parent>@` to learn which cashier(s) it can use.

2. **Join.** Each player writes a join intent
   (`P_PAYIN_INTENT_KEY`) on its own identity carrying the table
   it wants to sit at. The cashier polls player identities and
   correlates these intents with incoming `sendcurrency` UTXOs.

3. **Game state.** The dealer writes hand-wide state
   (`T_GAME_INFO_KEY`, `T_BETTING_STATE_KEY`, etc.) on the
   `t<N>.<parent>@` table identity. Each `updateidentity` is the
   "broadcast" — there is no separate notification. Other nodes
   poll the table identity (`bet_dcv_listen_loop` or equivalent)
   and act on state changes.

4. **Player actions.** Each player writes its bet into the dealer's
   `T_PLAYER_BET_KEY` slot on the table identity. (Currently this
   write is gated by the dealer's round counter — the player reads
   the dealer's `T_BETTING_STATE_KEY` first to learn which round
   the action belongs to.)

5. **Cashier mirror.** When the cashier reaches a decision it
   cannot get to by polling alone (settlement amounts, payin
   acknowledgements), the dealer writes a request key on the
   cashier identity and the cashier writes the answer back to its
   own identity. This is the "split-writer" pattern referenced in
   [`game_state.md`](../reference/game-states.md).

6. **Payout.** Cashier issues `sendcurrency` to the winning player
   identities; observers pick up the txid by reading the cashier
   identity's `C_SETTLEMENT_KEY`.

Sequence-diagram caveat: because every step is a chain write,
"latency" is dominated by `wait_for_a_blocktime()` (regtest:
typically a few seconds; mainnet: ~60 s). The dealer's `update_with_retry`
helper in `vdxf.c` swallows `bad-txns-inputs-spent` and `mempool
already contains` transients that arise when two writers
accidentally race on the same identity.

---

## GUI communication

Each node still runs a local WebSocket server for its GUI
front-end:

* Dealer: `9000`
* Player: `9001` (P1), `9002` (P2), … configurable via `ws_port` /
  `gui_ws_port` in `player.ini`
* Cashier: `9002` (default in `cashier.ini`)

This is the **only** non-blockchain socket in a current `bet`
deployment. The wire format is documented in
[`GUI_MESSAGE_FORMATS.md`](../reference/gui-message-formats.md).

Inter-node communication (the legacy nng layer) is replaced
entirely by CMM writes — there is no `9000`/`7797`/`7798` socket
between two `bet` nodes anywhere in the current code path.
