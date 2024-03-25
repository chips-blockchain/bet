## Player Rejoining
-------------------
Bet stores the game related confidential information either in local DB or in local files on the node in which bet is running. If not all, most of the game related public information and the players information is stored across the verus IDs(specifically in player/table/dealer/cashier IDs). When the player gets disconnected, bet allows the player to rejoin and if the player fails to rejoin within the agreed time interval(atm, its 20s) then bet allows the dealer to make a decision based on the defined rules in order for the game to move on.

The action taken by the dealer depends on the game state, and at any state player has the choice to rejoin with in the 20s window of time, if by any reason player fails to rejoin the following will be the dealers action towards it:
  -  if the players disconnection happens before the deck shuffling then dealer raise a request to reverse the payin_tx and removes the corresponding players join info from the table and dealer IDs.
  -  if the players disconnection happens during the deck shuffling then dealer raises a request to reverse the payin_tx(dealer may chose to impose a penanlty of 5% on payin_tx or some fixed amount based on whichever is lower. The reason for penanlty is because as a part of deck shuffling players has already been spent many block fees for deck shuffling) and removes the corresponding players info from the table and dealer IDs and reset the game state to player joining state and will wait for other player to pitch in and fill the position.
  -  If the players disconnection happens during the game, then the dealer considers players move as <b>fold</b> and proceeds with the game.
 
It's also possible that the player can raise a dispute request to claim the payin_tx while being in the game, to prevent any misuse of the dispute handling cashiers will always check if that payin_tx is attached to any game ID in any of the active tables exist atm, if the payin_tx is attached to the active game ID then cashier nodes will wait until the game gets finished to handle the dispute request from the player.

### Data Needed for Rejoin
---------------------------
The secret information that is associated with the player is generated during the deck initialization process and which is
  -  Player keypair
  -  Deck privkeys (Each card represents a keypair, so 52 cards privkeys)
  -  Players shuffling pattern

  The public information that needs to stored/provided locally for the player rejoin is
    -  game_id (This will be fetched from the game_id file, player no need to remember)
    -  dealer_id (This will be fetched from verus_player.ini config)
    -  table_id (This will be fetched from verus_player.ini config)

All this secret information related to the player is stored under the local directory <b>/.game_info/player</b> on the node in which bet is running which is located under <b>bet/privatebet</b>. All these files in game_info are prepended with game_id, for a typical game the files stored locally by the player are:
  -  game_id (contains the present game id to which the player made payin_tx)
  -  game_id_str.player_key
  -  game_id_str.deck_keys
  -  game_id_str.deck_shuffle_pattern

 The information from the verus IDs the player fetches during the rejoin is
   -  Players payin_tx
   -  Players game state
   -  Shuffled and blinded deck
   -  Public info of the table and other player info
