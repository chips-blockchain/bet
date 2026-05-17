# Pangea-Bet Documentation

This directory is the documentation root for the `bet` (Pangea poker) project.
It follows the [Diátaxis](https://diataxis.fr) layout: each piece of
documentation has exactly one role.

| Bucket           | Role                                                                 |
|------------------|----------------------------------------------------------------------|
| `tutorials/`     | Walkthroughs you follow start to finish — "do this to get a working system." |
| `guides/`        | Task-oriented how-tos for a specific operational job.                |
| `reference/`     | Look-it-up facts: configs, message formats, keys, RPCs, glossary.    |
| `explanation/`   | Conceptual / architectural background — *why* it works the way it does. |
| `archive/`       | Historical docs preserved for context, no longer authoritative.      |

A short, audience-driven index follows. The full list of files lives at the
bottom.

---

## I'm new — get me a running system

1. **[Build `bet` from source](./guides/build-from-source.md)** — submodules,
   system packages, `make -j`.
2. **[Bring up a local VRSCTEST regtest](./tutorials/cli-auto-vrsctest.md)** —
   the canonical single-host setup. Patched `verusd`, chain config, the
   pre-existing tmux sessions, the `--reset` CLI-auto flow.
3. **[Community quick-start (CLI)](./tutorials/community-quickstart.md)** — what a
   new contributor / tester should run to sanity-check the stack headlessly.
4. **[Community quick-start (GUI)](./tutorials/community-quickstart-gui.md)** — how to
   set up the backend nodes and connect them to the React frontend GUI to play manually.

## I'm running a node

* **Dealer** — [dealer-config.md](./reference/dealer-config.md)
* **Player** — [player-config.md](./reference/player-config.md)
* **Cashier** — [cashier-config.md](./reference/cashier-config.md)
* **Identity setup** — [revoke-recovery.md](./tutorials/revoke-recovery.md)
  walks the `registernamecommitment` + `registeridentity` pair, plus the
  revocation/recovery authority model.

## I'm building a GUI / integrating

* **[GUI message formats](./reference/gui-message-formats.md)** — full
  WebSocket reference (per-role port, every method, every payload).
* **[GUI quick reference](./reference/gui-quick-reference.md)** — one-page
  cheat sheet derived from the above.
* **[GUI ↔ backend mapping](./reference/gui-backend-mapping.md)** — developer
  view: which C file emits which message.
* **[GUI simulator guide](./guides/gui-simulator.md)** — `tools/gui_simulator.js`
  for headless integration tests.

## I want to understand how it works

* **[Verus overview](./explanation/verus-overview.md)** — how `bet` uses
  Verus identities, what lives on-chain vs. locally, the single-writer rule.
* **[Architecture](./explanation/architecture.md)** — layered structure of
  the codebase and the Verus RPC abstraction.
* **[Identity tree](./explanation/identity-tree.md)** — aggregator vs.
  operational vs. per-table / per-player identities and how `bet` creates
  them.
* **[Ad-hoc multisig](./explanation/adhoc-multisig.md)** — what "ad-hoc
  multisig" means on Verus (`primaryaddresses` + `minimumsignatures` +
  revocation/recovery) vs. the legacy P2SH path.
* **[Node communication](./explanation/node-communication.md)** — CMM-only
  model, contrasted with the legacy nng pub/sub world.
* **[Deck shuffling](./explanation/deck-shuffling.md)** — multi-pass
  Curve25519 shuffle and how it lands on CMM keys.
* **[`getidentitycontent`](./explanation/getidentitycontent.md)** — how `bet`
  reconstructs CMM state from `height_start` and why it matters.
* **[Player rejoin](./explanation/player-rejoin.md)** — turn timeout,
  reconnect, local state recovery.

## Reference index

* [Glossary](./reference/glossary.md)
* [Game states](./reference/game-states.md) — full `enum game_state`
* [VDXF keys](./reference/vdxf-keys.md) — every CMM key `bet` reads/writes
* [Transaction types](./reference/tx-types.md) — payin / payout / identity update
* [RPC dependency surface](./reference/rpc-dependency.md) — the Verus RPCs
  `bet` shells out to (compatibility checklist for upstream upgrades)
* [`./bet print*` commands](./reference/cli-print.md) — inspect on-chain
  state from the shell
* [Player join flow](./reference/player-join-flow.md) — end-to-end join
  sequence (payin + dealer poll + cashier verify)

## Project metadata

* **[`TODO.md`](TODO.md)** — current deferred-work backlog.
* **[`_inventory.md`](_inventory.md)** — Phase-1/2/3 documentation
  migration worksheet (decision record).
* **[`_code_suggestions.md`](_code_suggestions.md)** — minor code-comment
  / help-text inconsistencies found during the doc overhaul.

## Archive

Historical docs (LN-era protocol, removed features, dated milestones) live
under [`archive/`](./archive/). They are preserved for context but are no
longer authoritative — every archived file has been superseded by content in
the buckets above.

---

## Directory layout

```
docs/
├── README.md           (this file)
├── TODO.md             (deferred-work backlog)
├── _inventory.md       (doc-migration decision record)
├── _code_suggestions.md
├── tutorials/
│   ├── cli-auto-vrsctest.md
│   ├── community-quickstart.md
│   └── revoke-recovery.md
├── guides/
│   ├── build-from-source.md
│   └── gui-simulator.md
├── reference/
│   ├── glossary.md
│   ├── dealer-config.md
│   ├── player-config.md
│   ├── cashier-config.md
│   ├── gui-message-formats.md
│   ├── gui-quick-reference.md
│   ├── gui-backend-mapping.md
│   ├── tx-types.md
│   ├── rpc-dependency.md
│   ├── game-states.md
│   ├── vdxf-keys.md
│   ├── cli-print.md
│   └── player-join-flow.md
├── explanation/
│   ├── verus-overview.md
│   ├── architecture.md
│   ├── identity-tree.md
│   ├── adhoc-multisig.md
│   ├── node-communication.md
│   ├── deck-shuffling.md
│   ├── getidentitycontent.md
│   └── player-rejoin.md
└── archive/            (historical / superseded)
```

For the top-level project overview, see [`../README.md`](../README.md).
