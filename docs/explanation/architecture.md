# `bet` architecture

`bet`'s code is organized into four layers between the application
code (the dealer, player, cashier and blinder programs that humans
think about) and the Verus blockchain. Each layer hides the one below
it from its caller: a function in `dealer.c` that wants to read a
table's CMM never has to know whether the daemon is being reached
through HTTP or through `popen("verus -chain=VRSCTEST ...")`, and the
RPC layer doesn't know what a poker table is. This document maps each
layer to the source files that implement it, names the functions that
make up the inter-layer contract, and shows how a typical call
descends and returns.

The layering is intentional rather than incidental. The Verus
migration replaced a nanomsg pub-sub layer that ran in roughly the
same architectural slot as today's RPC layer, and keeping the
boundaries clean means the same swap could be made again — for a
different chain, for a different identity-backed key-value store, or
for a mock during testing — without touching the poker code.

```
┌─────────────────────────────────────────────────────────────┐
│  Application Layer                                          │
│  dealer.c, player.c, cashier.c, game.c, blinder.c           │
│                                                             │
│  Example calls:                                             │
│    poker_get_key_json(table_id, T_TABLE_INFO_KEY, 0)        │
│    poker_join_table()                                       │
│    poker_list_dealers()                                     │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  Poker API Layer (poker_vdxf.c)                             │
│  Poker-specific operations with key parsing                 │
│                                                             │
│  Functions:                                                 │
│    poker_get_key_str()      - Read string from a poker key  │
│    poker_get_key_json()     - Read JSON from a poker key    │
│    poker_append_key_json()  - Append data under a poker key │
│    poker_update_key_json()  - Replace a poker key's value   │
│    poker_join_table()       - Join a poker table            │
│    poker_list_dealers()     - List registered dealers       │
│    poker_is_table_full()    - Check table capacity          │
│    poker_print_table_keys() - Inspect a table identity      │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  Generic VDXF Layer (vdxf.c)                                │
│  Verus ID and ContentMultiMap operations                    │
│                                                             │
│  Functions:                                                 │
│    get_cmm()                       - Read identity CMM      │
│    update_cmm()                    - Replace identity CMM   │
│    get_cmm_from_height()           - History via            │
│                                      getidentitycontent     │
│    append_cmm_from_id_key_data_*() - Read-modify-write a    │
│                                      single key             │
│    update_cmm_from_id_key_data_*() - Overwrite a single key │
│    update_with_retry()             - Wraps updateidentity   │
│                                      with block/UTXO        │
│                                      settle-time handling   │
│    is_id_exists() / id_cansignfor()/ id_canspendfor()       │
│    get_vdxf_id()                   - Hash a key name to its │
│                                      vdxfid                 │
│    verus_sendcurrency_data()       - Submit a sendcurrency  │
│                                      transaction            │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  RPC Abstraction Layer (commands.c)                         │
│  Unified interface for blockchain RPC calls                 │
│                                                             │
│  Function:                                                  │
│    chips_rpc(method, params, &result)                       │
│                                                             │
│  Internally selects transport based on configuration:       │
│    if (rpc_config.use_rest_api)                             │
│      → HTTP POST via libcurl                                │
│    else                                                     │
│      → CLI execution via popen                              │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  Transport Layer                                            │
│                                                             │
│  REST API (libcurl):                                        │
│    HTTP POST to rpc_config.url (default                     │
│    http://127.0.0.1:22778)                                  │
│    JSON-RPC payload: {"method":"getidentity","params":[]}   │
│                                                             │
│  CLI (popen):                                               │
│    verus -chain=VRSCTEST getidentity "id@" -1               │
└─────────────────────────────────────────────────────────────┘
```

---

## Configuration

The application and the layers below it read three configuration
files at startup. They are split deliberately so the chain and the
identity layout can be retargeted independently.

### Chain and CLI binary

`poker/config/blockchain.ini` selects which chain and which CLI
binary `bet` talks to. The local VRSCTEST regtest used for
development:

```ini
[blockchain]
blockchain_cli = "verus -chain=VRSCTEST"
currency       = VRSCTEST
new_block      = "Y"
```

`blockchain_cli` is the literal command prefix used by the CLI
transport path; `currency` is the asset name passed to `sendcurrency`
for payins and payouts.

### Identities and keys

`poker/config/keys.ini` declares the identity names and the CMM key
prefix the application will read and write. On the dev regtest:

```ini
[identities]
parent_id  = sg777z.VRSCTEST@
cashier_id = cashier.sg777z.VRSCTEST@
dealers_id = dealers.sg777z.VRSCTEST@
```

The VDXF key namespace is **not** configured in `keys.ini`. The
prefix (`chips.vrsc::poker.`) and every key name are defined
as compile-time macros in `poker/include/vdxf.h` — every `*_KEY`
macro composes the single `VDXF_POKER_KEYS_PREFIX` macro with a
fixed suffix. The full readable string is hashed to a vdxfid by
`get_vdxf_id` at runtime. The prefix is opaque to Verus and does
not have to match the chain name.

### RPC credentials

When REST transport is enabled, `chips_rpc` reads HTTP credentials
from `poker/.rpccredentials`. The file is gitignored; a template
lives at `poker/.rpccredentials.example`:

```ini
[rpc]
rpc_url      = http://127.0.0.1:22778
rpc_user     = your_rpc_user
rpc_password = your_rpc_password
use_rest_api = yes
```

The default in code (`config.c:771`) is `use_rest_api = 0` — meaning
the CLI transport path is used unless this file explicitly turns REST
on. REST is faster (one TCP connection vs. one process fork per call)
but requires the credentials file to be present and correctly
populated; CLI works out of the box but pays a process-fork per RPC.

---

## Key files

| File | Purpose |
|------|---------|
| `poker/src/poker_vdxf.c` | Poker-specific API layer |
| `poker/include/poker_vdxf.h` | Poker API declarations |
| `poker/src/vdxf.c` | Generic VDXF operations and `update_with_retry` |
| `poker/include/vdxf.h` | VDXF declarations and key-name macros |
| `poker/src/commands.c` | RPC abstraction (`chips_rpc`) and CLI/HTTP routing |
| `poker/src/config.c` | Configuration parsing for all three .ini files |

---

## How a call descends

```c
#include "poker_vdxf.h"

// Application code in dealer.c:
cJSON *table_info = poker_get_key_json(table_id, T_TABLE_INFO_KEY, 0);
```

The call descends as follows:

1. `poker_get_key_json` in `poker_vdxf.c` forwards the full key
   name (`T_TABLE_INFO_KEY`, already prefixed via the
   `VDXF_POKER_KEYS_PREFIX` macro in `vdxf.h`) to
   `get_cJSON_from_id_key` in `vdxf.c`.
2. `get_cJSON_from_id_key` resolves the identity, decides which
   read variant to use (current vs. historical), and calls
   `chips_rpc("getidentity", params, &result)`.
3. `chips_rpc` in `commands.c` consults `rpc_config.use_rest_api`:
   - If true, it serializes the call as JSON-RPC and POSTs it to
     `rpc_config.url` via libcurl, parses the response body.
   - If false, it builds an argv from `blockchain_cli + method +
     params`, runs it via `popen`, captures stdout, and parses the
     JSON output.
4. The parsed `result` flows back up through the layers. Nothing
   above the RPC layer ever sees the transport choice.

The same pattern holds for the write path. `update_with_retry` in
`vdxf.c` is the choke point for every `updateidentity` call — it
issues the RPC, waits for the resulting transaction to confirm via
`wait_for_a_blocktime` plus `check_if_tx_exists`, and surfaces the
final txid (or an error) to its caller. Single-writer-per-identity
(see `verus-overview.md`) means there's no contention to worry about
above this layer — `update_with_retry` is the only place that has to
know what "settled on-chain" means.

---

## Why the layering matters

The layered design is what made the original nanomsg → Verus
migration tractable. The application code in `dealer.c`/`player.c`
never had to learn about the new transport; it kept calling the same
poker-layer functions, and the bottom three layers were rewritten
under them. The same property holds for any future replatforming:
swapping CLI for REST is a one-line config change, swapping REST for
a different RPC dialect is a `commands.c`-only change, and swapping
Verus for a different identity-backed key-value store would touch
only `vdxf.c`.

---

## Related documentation

- [Verus Overview](verus-overview.md) — how `bet` uses Verus identities
- [IDs, Keys and Data](../reference/vdxf-keys.md) — VDXF key reference
- [Game State](../reference/game-states.md) — state machine across the layered stack
- [getidentitycontent](getidentitycontent.md) — history-aware reads
- [CLI print commands](../reference/cli-print.md) — inspecting on-chain state from
  the command line
