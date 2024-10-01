# Player Rejoining in Verus Migration

Bet stores game-related confidential information either in a local database or local files on the node where Bet is running. Most game-related public information and player information are stored across Verus IDs (specifically in player, table, dealer, and cashier IDs). When a player gets disconnected, Bet allows the player to rejoin. If the player fails to rejoin within the agreed time interval (currently 20 seconds), the dealer can make a decision based on predefined rules to keep the game moving.

The dealer's action depends on the [game state](./game_state.md). At any game state, a player can rejoin upon disconnection, and the rejoin window remains the same. If a player fails to rejoin, the dealer can take the following actions:
- If the disconnection occurs before deck shuffling, the dealer requests to reverse the payin_tx and removes the player's join info from the corresponding table and dealer IDs.
- If the disconnection occurs during deck shuffling, the dealer requests to reverse the payin_tx (a penalty of 5% or a fixed amount, whichever is lower, may be imposed due to block fees incurred during shuffling). The dealer then removes the player's info from the table and dealer IDs, resets the game state to player joining, and waits for another player to fill the position.
- If the disconnection occurs during the game, the dealer considers the player's move as a **fold** and proceeds with the game.

Player disconnections at any game state and failure to rejoin will result in quashing previous updates made to the IDs. In many cases, players and dealers must redo processes, incurring penalties, mostly in block fees.

Players can raise a dispute request to claim the payin_tx while in the game. To prevent misuse, cashiers will check if the payin_tx is attached to any game ID in active tables. If attached, cashier nodes will wait until the game finishes to handle the dispute request.

## Data Needed for Rejoin

The secret/confidential information associated with the player is generated during the deck initialization process and includes:
- Player keypair
- Deck private keys (each card represents a keypair, so 52 card private keys)
- Player's shuffling pattern

The public information available on the player ID and needed for rejoining includes:
- game_id (fetched from the game_id file)
- dealer_id (fetched from verus_player.ini config)
- table_id (fetched from verus_player.ini config)

This information can be fetched from the local config file `verus_player.ini` and the `game_id` file.

All secret information related to the player is stored under the local directory `/.game_info/player` on the node where Bet is running, located under `bet/privatebet`. Files in `game_info` are prepended with game_id. For a typical game, the files stored locally by the player are:
- game_id (contains the current game id for the payin_tx)
- game_id_str.player_key
- game_id_str.deck_keys
- game_id_str.deck_shuffle_pattern

Information fetched from Verus IDs during rejoin includes:
- Player's payin_tx
- Player's game state
- Shuffled and blinded deck
- Public info of the table and other players

With the latest design changes, as players update all game-related info to the player ID, there is a provision to store game-related and player-related confidential information in encrypted form on the player ID. This data is encrypted using a PIN/password configured locally by the player. During rejoin, the player only needs to remember the locally configured PIN to rejoin.
