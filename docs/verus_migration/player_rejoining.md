## Player Rejoining
-------------------
Bet stores the game related confidential information either in local DB or in local files on the node in which bet is running. If not all, most of the game related public information and the players information is stored across the verus IDs(specifically in player/table/dealer/cashier IDs). When the player gets disconnected, bet allows the player to rejoin and if the player fails to rejoin within the agreed time interval(atm, its 20s) then bet allows the dealer to make a decision based on the defined rules in order for the game to move on.

The action taken by the dealer is depends on the [game state](./game_state.md), and at any game state player can rejoin upon the disconnection and the window of time interval to rejoin remains same at all game states. If by any reason player fails to rejoin dealer can take following action:
  -  if the players disconnection happens before the deck shuffling then dealer raise a request to reverse the payin_tx and removes the corresponding players join info from the corresponding table ID and dealer IDs of the dealer.
  -  if the players disconnection happens during the deck shuffling then dealer raises a request to reverse the payin_tx(dealer may chose to impose a penanlty of 5% on payin_tx or some fixed amount based on whichever is lower. The reason for penanlty is because as a part of deck shuffling players has already been spent many block fees for deck shuffling) and removes the corresponding players info from the table and dealer IDs and reset the game state to player joining state and will wait for other player to pitch in and fill the position.
  -  If the players disconnection happens during the game, then the dealer considers players move as <b>fold</b> and proceeds with the game.

Player disconnections at any game state and failure to rejoin will result in quashing previous updates made to the IDs and many cases players and dealers has to undergo the redoing process, due to which there always be going to be some penalty to the player and dealer disconnections and this penalty is mostly in block fees. 

It's also possible that the player can raise a dispute request to claim the payin_tx while being in the game, to prevent any misuse of the dispute handling cashiers will always check if that payin_tx is attached to any game ID in any of the active tables exist atm, if the payin_tx is attached to the active game ID then cashier nodes will wait until the game gets finished to handle the dispute request from the player.

### Data Needed for Rejoin
---------------------------
The secret/confidential information that is associated with the player is generated during deck initialization process and which includes
  -  Player keypair
  -  Deck privkeys (Each card represents a keypair, so 52 cards privkeys)
  -  Players shuffling pattern

  The public information that gets available on the player ID and needed for the player rejoin is 
    -  game_id (This will be fetched from the game_id file, player no need to remember)
    -  dealer_id (This will be fetched from verus_player.ini config)
    -  table_id (This will be fetched from verus_player.ini config)
Along with player ID the above information can also be fetched from the local config file such as `verus_player.ini`(which contains `dealer_id` and `table_id`) and from the file `game_id`(which contains `game_id`).

All this secret information related to the player is stored under the local directory <b>/.game_info/player</b> on the node in which bet is running which is located under <b>bet/privatebet</b>. All these files in game_info are prepended with game_id, for a typical game the files stored locally by the player are:
  -  game_id (contains the present game id to which the player made payin_tx)
  -  game_id_str.player_key
  -  game_id_str.deck_keys
  -  game_id_str.deck_shuffle_pattern

 The information from the verus IDs the player fetches during the rejoin is
   -  Players payin_tx
   -  Players game state
   -  Shuffled and blinded deck
   -  Public info of the table and other players info

With the latest design changes as player updates all the game related info to the player ID, we also provide a provision to store the game related and player related confidential information to the player ID in encrypted form. This players data is encrypted using PIN/password that player configured locally and with this during player rejoin, player just needs to remember the locally configured PIN and with which player can rejoin.
