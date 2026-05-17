# Community Quickstart: Playing via GUI

This guide explains how to set up the Pangea Poker backend nodes and connect them to the React frontend GUI so you can play a hand of poker directly through your browser. 

If you are looking to run automated headless tests without a GUI, refer to the [CLI Auto Quickstart](community-quickstart.md) instead.

---

## Phase 1: Prerequisites & Configuration

Before you begin, ensure you have the Verus daemon running with `-chain=chips` (or your local `VRSCTEST` regtest) and have cloned the `pangea-poker` (GUI) repository on your machine.

All backend nodes share the `bet/poker/config/` directory. You must configure these files with your registered Verus identities before starting:

1. **`blockchain.ini`**: Set your chain (e.g., `blockchain_cli = "verus -chain=chips"`, `currency = chips`).
2. **`keys.ini`**: Verify the aggregator identity FQNs (`cashier_id`, `dealers_id`).
3. **`.rpccredentials`**: Ensure it matches your daemon's RPC user/password.
4. **`dealer.ini` / `cashier.ini`**: Verify `dealer_id`, `cashier_id`, and `table_id` FQNs. Ensure the `[players]` section contains the comma-separated list of allowed player IDs.
5. **`p1.ini` / `p2.ini`**: Verify the `player_id` FQN for each player config. Make sure `ws_port` is distinct for each player (e.g., `9001` for p1, `9002` for p2).

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