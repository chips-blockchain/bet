# Code change suggestions surfaced during doc rewrites

This is the running list of code changes the doc-rewrite work has identified.
The rule for adding an item here is **"important enough to change in the
code"** — not "any inconsistency or wart noticed in passing". If a doc can
faithfully describe the current behaviour and a community reader won't be
misled by it, no entry goes here.

Items are kept in the order they were found, with the doc-rewrite turn that
surfaced them noted. Nothing here is acted on without explicit approval.

---

## During rewrite of `docs/reference/cli-print.md` → `cli-print.md` (May 11, 2026)

### 1. `bet_help_print_id_command_usage` stale ID-type list — RESOLVED (May 15, 2026)

The help string was rewritten in `poker/src/help.c` to require fully-qualified
IDs and to list the actual supported types (`table`/`t`, `dealer`/`d`,
`player`/`p`, `dealers`, `cashier`/`c`). The `cashier` alias was singularised
from the historical `cashiers` to align with the fact that the printer
handles operational cashier identities, not the (currently nonexistent)
cashier aggregator.

---

## During rewrite of `docs/explanation/identity-tree.md` (May 11, 2026)

### 2. Stale `verus_dealer.ini` filename in dealer config error string

**Where:** `poker/src/err.c:170`

**What it says now:**

```c
return "Error in dealer config args, check if all the necessary args are confiured in verus_dealer.ini";
```

**What the code actually reads** (`poker/src/config.c:32, 86`): the file is
`dealer.ini`, located under `<config_base>/dealer.ini`. The `verus_dealer_config`
*variable name* in `config.c` is a leftover but the *path string* it
holds is `<config_base>/dealer.ini`. There is no `verus_dealer.ini`.

**Why this matters:** when a user runs `./bet start dealer --config
config/dealer.ini` and forgets a required arg, the error message tells
them to check `verus_dealer.ini` — a file that doesn't exist. They
spend time looking for a file they don't have, instead of fixing the
file they do have.

**Suggested fix:** replace the literal `verus_dealer.ini` with
`dealer.ini` in `err.c:170` (and the matching place in
`err.c:168` for `verus_player.ini` → `player.ini` if that one is also
stale; needs a one-line check). The `verus_dealer_config` symbol in
`config.c` is internal and can keep its name without affecting the
user-facing string.

**Priority:** low. One-line, no test impact, but worth grabbing the
next time anyone touches `err.c`.

---

### 3. `T_GAME_INFO_KEY` docstring describes wrong fields

**Where:** `poker/include/vdxf.h:249-253`

**What it says now:**

```c
/*
* t_game_info {
* t_game_ids : 256 bit unique string in hex
* game_info : Holds the info of the gaming state
* }
*/
#define T_GAME_INFO_KEY "chips.vrsc::poker.t_game_info"
```

**What the code actually writes/reads** (`poker/src/game.c:88` — `get_game_state`, and `append_game_state`): the payload is `{ "game_state": <int>, "game_state_info": <optional> }`. The fields named in the docstring (`t_game_ids`, `game_info`) do not exist in the actual JSON.

**Why this matters:** any contributor reading `vdxf.h` to understand
the key family will write code against the documented shape and break.
A doc that's wrong is worse than a doc that's missing.

**Suggested fix:** replace the docstring with the actual shape:

```c
/*
 * t_game_info — written per-game on the table identity (and on the
 * cashier identity for the G_DECK_SHUFFLING_B and
 * G_SETTLEMENT_COMPLETE_BY_CASHIER cashier-side mirrors). Key is
 * suffixed by game_id at write time.
 *
 * {
 *   "game_state":      <int from enum G_*>,
 *   "game_state_info": <optional metadata for the current state>
 * }
 */
```

**Priority:** low. Internal docstring, not user-facing. One-line edit
in `vdxf.h` whenever someone is in the file.

---

## During rewrite of `docs/guides/gui-simulator.md` (May 11, 2026)

### 4. `tools/gui_simulator.js` sends an unsupported betting-reply shape

**Where:** `tools/gui_simulator.js:164-172`

**What it sends now:**

```js
const response = {
  method: 'betting',
  playerid: parsed.playerid || 0,
  round: parsed.round || 0,
  min_amount: parsed.toCall || 0,
  possibilities: parsed.possibilities || [],   // echoes full allowed-actions array
  action: action,                              // string e.g. "call"
  bet_amount: betAmount
};
```

**What the backend accepts** (`poker/src/states.c:697-708`,
`bet_player_round_betting`):

```c
possibilities = cJSON_GetObjectItem(argjson, "possibilities");
if (!possibilities || cJSON_GetArraySize(possibilities) != 1) {
    dlg_error("[GUI] betting message has no chosen action in possibilities[]");
    return ERR_ARGS_NULL;
}
action_code = jinti(possibilities, 0);   // integer action code from enum action_type
```

i.e. `possibilities` must be a **single-element array** carrying the
*chosen* action code (integer from `enum action_type` in `bet.h`).
The string `action` field is ignored; `round` is re-read from
`T_BETTING_STATE_KEY` and not trusted from the message; `min_amount`
is unused.

**Why this matters:** the simulator's betting reply is rejected
upstream of any state change, so the player auto-folds after the
121-second timeout instead of acting. The simulator looks like it's
working (it logs `🤖 Auto-Response: CALL`) but the round never
progresses.

**Suggested fix:** map the string action to the corresponding
backend `enum action_type` integer (1=small_blind, 2=big_blind,
3=check, 4=raise, 5=call, 6=allin, 7=fold) and emit:

```js
const ACTION_CODE = { small_blind:1, big_blind:2, check:3, raise:4, call:5, allin:6, fold:7 };
const response = {
  method: 'betting',
  playerid: parsed.playerid || 0,
  toCall: parsed.toCall || 0,
  bet_amount: betAmount,
  possibilities: [ ACTION_CODE[action] ]
};
```

**Priority:** medium. The simulator is a dev tool, but it's
referenced from the testing docs as the path of least resistance,
and "fails silently and waits for a timeout" is a worse failure
mode than "throws a parse error".

---

## Items considered but not added

These are things the audit surfaced where I deliberately chose to describe
the wart in the rewritten doc rather than recommend a code change.

- **`print_vdxf_info` suffix inconsistency** (`print.c:212-237`).
  `T_GAME_INFO_KEY` is auto-suffixed with the current `game_id` before being
  read; `T_TABLE_INFO_KEY` / `T_PLAYER_INFO_KEY` are not. The non-uniformity
  is real but is not actually broken: `T_TABLE_INFO_KEY` on a dealer id is
  unsuffixed by design (it's the table template), and on a table id the
  game-suffixed read is what `print_id table` does instead. So `./bet print`
  works correctly for the narrow surface it claims to support. Documented in
  `cli-print.md`; no code change recommended.
