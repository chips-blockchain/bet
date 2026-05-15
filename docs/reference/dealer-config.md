# Dealer Configuration

The dealer node reads its configuration from four INI files in
`poker/config/`:

| File              | Purpose                                                       |
|-------------------|---------------------------------------------------------------|
| `dealer.ini`      | Per-table game parameters and dealer-level knobs.             |
| `blockchain.ini`  | Verus CLI invocation, currency name, new-block polling flag.  |
| `keys.ini`        | Identity names and VDXF key prefix used to derive CMM keys.   |
| `.rpccredentials` | Username/password for the Verus daemon's RPC (kept out of `config/` because it holds secrets). |

A non-default location for any file can be passed on the command line
with `-c <path>` when starting `bet`. If unset, `bet` resolves paths
relative to its own executable: `<dir-of-bet>/config/*.ini` (or
`<dir-of-bet>/../config/*.ini` when run from `bin/`). See
`bet_init_config_paths` in `poker/src/config.c`.

---

## 1. `dealer.ini`

The shipped example covers only the keys a dev regtest setup needs:

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

All identity fields must be fully-qualified Verus IDs (containing
`@`); the parser rejects bare short names with `ERR_INI_PARSING`.

The parser (`bet_parse_dealer_config_ini_file` + the per-table reader
in `config.c`) also accepts the following optional sections, all of
which fall back to compiled-in defaults when omitted:

### `[table]`

| Key            | Units               | Default      | Meaning                                       |
|----------------|---------------------|--------------|-----------------------------------------------|
| `max_players`  | integer             | 2 (CLI flag) | Seats at the table. Hand starts once full.    |
| `big_blind`    | CHIPS (double)      | 0.02         | Stored internally as `BB_in_table_chips`; `small_blind = big_blind / 2`. |
| `min_stake`    | CHIPS (double)      | —            | Minimum payin a player can post.              |
| `max_stake`    | CHIPS (double)      | —            | Maximum payin (cap on stack size).            |
| `table_id`    | string              | —            | Fully-qualified Verus ID of the per-table identity this dealer manages, e.g. `t1.sg777z.VRSCTEST@`. |

### `[verus]`

| Key          | Default | Meaning                                          |
|--------------|---------|--------------------------------------------------|
| `dealer_id`  | —       | Operational dealer identity, fully-qualified (e.g. `d1.sg777z.VRSCTEST@`). |
| `cashier_id` | —       | Operational cashier identity, fully-qualified (e.g. `cashier.sg777z.VRSCTEST@`). Defaults to `bet_get_cashiers_id_fqn()` when omitted. |

### `[dealer]` (all optional)

| Key             | Units                        | Default                    | Meaning                                                                                |
|-----------------|------------------------------|----------------------------|----------------------------------------------------------------------------------------|
| `chips_tx_fee`  | CHIPS (double)               | `0.0001`                   | Fee the dealer attaches to its on-chain sends. Must be ≥ network minimum.              |
| `dcv_commission`| % of winning pot             | `0.75`                     | Dealer's cut applied at settlement (`host.c:960`).                                     |
| `gui_host`      | URL                          | `http://<dealer_ip>:1234/` | Where the dealer advertises its GUI when peer nodes ask. Filled in automatically if blank. |
| `gui_ws_port`   | TCP port                     | `9000`                     | Local WebSocket port for the dealer's GUI/inspection front-end.                        |
| `min_cashiers`  | integer                      | (compile-time `threshold_value`) | Minimum cashier multisig threshold this dealer requires. Carried over from the legacy multi-cashier design and is not exercised by the current single-cashier setup. |

### `[private table]` (all optional, dormant)

| Key                 | Default | Meaning                                                                |
|---------------------|---------|------------------------------------------------------------------------|
| `is_table_private`  | `0`     | If `1`, table is gated by `table_password`.                            |
| `table_password`    | empty   | Shared secret distributed offline to whitelisted players.              |

These settings are parsed and stored in `is_table_private` /
`table_password` but only consumed by `bet_player_stack_info` in the
old nanomsg join handshake (`client.c:1707`). That path is not
wired in the current Verus build, so the values have no runtime
effect today.

---

## 2. `blockchain.ini`

```ini
[blockchain]
blockchain_cli = "verus -chain=VRSCTEST"
currency       = VRSCTEST
new_block      = "Y"
```

| Key              | Units | Meaning                                                                                                                  |
|------------------|-------|--------------------------------------------------------------------------------------------------------------------------|
| `blockchain_cli` | string| Command (or full path) used to invoke the Verus CLI. Quoted so a `-chain=...` argument can be included.                  |
| `currency`       | string| Native currency name used when constructing `sendcurrency` payloads. Defaults to `CHIPS` if absent.                      |
| `new_block`      | `Y/N` | If `Y`, the cashier (when colocated with the dealer) processes pending payin requests on every new block notification.   |

---

## 3. `keys.ini`

Identity names and the VDXF namespace prefix for all CMM keys. The
example below matches the dev regtest layout:

```ini
[identities]
cashier_id = cashier.sg777z.VRSCTEST@   # aggregator
dealers_id = dealers.sg777z.VRSCTEST@   # aggregator
```

Both fields must be fully-qualified Verus IDs; the parser rejects
bare short names. There is no longer a `parent_id` field — the parent
is implicit in each FQN.

Note the distinction between **aggregator** identities (`cashier_id`,
`dealers_id` — hold the on-chain discovery lists) and the
**operational** identities (`d1.sg777z.VRSCTEST@`,
`cashier.sg777z.VRSCTEST@`) configured in `dealer.ini`. See
[`id_creation_process.md`](../explanation/identity-tree.md)
for the full identity taxonomy.

VDXF key names are not configured in `keys.ini`. The prefix
(`chips.vrsc::poker.`) and every key name are compile-time
macros in `poker/include/vdxf.h`; the prefix lives once as
`VDXF_POKER_KEYS_PREFIX` and every `*_KEY` macro composes it with a
fixed suffix.

---

## 4. `.rpccredentials`

Plain key/value file with the Verus daemon's RPC username and
password. Read by `rpc_credentials_file` resolution in
`bet_init_config_paths`. Format:

```
rpcuser=...
rpcpassword=...
```

`bet` shells out to `verus` for almost every on-chain operation; it
does not connect to the daemon's RPC port directly today.

---

## 5. Becoming a dealer

There is no on-chain admission control on becoming a dealer at the
protocol level. To bring a new dealer online:

1. Provision a Verus wallet on the chain you intend to host on
   (VRSCTEST for development).
2. Register the operational dealer identity (`./bet register_dealer`)
   under the dev/parent VerusID and any per-table identities you want
   to host. See
   [`id_creation_process.md`](../explanation/identity-tree.md).
3. Update `dealer.ini` with the new identity FQNs and table
   parameters.
4. Start the dealer node and confirm it can sign updates against the
   parent aggregator.

Inviting it into a multi-dealer deployment is a social/operational
decision, not a protocol-level one — the discovery list lives in the
`dealers.<parent>@` aggregator's CMM and anyone who can write that
identity can extend it.
