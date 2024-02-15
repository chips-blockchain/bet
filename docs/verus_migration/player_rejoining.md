## Player Rejoining
-------------------
Bet stores secret information local to the node in whcih its running either on local DB or in local files. If not all, most of the game related public information of the player is stored across verus IDs(specifically in table/dealer/cashier IDs). When the player gets disconnected, bet allows the player to rejoin and if the player fails to rejoin within the agreed time interval(atm, its 20s) then bet allows the dealer to take a predtermined action for the player inorder for the game to moveon. 

The action taken by the dealer depends on the game state, i.e 
  -  if the disconnection happens before betting then the correponding player is dropped from the table and dealer(or player) raise a request to reverse the payin_tx of the player.
  -  if the disconnection happens during the game, then the dealer considers players move as <b>fold</b> and proceeds with the game.

It's also possible that the player can raise a dispute request to claim the payin_tx while being in the game, to prevent any misuse of the dispute handling cashiers will always check if that payin_tx is not attached to any table and if its attached to any table then cashiers reject the dispute handling.

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
