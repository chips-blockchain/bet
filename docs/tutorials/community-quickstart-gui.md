# Community Quickstart: Playing via GUI

This guide explains how to set up the Pangea Poker backend nodes and connect them to the React frontend GUI so you can play a hand of poker directly through your browser. 

If you are looking to run automated headless tests without a GUI, refer to the [CLI Auto Quickstart](community-quickstart.md) instead.

### Before you start

You will need:

1. **Verus daemon** running with `-chain=chips` and `verus` on your `PATH`.
2. **`bet` built** — clone this repo with submodules and compile; see [Build `bet` from source](../guides/build-from-source.md). From the repo root, `make -j$(nproc)` produces `poker/bin/bet`.
3. **`pangea-poker` cloned** — the React GUI frontend; see the [pangea-poker README § Development](https://github.com/chips-blockchain/pangea-poker#development) (`npm install`, then `npm start` in Phase 3 below).
4. **Registered Verus identities** and config under `bet/poker/config/` — worked examples in Phase 1.

---

## Phase 1: Prerequisites & Configuration

Configure the shared `bet/poker/config/` directory with your **registered Verus identities** before starting nodes. The snippets below show the **layout** the shipped examples use; replace every FQN with the IDs you registered on CHIPS.

### Example layout: cashier, dealer, table, two players

In this walkthrough:

- The **cashier** identity holds player payins and pays out the pot.
- Dealer **`d1`** hosts table **`t1`**.
- Players **`p1`** and **`p2`** join through the GUI (WebSocket ports **9001** and **9002**).

| Role     | Example FQN (shipped CHIPS layout)   |
|----------|--------------------------------------|
| Cashier  | `cashier.sg777z.chips.vrsc@`          |
| Dealer   | `d1.sg777z.chips.vrsc@`              |
| Table    | `t1.sg777z.chips.vrsc@`              |
| Player 1 | `p1.sg777z.chips.vrsc@`              |
| Player 2 | `p2.sg777z.chips.vrsc@`              |

### Example config files

Only the fields you need to verify for a first GUI run are shown. Optional knobs live in the reference docs linked at the end of this section.

**`blockchain.ini`** — how `bet` invokes the daemon and names the currency:

```ini
[blockchain]
blockchain_cli = "verus -chain=chips"
currency       = chips
new_block      = "Y"
```

**`keys.ini`** — aggregator identities (discovery lists on chain):

```ini
[identities]
cashier_id = cashier.sg777z.chips.vrsc@
dealers_id = dealers.sg777z.chips.vrsc@
```

**`.rpccredentials`** — copy `poker/.rpccredentials.example` to `poker/.rpccredentials` and set user/password to match your daemon config (same values as in your chain’s `*.conf`):

```ini
[rpc]
url = http://127.0.0.1:22778
user = your_rpc_user
password = your_rpc_password
use_rest_api = 1
```

With `use_rest_api = 1`, `bet` talks to the daemon over HTTP JSON-RPC. Set `use_rest_api = 0` to use the `verus` CLI as a subprocess instead (see [Architecture § RPC credentials](../explanation/architecture.md)).

**`dealer.ini`** — table parameters and which players may join:

```ini
[table]
max_players = 2
big_blind   = 0.001
min_stake   = 20
max_stake   = 100
table_id    = t1.sg777z.chips.vrsc@

[verus]
dealer_id  = d1.sg777z.chips.vrsc@
cashier_id = cashier.sg777z.chips.vrsc@

[players]
ids = p1.sg777z.chips.vrsc@, p2.sg777z.chips.vrsc@
```

**`cashier.ini`** — allowed players (must match dealer):

```ini
[cashier]
gui_ws_port = 9002

[players]
ids = p1.sg777z.chips.vrsc@, p2.sg777z.chips.vrsc@
```

**`p1.ini`** / **`p2.ini`** — one file per player process. When both players run on the **same host**, give each a different `ws_port` (below); on separate hosts each can use any open port (often **9001** on each machine):

```ini
# p1.ini
[verus]
dealer_id   = d1.sg777z.chips.vrsc@
table_id    = t1.sg777z.chips.vrsc@
wallet_addr = *
player_id   = p1.sg777z.chips.vrsc@
ws_port     = 9001
```

```ini
# p2.ini
[verus]
dealer_id   = d1.sg777z.chips.vrsc@
table_id    = t1.sg777z.chips.vrsc@
wallet_addr = *
player_id   = p2.sg777z.chips.vrsc@
ws_port     = 9002
```

### What must match

- **`table_id`**, **`dealer_id`**, and **`cashier_id`** — same FQNs wherever they appear in `dealer.ini` and both player configs.
- **`[players] ids`** — identical comma-separated list in `dealer.ini` and `cashier.ini`.
- **`player_id`** — each `pN.ini` must use a different player FQN from that list.
- **`ws_port`** — must be an open port on the machine where that player backend runs. **Unique per player only when two player processes share one host** (e.g. **9001** / **9002** below); if each player runs on its own machine, both can use **9001**. In Phase 4, the GUI **host** and **port** must match the player process you started (`ws://<host>:<port>`).
- **RPC credentials** — `.rpccredentials` user/password must match the running daemon when using REST.

### Further reading

- [Dealer configuration](../reference/dealer-config.md)
- [Cashier configuration](../reference/cashier-config.md)
- [Player configuration](../reference/player-config.md)

---

## Phase 2: Starting the Backend Nodes

You will need to open a separate terminal (or `tmux` pane) for each node.

### Terminal 1: Dealer
Start the dealer node with the `--reset` flag to ensure a clean state for a new hand:
```bash
cd bet/poker
./bin/bet start dealer --config config/dealer.ini --cli --reset
```
*Wait until you see:* `Dealer ready (RESET). Waiting for players to join...`

### Terminal 2: Cashier
```bash
cd bet/poker
./bin/bet start cashier --config config/cashier.ini --cli
```
*Wait until you see:* `Cashier idle — polling player IDs for join requests`

### Terminal 3: Player 1 (GUI mode)
Start the player node using the `--gui` flag. This spins up the WebSocket server on the port specified in `p1.ini` (e.g., 9001).
```bash
cd bet/poker
./bin/bet start player --config config/p1.ini --gui
```
*Wait until you see:* `Player node started. WebSocket server listening on port 9001`

*(Optional)* **Terminal 4: Player 2 (GUI mode)**
```bash
cd bet/poker
./bin/bet start player --config config/p2.ini --gui
```

At this stage, the backend is fully initialized and **waiting** for the GUI to connect. No payin transactions have been broadcasted yet.

---

## Phase 3: Starting the Frontend (GUI)

Open a new terminal for the React frontend:

```bash
cd pangea-poker
npm install  # If running for the first time
npm start
```

This will automatically open your web browser to `http://localhost:1234`.

---

## Phase 4: Connecting the GUI to the Backend

1. In the browser, navigate to the **Player** (or **Private Tables**) tab in the startup modal.
2. Enter the IP address of your backend node (use `localhost` if running locally).
3. Enter the WebSocket port for Player 1 (e.g., `9001`).
4. Click the **Set Nodes** button.
5. The GUI will connect to your player backend and fetch your wallet balance.

---

## Phase 5: Joining the Table and Playing

### Step 1: Find a Table
1. A large **Find Table** button will appear in the center of the screen.
2. Click **Find Table**. The backend will query the blockchain for available dealers and tables.

### Step 2: Take a Seat
1. Once a table is found, the table layout will appear.
2. Occupied seats will be visually marked.
3. Click on any **Available Seat** to sit down.
4. This action approves the join request. The backend will now execute your `payin` transaction to the cashier.

### Step 3: Wait for the Game to Start
* You will see a notification: **"Joining table... Please wait 10-30 seconds"**.
* The game must wait for the blockchain to mine your payin transaction (and the transactions of other players).
* If you are playing with two players, open a **new browser tab**, repeat Phase 4 using Player 2's port (e.g., `9002`), and repeat Steps 1-2 to seat Player 2.

### Step 4: Play the Hand
Once all required players have successfully joined and their payins are mined, the dealer will shuffle the deck and deal the hole cards. 
* Your cards will appear on screen.
* When it is your turn, the betting controls (Check, Call, Raise, Fold, All-in) will become active based on the current game state.

Enjoy your decentralized poker game!