# Player Configuration

A player node reads four config files (same set as the dealer — see
[Dealer Configuration § 0](dealer-config.md)). What
differs is which keys it actually consults from each, and that the
`player.ini` filename is typically replaced by a per-seat variant
(`p1.ini`, `p2.ini`, …) so multiple players can share a host:

```bash
./bin/bet player -c poker/config/p1.ini
./bin/bet player -c poker/config/p2.ini
```

When `-c` is not passed, the default is `poker/config/player.ini`
(see `bet_init_config_paths` in `poker/src/config.c`).

---

## 1. `player.ini` / `pN.ini`

The shipped example is intentionally minimal:

```ini
[verus]
dealer_id   = d1       # Dealer to join
table_id    = t1       # Table to join
wallet_addr = *        # Wallet address to spend from ("*" = any)
player_id   = p1       # Player's Verus ID (must be signable)
ws_port     = 9001     # WebSocket port for GUI mode
```

### `[verus]`

| Key           | Default | Meaning                                                                                          |
|---------------|---------|--------------------------------------------------------------------------------------------------|
| `dealer_id`   | —       | Short name of the dealer identity to join (`d1` → `d1.<parent>@`).                               |
| `table_id`    | —       | Short name of the table identity to sit at (`t1` → `t1.<parent>@`).                              |
| `player_id`   | —       | Short name of the player's own identity (`p1` → `p1.<parent>@`). The node must hold a private key authorized to sign updates for this identity. |
| `wallet_addr` | `*`     | R-address (or `*`) used to fund the payin. `*` lets the wallet pick a UTXO automatically.        |
| `ws_port`     | `9001`  | TCP port for the GUI WebSocket. The example pN.ini files use 9001/9002 to keep two players non-overlapping on the same host. |

### `[player]` (all optional)

The parser (`bet_parse_player_config_ini_file` in `config.c`)
recognises these fields:

| Key                          | Units              | Default | Meaning                                                                                                 |
|------------------------------|--------------------|---------|---------------------------------------------------------------------------------------------------------|
| `max_allowed_dcv_commission` | %                  | `2`     | If the dealer advertises a higher cut, the player aborts the join. Only consulted in the nanomsg-era `bet_player_stack_info` path (`client.c:1549`), which is not wired in the current Verus build. |
| `table_stake_size`           | BB (integer)       | `20`    | Stake to post on join, expressed in big-blinds. Clamped server-side to `[min_stake, max_stake]` from the dealer's `dealer.ini`. |
| `name`                       | string             | empty   | Free-form display name surfaced in `walletInfo`/`seats`.                                                |
| `gui_ws_port`                | TCP port (integer) | `9001`  | Alternative name for `ws_port`. The two are parsed by separate code paths; keep one in any given file.   |

### `[private table]` (all optional, dormant)

| Key                 | Default | Meaning                                                                                                  |
|---------------------|---------|----------------------------------------------------------------------------------------------------------|
| `is_table_private`  | `0`     | If `1`, the player will only attempt to join private tables and supply `table_password`.                 |
| `table_password`    | empty   | Shared secret previously distributed off-chain.                                                          |

Same caveat as for the dealer side: these fields are parsed and
stored but the join handshake that consumed them
(`bet_player_stack_info`) goes through the disabled nanomsg loop and
has no effect today.

### `[gui]` (legacy)

Older `player.ini` files used to enumerate cashier-hosted GUIs:

```ini
[gui]
cashier-1 = http://141.94.227.65:1234/
cashier-2 = http://141.94.227.66:1234/
```

The parser still scans these (`bet_display_cashier_hosted_gui` in
`config.c`) but the values are only printed as warnings — the player
node no longer drives the user to a remote GUI URL. In the current
build the GUI is hosted on the player node itself at
`http://localhost:<ws_port>/`.

---

## 2. Other files

* **`blockchain.ini`** — same content as on a dealer node;
  `blockchain_cli`, `currency`, `new_block`. See
  [Dealer Configuration § 2](dealer-config.md).
* **`keys.ini`** — same content as on a dealer node;
  `parent_id` and the `*_short` identity names. Must match the
  dealer's `keys.ini` (otherwise the two address different
  identities and cannot see each other's CMM updates).
* **`.rpccredentials`** — RPC username/password for the player's
  local Verus daemon. Required for `verus`-CLI invocations.

---

## 3. Becoming a player

Joining is permissionless: any Verus identity under the parent
namespace can be used to sit at a table that has open seats. The
operational steps are:

1. Provision a Verus wallet with enough funds in the chain's native
   currency to cover the payin plus tx fees.
2. Register a player identity (`registernamecommitment` +
   `registeridentity`) under the parent — see
   [`revoke-recovery.md`](../tutorials/revoke-recovery.md) for
   the registration flow.
3. Configure `dealer_id`, `table_id`, and `player_id` in your
   `pN.ini` to match what the dealer is hosting.
4. Start the player node:

   ```bash
   ./bin/bet player -c poker/config/p1.ini
   ```

   In GUI mode, open `http://localhost:<ws_port>/` and follow the
   join flow described in
   [`PLAYER_JOIN_FLOW.md`](player-join-flow.md).

The dealer-side admission decision is the on-chain seat
reservation, not the dealer's local config: writing your join intent
to the table identity's CMM is the equivalent of a "request to join",
and the dealer either commits a `payin_tx` claim back to your
identity or rejects.
