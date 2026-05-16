# Documentation Inventory — migration worksheet

> **Status (May 11, 2026):** Phase 1 (audit) ✓, Phase 3 (content rewrite) ✓,
> Phase 2 (file moves into Diátaxis layout) ✓. The user-approved decisions
> recorded under "Open decisions" below were:
>
> 1. **Layout:** `docs/{tutorials,guides,reference,explanation,archive}/`
>    (Diátaxis with `guides/` instead of `how-to/`).
> 2. **CHANGELOG:** deferred — no `CHANGELOG.md` for now; archive entries
>    kept verbatim under `docs/archive/`.
> 3. **Namespaces:** `docs/protocol/` and `docs/verus_migration/` were
>    **collapsed** — neither survives as a subdirectory. All content
>    redistributed to the Diátaxis buckets or `docs/archive/docs/protocol/`
>    / `docs/archive/docs/verus_migration/`.
> 4. **`predictable_betting.md`:** **deleted** — not a poker doc.
>
> The "Move target (Phase 2)" columns below are now the *delivered* layout.
> All cross-document links were updated. Both `docs/protocol/` and
> `docs/verus_migration/` were removed once empty.
>
> This worksheet stays in the tree as the decision record. It is not part
> of the published documentation index.

This worksheet is the **decision record** for the documentation reorganization
plan (see chat thread starting May 10, 2026). Every Markdown file currently in
the `bet` repo is tagged here with one of the four actions below, plus a short
rationale.

> Scope: `README.md` + everything under `docs/**` + `poker/docs/**`. We
> deliberately exclude `external/`, `.cursor/`, `agent-transcripts/`,
> `node_modules/`, generated artifacts, and the in-tree `.pdf` / `Doxyfile` /
> `install.sh` (called out at the bottom).

---

## Tag legend

| Tag           | Meaning                                                                                          | Phase 3 effort |
|---------------|--------------------------------------------------------------------------------------------------|----------------|
| `KEEP-AS-IS`  | Content is current and correct. Just move to the new layout.                                     | none           |
| `REWRITE`     | Topic is still in scope, content is stale/inaccurate. Replace body, keep the topic.              | medium-large   |
| `ARCHIVE`     | Historical value only (old design, removed feature, dated milestone). Move to `docs/archive/`.   | none           |
| `DELETE`      | Empty file, self-declared obsolete, or fully superseded by another file we are keeping.          | none           |

`ARCHIVE` means we **don't lose history** — the file stays in the repo under
`docs/archive/<original-path>` with a one-line banner pointing at the current
equivalent. `DELETE` only applies to zero-content files or files that are dead
weight (e.g., duplicates, "obsolete" stamped on line 2).

---

## Summary

| Bucket                            | KEEP-AS-IS | REWRITE | ARCHIVE | DELETE | Total |
|-----------------------------------|-----------:|--------:|--------:|-------:|------:|
| Root `*.md`                       |          0 |       1 |      10 |      0 |    11 |
| `docs/*.md`                       |          3 |       4 |       4 |      0 |    11 |
| `docs/protocol/*.md`              |          2 |      10 |      19 |      1 |    32 |
| `docs/verus_migration/*.md`       |          5 |       6 |       1 |      4 |    16 |
| `poker/docs/*.md`                 |          0 |       1 |       0 |      0 |     1 |
| **Total**                         |     **10** |  **22** |  **34** |  **5** | **71**|

Headline: **only ~14 % of the existing docs (10/71) are current as-written**.
The bulk of the work in Phase 3 is rewriting 22 files, and the bulk of the
*win* in Phase 1/2 is archiving 34 files so a community reader is not
immediately confused by stale LN / socket / CHIPS-PBaaS content.

### Phase 3 progress snapshot (May 11)

| Bucket                       | REWRITE planned | REWRITTEN | DEPRECATED-IN-PLACE | DELETED | Outstanding |
|------------------------------|----------------:|----------:|--------------------:|--------:|------------:|
| Root `*.md`                  |               1 |         1 |                   0 |       0 |       **0** |
| `docs/*.md`                  |               4 |         4 |                   0 |       0 |       **0** |
| `docs/protocol/*.md`         |              10 |         6 |                   4 |       0 |       **0** |
| `docs/verus_migration/*.md`  |               6 |         6 |                   0 |       0 |       **0** |
| `poker/docs/*.md`            |               1 |         0 |                   1 |       0 |       **0** |
| **Total**                    |          **22** |    **17** |               **5** |   **0** |       **0** |

Plus four stub files deleted (`cashier_setup.md`, `dealer_rejoining.md`,
`dispute_resolution.md`, `permutation.md`, `player.md`) and the four
salvage-via-redirect cases captured under DEPRECATED-IN-PLACE
(`release.md`, `setup_comon_errors.md`, `tx_flow.md`, `ubuntu_compile.md`,
`schema.md`, `player_joining.md`, `poker/docs/GUI_PROTOCOL.md`,
`dealer_setup.md`, `player_setup.md`).

Phase 3 content work is **complete**. Remaining doc-overhaul work is all
Phase 2 (file moves) and depends on the open decisions below.

### KEEP-AS-IS audit (May 11)

Re-checked the three `docs/*.md` files originally tagged `KEEP-AS-IS`
against current code. (Background: the original Phase-1 classification was
on a 0-for-5 streak for `KEEP-AS-IS` accuracy, so this re-audit was needed.)

| File                            | Verdict on May 11 | Notes                                                                                  |
|---------------------------------|------------------|----------------------------------------------------------------------------------------|
| `docs/TODO.md`                  | KEEP-AS-IS ✓     | All four backlog items reference live code paths with file:line refs; no staleness.    |
| `docs/reference/player-join-flow.md`      | KEEP-AS-IS ✓     | Cross-checked §1.1–§1.4 against `vdxf.c:join_table`, `poker_vdxf.c`, `blinder.c`. Accurate. |
| `docs/tutorials/cli-auto-vrsctest.md`  | KEEP-AS-IS ✓     | Chain config, ID list, CLI-auto bring-up commands match the running setup; §10 matches `bet.c` startup flags. |

No content changes needed in any of the three.

---

## Root-level Markdown (`/`)

These are the "scratchpad" notes that accumulated at the repo root. Every one
of them documents a past one-off change or an old GUI flow — none of them
belong at the top level of a community-facing repo.

| File                                | Tag       | Rationale                                                                                                         | Move target (Phase 2)          |
|-------------------------------------|-----------|-------------------------------------------------------------------------------------------------------------------|--------------------------------|
| `README.md`                         | REWRITTEN | Replaced CHIPS/Lightning-era front door with a Verus-only intro: node-type table, quick-start using the modern `./bin/bet start <role>` form, single-writer architecture paragraph, GUI WebSocket port table, links to the rewritten doc set. Removed the stale `chipsd`/`chips-cli` invocations. | `README.md` (in place)         |
| `FIND_TABLE_BUTTON_FLOW.md`         | ARCHIVE   | One-off GUI flow note describing a "Find Table" button. Concrete feature is in `pangea-poker`; no value to a backend reader. | `docs/archive/`                |
| `GUI_FLOW_DETAILED.md`              | ARCHIVE   | Step-by-step click trace of "Set Nodes" → WebSocket. Paths point at `/root/pangea-poker/...` (machine-specific). Superseded by `gui_e2e_test_plan_8a4b327f.plan.md`. | `docs/archive/`                |
| `GUI_TESTING_GUIDE.md`              | ARCHIVE   | "Join approval flow" GUI testing — describes a mechanism that was **removed** (see `REMOVED_GUI_JOIN_APPROVAL.md`). | `docs/archive/`                |
| `GUI_WAIT_MECHANISM.md`             | ARCHIVE   | Same removed feature (`gui_join_mutex`/`gui_join_cond`). Documents code that no longer exists.                    | `docs/archive/`                |
| `LIVE_TESTING_RESULTS.md`           | ARCHIVE   | Snapshot of `gui_simulator.js` test results. Historical milestone, not reference material.                        | `docs/archive/`                |
| `LIVE_TESTING_SUMMARY.md`           | ARCHIVE   | Same simulator milestone, summary form.                                                                           | `docs/archive/`                |
| `PLAYER_INIT_STATE_MACHINE.md`      | ARCHIVE   | Documents `P_INIT_WAIT_JOIN` for the removed GUI join-approval mechanism. State machine still partially exists; the **document** is stale. | `docs/archive/`                |
| `REFACTORING_SAMPLES.md`            | ARCHIVE   | Before/after C snippets (cashier.c strcpy etc.). Code-review artifact, not user-facing.                           | `docs/archive/`                |
| `REMOVED_GUI_JOIN_APPROVAL.md`      | ARCHIVE   | Changelog-style note about removing the GUI join approval flow. Belongs in a CHANGELOG entry, not at the root.    | `docs/archive/`                |
| `SEAT_SELECTION_IMPLEMENTATION.md`  | ARCHIVE   | Describes an intermediate version of seat selection. Current behaviour is auto-seat in CLI auto and GUI-driven seat pick in GUI mode. | `docs/archive/`                |
| `VERUS_TEST_BRANCH_SUMMARY.md`      | ARCHIVE   | "What's in the `verus_test` branch" summary — a release-notes draft for the branch we're now on. Salvage one paragraph into a CHANGELOG entry, archive the rest. | `docs/archive/` + CHANGELOG    |

> After Phase 2 the repo root contains exactly one Markdown file: `README.md`.

---

## `docs/*.md`

| File                                       | Tag        | Rationale                                                                                                                              | Move target (Phase 2)                  |
|--------------------------------------------|------------|----------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------|
| `docs/README.md`                           | REWRITTEN  | Updated to point at the rewritten content set: dropped the broken `bet-api.md` link, redirected the stale `tx_flow.md`/`release.md`/`setup_comon_errors.md` entries to their modern equivalents (`tx_types.md`, `compile.md`, `VRSCTEST_LOCAL_SETUP.md`/`COMMUNITY_TESTING_GUIDE.md`), rewrote the Documentation Structure block to reflect the actual file set, and added a "Daemon Compatibility Surface" pointer for the new `upgrade.md`. | `docs/README.md` (in place)            |
| `docs/TODO.md`                             | KEEP-AS-IS | Current "deferred work" backlog (May 10, 2026). Items 2/3/4/5 all reference live code. Useful as `docs/CONTRIBUTING/backlog.md`.       | `docs/contributing/backlog.md` (rename)|
| `docs/tutorials/cli-auto-vrsctest.md`             | KEEP-AS-IS | **Canonical** CLI-auto / VRSCTEST regtest setup guide. Includes the agent operating rules in §0 that the tmux workflow relies on. Largest single source of truth right now. | `docs/tutorials/cli-auto-vrsctest.md`  |
| `docs/reference/player-join-flow.md`                 | KEEP-AS-IS | Current player-join flow (single-writer cashier, payin tx, t_player_info). May 10, 2026. Matches code.                                 | `docs/reference/player-join-flow.md`   |
| `docs/tutorials/community-quickstart.md`          | REWRITTEN  | Replaced the CHIPS-PBaaS guide with a VRSCTEST-regtest version: correct CLI form (`./bin/bet start <role> --config ... --cli/--auto`), correct config filenames (`dealer.ini`/`cashier.ini`/`pN.ini` not `verus_dealer.ini`), 52-card deck, 120 s + 12 blocks timeout, no LN/chipsd, payin via `sendcurrency`. Added a "differences from legacy guide" section. | `docs/tutorials/community-quickstart.md`     |
| `docs/reference/gui-backend-mapping.md`              | REWRITTEN  | Re-cast as a dev-facing companion to `GUI_MESSAGE_FORMATS.md`: per-file module layout, backend-variable → GUI-field table, where each outbound builder is called from, the two-enum action-code gotcha (backend's `enum action_type` vs the GUI's `Possibilities`), and how to add a new message. Marked Phase 2/3 status as completed. | `docs/reference/gui-backend-mapping.md`      |
| `docs/GUI_INTEGRATION_STATUS.md`           | ARCHIVE    | Dated "Jan 3 2026 — Complete and Tested" status report. Pure history.                                                                  | `docs/archive/`                        |
| `docs/GUI_JOIN_APPROVAL.md`                | ARCHIVE    | Documents the removed approval flow.                                                                                                   | `docs/archive/`                        |
| `docs/GUI_JOIN_FIX.md`                     | ARCHIVE    | Documents the wallet-info fix for the removed approval flow.                                                                           | `docs/archive/`                        |
| `docs/guides/gui-simulator.md`              | REWRITTEN  | Rewrote against the actual `tools/gui_simulator.js` (real paths, real flags, `./bin/bet start player ... --gui` form). Flagged that the simulator's betting reply shape doesn't match the backend (single-element `possibilities` of integer codes) and logged the mismatch in `_code_suggestions.md` item 4. | `docs/how-to/gui-simulator.md`               |
| `docs/MIGRATION_TO_REST_API_ESTIMATION.md` | ARCHIVE    | Pre-decision estimation document for the CHIPS REST API migration that was effectively superseded by the Verus RPC abstraction.        | `docs/archive/`                        |

`docs/CNAME`, `docs/Makefile`, `docs/Unsolicited_PANGEA_WP.pdf` — see "Other
files" at the bottom.

---

## `docs/protocol/*.md`

This directory is where the legacy pre-Verus protocol lived. Most files refer
to **Lightning Network**, **nng pub/sub sockets**, **hardcoded notary IPs
`159.69.23.x`**, or **`chips-cli`** — i.e. they describe the protocol the
codebase no longer implements.

| File                          | Tag        | Rationale                                                                                                                                        | Move target (Phase 2)                       |
|-------------------------------|------------|--------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------|
| `architecture_doc.md`         | ARCHIVE    | LN-era architecture musings (c-lightning + UNIX sockets). Pre-Verus.                                                                              | `docs/archive/`                             |
| `bet-api.md`                  | ARCHIVE    | Self-declares "obsolete now" on line 2. Old `bet-cli` API.                                                                                       | `docs/archive/`                             |
| `BET_Initial_Draft.md`        | ARCHIVE    | Original Pangea/Privatebet whitepaper-style draft (LN payment channels, MofN multisig). Historical interest.                                      | `docs/archive/`                             |
| `bet_without_ln.md`           | ARCHIVE    | About the `bet_ln_config = N` config switch from the LN era. Config no longer exists.                                                            | `docs/archive/`                             |
| `cashier_configuration.md`    | REWRITTEN  | Replaced the multi-cashier election narrative with current single-cashier-identity model: documented the one `[cashier]` knob (`gui_ws_port`), the dormant legacy `node-N` block, how to register & run a cashier, and pointers to multisig/revocation/recovery setup. Inbound link from `docs/README.md` now lands on accurate content. | `docs/reference/cashier-config.md`          |
| `cashier_protocol.md`         | ARCHIVE    | nng socket-based payin flow with `stack_info_req`/`stack_info_resp`. Replaced by Verus-ID + `sendcurrency` (see `PLAYER_JOIN_FLOW.md`).          | `docs/archive/`                             |
| `cashier_setup.md`            | DELETED    | Removed May 11. Was a 0-byte stub; no inbound references.                                                                                        | —                                           |
| `compile.md`                  | REWRITTEN  | Replaced CHIPS+LN+bet build instructions with the current `bet`-only build: Ubuntu/macOS system packages, recursive submodule clone, `make -j`, runtime expectation of the `verus` CLI on `PATH`. Points to `VRSCTEST_LOCAL_SETUP.md` for the chain side. | `docs/how-to/build-from-source.md`          |
| `dealer_configuration.md`     | REWRITTEN  | Fully rewritten against current config layout (`dealer.ini`, `blockchain.ini`, `keys.ini`, `.rpccredentials`) and the actual fields parsed in `config.c::bet_parse_dealer_config_ini_file`. Calls out the dormant `[private table]` and `min_cashiers` fields. | `docs/reference/dealer-config.md`           |
| `dealer_setup.md`             | DEPRECATED-IN-PLACE | Walked May 11 against `dealer.ini`, `config.c::bet_parse_dealer_config_ini_file`, `bet.c::bet_start_dcv`. Old doc described an LN-era flow (`bet_setup.ini`, `./bin/bin/bet dcv <ipv4>`, "Set Nodes" GUI step, multisig `dispute_resolution`). Every concrete step is wrong against current code; the live equivalents are split across `compile.md`, `VRSCTEST_LOCAL_SETUP.md`, `dealer_configuration.md`, and `COMMUNITY_TESTING_GUIDE.md`. **No salvage** — body replaced with a redirect pointing at those four. To be archived once `docs/README.md` confirms no live inbound link. | `docs/archive/` (planned)                   |
| `DigitalOceanServerSetup.md`  | ARCHIVE    | DO droplet setup with firewall rules for ports `7797/7798/7901/7902/9000` — those ports belong to the dead socket protocol.                      | `docs/archive/`                             |
| `finding_dealer.md`           | ARCHIVE    | Cashier-DB-mediated dealer discovery over nng. Replaced by `poker_list_dealers()` reading `dealers` CMM key on the parent Verus ID.              | `docs/archive/`                             |
| `glossary.md`                 | REWRITTEN  | Expanded from 5 lines to a full Verus-era glossary (CMM, vdxfid, primaryaddresses, single-writer-per-identity, T/P/C key prefixes, etc). Stale `cashier_nodes.json` link removed. | `docs/reference/glossary.md`                |
| `GUI_MESSAGE_FORMATS.md`      | REWRITTEN  | Rewrote from code: corrected action codes (was 4=call/5=raise, code has 4=raise/5=call), removed mythical `init_d`/`init`/`turn`/`game_info` outbound messages, documented the actual `gui.c` outbound surface (`walletInfo`, `seats`, `blindsInfo`, `dealer`, `deal`, `betting` round_betting+action+blind variants, `finalInfo` with `showInfo`), the real `betting` request format (`possibilities[0]`, server-side round/amount enforcement), the per-node wallet-method support matrix, and `warning`/`error` codes. | `docs/reference/gui-message-formats.md`     |
| `GUI_QUICK_REFERENCE.md`      | REWRITTEN  | Rewritten May 11 against `poker/include/bet.h:49` (action codes), `poker/include/common.h:140-143` (per-role default ports), and the actual `gui.c` outbound surface. Fixed: action codes (4=raise, 5=call — old doc had them swapped), per-role default ports (9000 dealer / 9001 player / 9002 cashier — old doc had a single port), wallet-method support matrix per role, the real `betting` request shape (`possibilities[0]`, no `amount` field), and removed mythical `init_d`/`turn`/`game_info` from the unsolicited-message list. | `docs/reference/gui-quick-reference.md`     |
| `handling_funds.md`           | ARCHIVE    | Old chips-cli deposit/withdraw flow over WebSocket `walletInfo`. Verus addresses now.                                                            | `docs/archive/`                             |
| `heartbeat_protocol.md`       | ARCHIVE    | nng pub/sub heartbeat (`beacon` over port 7797). Not present in the Verus build.                                                                 | `docs/archive/`                             |
| `HowToPangea.md`              | ARCHIVE    | LN + chipsd quickstart. Replaced by `VRSCTEST_LOCAL_SETUP.md`.                                                                                   | `docs/archive/`                             |
| `messageFormats.md`           | ARCHIVE    | Old GUI message protocol (`method:"game"`, `method:"seats"`). Superseded by `GUI_MESSAGE_FORMATS.md`.                                            | `docs/archive/`                             |
| `method_game.md`              | ARCHIVE    | Sub-doc of the same old GUI protocol.                                                                                                            | `docs/archive/`                             |
| `method_seats.md`             | ARCHIVE    | Same.                                                                                                                                            | `docs/archive/`                             |
| `node_communication.md`       | REWRITTEN  | Replaced nng pub/sub + push/pull description with the CMM-only model: side-by-side legacy-vs-current table, walk-through of a hand purely in `updateidentity` terms, dealer/player/cashier WebSocket ports for GUI. Notes that the nng plumbing is still compiled but unreachable. | `docs/explanation/node-communication.md`    |
| `player_configuration.md`     | REWRITTEN  | Rewritten against `player.ini` / `pN.ini`: documented the actual `[verus]` and `[player]` fields, marked the dormant `max_allowed_dcv_commission` / `[private table]` / `[gui]` legacy paths, and pointed to the live GUI URL (`http://localhost:<ws_port>/`). | `docs/reference/player-config.md`           |
| `player_gui.md`               | ARCHIVE    | Old GUI screenshots/flow ("Private Table → Player → enter IP"). UI evolved.                                                                      | `docs/archive/`                             |
| `player_setup.md`             | DEPRECATED-IN-PLACE | Walked May 11 against `player.ini`, `pN.ini`, `config.c::bet_parse_player_config_ini_file`, `bet.c::bet_start_player`. Old doc described the LN-era `chipsd`+`lightningd` quickstart, a non-existent `player_setup.ini`, and a `[gui]` URL list pointing at cashier-hosted GUIs (no longer the deployment model — each player runs its own GUI). **No salvage** — body replaced with a redirect to `compile.md`, `VRSCTEST_LOCAL_SETUP.md`, `player_configuration.md`, and `COMMUNITY_TESTING_GUIDE.md`. To be archived. | `docs/archive/` (planned)                   |
| `predictable_betting.md`      | KEPT (re-framed) | Standalone design sketch for prediction-market wagers — nothing in here is implemented. Added a status header making this explicit so readers don't mistake it for current behavior. Conceptual content (prorata/odds-based settlement) left intact for community discussion. | `docs/explanation/predictable-betting.md`   |
| `release.md`                  | DEPRECATED-IN-PLACE | No Verus-era binaries are published yet; the chips-blockchain release URLs are gone. Replaced content with a deprecation notice pointing at `compile.md` and the upstream `VerusCoin/VerusCoin` repo. To be archived under `docs/archive/` when the inbound link in `docs/README.md` is also updated. | `docs/archive/` (planned)                   |
| `setup_comon_errors.md`       | DEPRECATED-IN-PLACE | All listed errors are LN/chipsd-specific and don't apply today. Replaced content with a deprecation notice pointing at `VRSCTEST_LOCAL_SETUP.md` and `COMMUNITY_TESTING_GUIDE.md`. To be archived after the `docs/README.md` link is rewritten. | `docs/archive/` (planned)                   |
| `setup_fund_nodes.md`         | ARCHIVE    | chips-cli funding instructions for the LN era.                                                                                                   | `docs/archive/`                             |
| `tx_flow.md`                  | DEPRECATED-IN-PLACE | Walks through `extract_tx_data` dumps of a CHIPS-era hand — none of that wire format exists today. Replaced content with a deprecation notice pointing at `tx_types.md`, `game_state.md`, and `PLAYER_JOIN_FLOW.md`. To be archived. | `docs/archive/` (planned)                   |
| `tx_types.md`                 | REWRITTEN  | Collapsed the three-class taxonomy to the actual on-chain footprint today: payin (`sendcurrency`), payout (`sendcurrency`), identity update (`updateidentity`). Marked the legacy `extract_tx_data` / embedded-JSON dust-tx pattern as no longer wired. | `docs/reference/tx-types.md`                |
| `ubuntu_compile.md`           | DEPRECATED-IN-PLACE | Old Ubuntu CHIPS+LN+bet sequence is fully superseded by the consolidated `compile.md`. Replaced content with deprecation notice; not currently linked from any live doc. To be archived. | `docs/archive/` (planned)                   |
| `upgrade.md`                  | REWRITTEN  | Re-cast as "Daemon Compatibility Surface": exhaustive table of the Verus RPCs `bet` shells out to, grouped (identity/wallet/tx/chain), each labelled live-vs-dormant. Removed the LN RPC list; added an upgrade-checklist for porting `bet` to a new Verus upstream. | `docs/reference/rpc-dependency.md`          |

> Also in `docs/protocol/`: `default_Doxyfile` and `install.sh`. Neither is
> Markdown; they don't belong in `docs/` anyway. Phase 2 candidates to move out
> (`tools/` or delete `default_Doxyfile`).

---

## `docs/verus_migration/*.md`

This directory is the *closest* to current and is where the rewrite work
should land its new home. Four files are empty stubs and should be removed
outright before they confuse anyone.

| File                       | Tag        | Rationale                                                                                                                                                  | Move target (Phase 2)                          |
|----------------------------|------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------|
| `adhoc-multisig.md` (was `adhoc_msig.md`) | REWRITTEN  | Rewritten May 11 against `cashier.c:114`, `payment.c:74`, `vdxf.c:join_table`, `states.c:657`. Reframed: the "multisig pain" the old doc described is the *legacy* P2SH path (still compiled but dead at runtime, comment at `states.c:657` confirms). New doc covers what ad-hoc multisig actually means on Verus (`primaryaddresses` + `minimumsignatures` + revocation/recovery on the identity record), how `bet` uses the cashier identity as both deposit address and signing vault, and is explicit about which legacy globals are still computed-but-unused. | `docs/explanation/adhoc-multisig.md` (Phase 2) |
| `architecture.md`          | REWRITTEN  | Rewritten May 11 against `vdxf.c`, `poker_vdxf.c`, `commands.c`, `config.c`. Layered structure preserved (accurate). Fixed: `chips` chain → `VRSCTEST` throughout; nonexistent `verus_ids_keys.ini` → split into real `blockchain.ini` + `keys.ini`; stale `*.chips.vrsc@` IDs → `*.VRSCTEST@`; nonexistent `poker_find_table()` dropped from API list; "REST recommended" replaced with actual default (`use_rest_api = 0` per `config.c:771`); diagram captions sharpened and `update_with_retry` added as the documented update choke point. Cross-links updated to rewritten `verus-overview.md`, `cli-print.md`, `getidentitycontent.md`. | `docs/explanation/architecture.md` (Phase 2)   |
| `dealer_rejoining.md`      | DELETED    | Removed May 11. Was a 0-byte stub. Inbound link in `docs/README.md` Advanced Topics removed in the same change (no replacement; concept not yet implemented). | —                                              |
| `deck_shuffling.md`        | REWRITTEN  | Rewritten May 11 against `player.c:player_init_deck`, full `vdxf.h` key family (PLAYER_DECK_KEY, T_D_P*, C_B_P*, T_B_P*, C_CARD_BV, T_CARD_BV), and the single-writer split. Curve25519 deck-representation content preserved. Layered on top: corrected the 4-pass shuffle flow (player → dealer → cashier → dealer canonicalize), each pass landing on its actor's own identity; explicit 5872-byte CMM-size constraint with its implication that the writes serialize at ≈ 4n `updateidentity` calls per hand; old "47 KB total" figure reframed as aggregate-over-hand vs. single-transaction. Cross-links to verus-overview / player-rejoin / getidentitycontent / architecture. | `docs/explanation/deck-shuffling.md` (Phase 2) |
| `dispute_resolution.md`    | DELETED    | Removed May 11. Was a 0-byte stub. Two inbound links in `docs/README.md` replaced with a one-line pointer to `docs/TODO.md` item 3 (where the on-chain key layout and unanswered design questions live). | —                                              |
| `game_state.md`            | REWRITTEN  | Rewritten May 11 against `game.h:6-29` (full enum), `game.c:32-62` (`game_state_str`), `dealer.c:122-674` (transition logic), and the `G_SETTLEMENT_COMPLETE_BY_CASHIER` / `G_DECK_SHUFFLING_B` cashier-side mirror pattern documented in `vdxf.h:19-29` and `vdxf.h:280-295`. Old doc listed 10 states; code has 14 — added `G_REVEAL_CARD_P_DONE`, `G_ROUND_BETTING`, `G_SHOWDOWN`, `G_SETTLEMENT_PENDING`, `G_SETTLEMENT_COMPLETE`, `G_SETTLEMENT_COMPLETE_BY_CASHIER`; collapsed split `G_REVEAL_CARD_B`/`G_REVEAL_CARD_P` (which never existed in code) into the single `G_REVEAL_CARD` (7). Added explicit per-state who-writes-where description and ASCII state-flow diagram covering the repeating reveal+bet block. Cross-links to verus-overview / deck-shuffling / player-rejoin / ids_keys_data. | `docs/reference/game-states.md` (Phase 2)      |
| `getidentitycontent.md`    | REWRITTEN  | Rewritten May 11 against `vdxf.c:189-267`. Old doc framed the need as "multiple entities update the same ID" — wrong after the single-writer migration. New doc correctly frames it as "each `updateidentity` REPLACES the CMM, `getidentitycontent` COMBINES updates from `height_start`" (verbatim from `vdxf.c:252`). RPC example updated to `verus -chain=VRSCTEST`; `bet`'s wrapper functions (`get_cmm_from_height`, `get_cmm_key_data_from_height`) and the `heightend=-1` mempool inclusion called out. | `docs/explanation/getidentitycontent.md` (Phase 2) |
| `id_creation_process.md`   | REWRITTEN  | Rewritten May 11 against `dealer_registration.c:75-150`, `config.c:32+86`, `dealer.ini`, `bet.h:292` (`struct float_num`/`struct table`), `misc.c:91-107`. Reframed around the permanent-vs-transient identity lifecycle and the aggregator-vs-operational distinction (old doc conflated `cashiers.…` aggregator with operational cashier). All `chips10sec` examples replaced with VRSCTEST; full `getidentity` JSON dumps dropped (dev-session-specific) in favour of structural description. `verus_dealer.ini` → `dealer.ini` with cross-reference to new `_code_suggestions.md` item 2 (stale error string). RA framing softened to match what the code actually does (parent-keyholder is implicit gatekeeper). Cross-links to `revoke-recovery.md`, `adhoc-multisig.md`, `PLAYER_JOIN_FLOW.md`, `cli-print.md`, `ids_keys_data.md`. | `docs/explanation/identity-tree.md` (Phase 2)  |
| `ids_keys_data.md`         | REWRITTEN  | Rebuilt May 11 as a real per-key reference, against the full `poker/include/vdxf.h` (all 23 base macros + the T_D_P*, T_B_P*, C_B_P* per-player families + the per-game `<game_id>` suffix convention) and the per-key docstrings already in the header. Old doc was 384 lines with duplicated sections (Mike Toutonghi quote + datatypes + identity-tree concept all appeared twice), `chips10sec` chain, stale `chips.vrsc::poker.cashiers`-prefix (missing the `sg777z.` segment), and listed only ~5 keys when code has 40+. New doc: brief conventions section (namespace, `<game_id>` suffix, VDXF data types, single-writer rule) then five per-identity sections (aggregator, dealer, table, cashier, player) covering every macro with key name, identity owner, writer, reader, payload shape, and suffix behaviour. Hole-card key + showdown reveal key + board cards + settlement info + cashier-side mirrors all present. Cross-references to verus-overview / architecture / game-state / deck-shuffling / player-rejoin / PLAYER_JOIN_FLOW / cli-print. `T_GAME_INFO_KEY` payload corrected against code (`vdxf.h:249-253` docstring is wrong — logged in `_code_suggestions.md` item 3). | `docs/reference/vdxf-keys.md` (Phase 2)        |
| `permutation.md`           | DELETED    | Removed May 11. Was a 0-byte stub; no inbound references.                                                                                                   | —                                              |
| `player_joining.md`        | DEPRECATED-IN-PLACE | Walked May 11 against `vdxf.c:join_table`, commit `8e5eefa9` (Jan 2026), and `docs/reference/player-join-flow.md`. Original content described a legacy flow (4-byte tx-hash dedup, embedded-data payin) that was killed by the simplification commit; current flow lives on the player identity's `P_JOIN_REQUEST_KEY` and is fully documented in `docs/reference/player-join-flow.md`. **No salvage** — the only specific mechanism (4-byte hash dedup) is no longer in code. Body replaced with a redirect pointer to `PLAYER_JOIN_FLOW.md`; existing inbound links keep working. Phase 2 will decide whether to fold into `archive/` or delete. | `docs/reference/player-join-flow.md` (canonical) / archive-or-delete (Phase 2) |
| `player.md`                | DELETED    | Removed May 11. Was a 0-byte stub; no inbound references.                                                                                                   | —                                              |
| `player-rejoin.md` (was `player_rejoining.md`) | REWRITTEN | Rewritten May 11 against `BET_TURN_TIMEOUT_SECS/BLOCKS` (`common.h:132-133`), `game.c:verus_check_turn_timeout`, `storage.c:player_deck_info` schema (line 35) + `player_local_state` schema (line 39), and `player.c:1430-1520` reconnect path. **Old doc was almost entirely wrong against current code**: claimed 20-second timeout (real: 120s + 12 blocks), claimed file-based storage under `~/.bet/privatebet/.game_info/player/` (real: SQLite `player_deck_info` + `player_local_state` tables), claimed 5% mid-shuffle penalty (not implemented), claimed PIN-encrypted secrets on player ID (not implemented). New doc describes the actual three-category local state, the actual reconnect sequence, and explicitly calls out which legacy-doc claims are unimplemented. | `docs/explanation/player-rejoin.md` (Phase 2) |
| `cli-print.md` (was `print.md`) | REWRITTEN   | Rewritten May 11 against `print.c` + `bet.c` + `poker_vdxf.c`. Now covers all five commands (`print_id`, `print_table_key`, `print`, `print_keys`, `show`), all five `print_id` types (`table`/`dealer`/`player`/`dealers`/`cashier`) with their aliases, the restricted key set for `./bet print`, and the `T_B_P*_DECK_KEY` / `T_CARD_BV_KEY` migration to the cashier id. May 15: added the "How every `print_id` printer reads history" section — explains why bootstrap reads use the daemon's `vdxfkey` filter (unbounded `getidentitycontent 0 -1` truncates and silently drops the most recent CMM writes), how each id type derives a per-id `start_block` from on-chain data (`T_TABLE_INFO_KEY.<game_id>.start_block` on tables, `P_GAME_HISTORY_KEY.<game_id>.join_block` on players, `getidentityhistory`-walk via `find_cmm_key_write_block` on cashiers because the cashier id stamps no block field), and why `P_JOIN_REQUEST_KEY` reads ahead of the `game_id` gate. `cashiers` alias renamed to `cashier`. | `docs/reference/cli-print.md` (Phase 2 move)   |
| `revoke-recovery.md` (was `rec_rev.md`) | REWRITTEN  | Rewritten May 11 on VRSCTEST. Old doc was on `chips10sec` test chain with real txids/i-addresses from a dev session, plus open-ended "what's the right design?" working-note prose. New doc explains revocation/recovery semantics from scratch, uses unambiguous `<R-ADDR>` / `<I-ADDRESS>` placeholders (no hardcoded values that won't work on the reader's chain), walks the two-step `registernamecommitment` + `registeridentity` flow, and adds a closing section on the deployment-wide vs per-identity authority-pair tradeoff. Confirmed `revocationauthority`/`recoveryauthority`/`registeridentity` are not invoked by `bet`'s code — pure operator workflow. Inbound link in `id_creation_process.md` updated. | `docs/tutorials/revoke-recovery.md` (Phase 2) |
| `schema.md`                | DEPRECATED-IN-PLACE | Walked May 11 against `storage.c:14-40`. Doc's `player_deck_info` schema was wrong against current code (5 cols documented vs. 8 cols in source — missing `tx_id`, `pa`, `table_id`, `dealer_id`). Doc also had self-deprecating "we don't store anything locally" footer that contradicted its own body. **No salvage** — the accurate schemas are now inlined where they actually matter (`player-rejoin.md` covers `player_deck_info` + `player_local_state`; `verus-overview.md` covers the broader SQLite role). Body replaced with redirect. Phase 2 decides archive-vs-delete. | `verus-overview.md` + `player-rejoin.md` (canonical) / archive-or-delete (Phase 2) |
| `verus-overview.md` (was `verus_migration.md`) | REWRITTEN  | Rewritten May 11 against `keys.ini`, `vdxf.h`, `config.c`, `storage.c`, `heartbeat.c`, and `update_with_retry`. Reframed as a current-state overview (not a migration plan); parent ID examples corrected to `*.sg777z.VRSCTEST@`; CMM key prefix described as-is (`chips.vrsc::poker.sg777z.`); single-writer-per-identity rule called out; UTXO contention model explained; off-topic CHIPS-conversion sidebar removed; heartbeat described honestly as `#ifdef LIVE_THREAD` compile-time dead; SQLite role narrowed to `player_local_state`; 5872-byte CMM size limit and `sleep(3)` removal documented. | `docs/explanation/verus-overview.md` (Phase 2) |

---

## `poker/docs/*.md`

| File                          | Tag      | Rationale                                                                                                          | Move target (Phase 2)                  |
|-------------------------------|----------|--------------------------------------------------------------------------------------------------------------------|----------------------------------------|
| `poker/docs/GUI_PROTOCOL.md`  | DEPRECATED-IN-PLACE | Replaced content with a "superseded" notice pointing at the rewritten `docs/reference/gui-message-formats.md`. Called out two specific places this old doc diverges from current code (string-action betting reply; obsolete `player_init_state` flow). To be archived once Phase 2 confirms no inbound links remain. | `docs/archive/` (planned)                    |

---

## Other files in `docs/` (not Markdown — out of inventory scope)

| File                                  | Decision                                                                                                          |
|---------------------------------------|-------------------------------------------------------------------------------------------------------------------|
| `docs/CNAME`                          | KEEP — used by GitHub Pages.                                                                                       |
| `docs/Makefile`                       | KEEP — used by the doc build (Sphinx? Pandoc?). Re-evaluate in Phase 4 if we adopt a different toolchain.          |
| `docs/Unsolicited_PANGEA_WP.pdf`      | KEEP — historical whitepaper. Move to `docs/archive/whitepaper/` to keep it next to the historical drafts.        |
| `docs/protocol/default_Doxyfile`      | Move out of `docs/` — belongs in `tools/doxygen/` if we still build doxygen, otherwise delete.                     |
| `docs/protocol/install.sh`            | Move out of `docs/` — `scripts/legacy-install.sh` and archive, or delete (LN-era).                                 |

---

## Open decisions (resolved May 11, 2026)

1. **Diátaxis bucket names.** Resolved → `tutorials/` / `guides/` /
   `reference/` / `explanation/` / `archive/`. Used `guides/` instead of
   `how-to/` as more familiar.
2. **CHANGELOG.** Resolved → **deferred.** No `CHANGELOG.md` for now;
   archive-style entries (`REMOVED_GUI_JOIN_APPROVAL.md`,
   `VERUS_TEST_BRANCH_SUMMARY.md`, `GUI_INTEGRATION_STATUS.md`) are kept
   verbatim under `docs/archive/`. Can be revisited if/when a real release
   cadence starts.
3. **`docs/protocol/` vs `docs/verus_migration/` namespaces.** Resolved →
   **collapsed.** Both directories were emptied; surviving content redistributed
   to `docs/{tutorials,guides,reference,explanation}/`; archived content moved
   to `docs/archive/docs/protocol/` and `docs/archive/docs/verus_migration/`
   to preserve original path context.
4. **`predictable_betting.md`.** Resolved → **deleted.** Not a poker doc;
   the prediction-market wagers proposal does not belong in this repo's
   documentation.
5. **`gui_e2e_test_plan_8a4b327f.plan.md` in `.cursor/plans/`.** Not yet
   actioned — still lives under `.cursor/plans/` and is the de facto GUI
   testing spec. If the community-facing GUI testing flow needs to be
   documented outside Cursor, that work goes into
   `docs/guides/gui-e2e-test.md` (not yet created).
