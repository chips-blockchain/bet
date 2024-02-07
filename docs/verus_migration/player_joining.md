Players Joining
---------------

This is an important phase in game setup. In the [ID creation document](./id_creation_process.md) we discussed about how cashiers and dealers are registered by the RA(Registration Authority). Here we see more details about the tables and how player finds and joins the tables. 

### Table
---------
Any dealer can host any number of tables, dealer can reuse the same ID repeatedly to host the tables. Some key points to note w.r.t tables are
1. The control is with the dealer to create any number of ID's and register them as tables to the corresponding dealer ID, though some concerns exist with regard to spam of IDs but we look at this way that more IDs means more economy to the chain and less hastle.
2. In order to join a specific table, player needs to mention that table name in their `verus_player.ini` config file, so its wise to have some catchy names for the tables and using the same ID repeatedly allows the players not to make any modifications to their config file. 
3. Using the bet command `./bet list_tables` players can find all the tables available on the chain. 

### How players find the table
-------------------------------
There are couple of ways using which players can find the tables registered on the chain which are listed below, the recommended way is using the bet API `list_tables`, i.e by `./bet list_tables` which gives more information about the tables along with its names.

1. Dealer ID contains the information about the tables, using the following commands one can list the available dealers in the system.
   a. Using the bet API `list_dealers`
   ```
	root@sg777-3 ~/bet/privatebet # ./bet list_dealers
	[bet.c:bet_start:549] Dealers ::["sg777_d"]
   ```
   b. By parsing the `dealers.poker.chips10sec@` ID using the bet API(`print_id`). Using verus client API(`getidentity`) also one can see the dealers info but that info is encoded in hex so we recommned using `print_id`.
   ```
      	root@sg777-3 ~/bet/privatebet # ./bet print_id dealers dealers
	[print.c:print_dealers_id:72] ["sg777_d"]
   ```	
2. Uisng the bet API `list_tables` all the tables registered on the chain along with the current status of those tables are listed
   ```
   root@sg777-3 ~/bet/privatebet # ./bet list_tables
	[vdxf.c:list_tables:1214] dealer_id::sg777_d
	[vdxf.c:list_tables:1217] {
	        "max_players":  2,
	        "big_blind":    0.00100000,
	        "min_stake":    0.20000000,
	        "max_stake":    1,
	        "table_id":     "sg777_t",
	        "dealer_id":    "sg777_d"
	}
	[vdxf.c:list_tables:1226] Player Info of Table ::sg777_t is ::{
	        "num_players":  2,
	        "player_info":  ["RUDCNptNJZrzFErgRXgPcfEcWXdY7Rn7x2_94960563187ebb17f825edde1a12ab262c3f1a0341cf8e7138a94a0bf63432e4_1", "RMVaEaXzMcwCuNVL3rkgt8i8j94DVfFAht_83d6a7c49d2822fb20fd88eac0cc6753e9f97253da06ddc5bd17b5b1b88fb113_2"]
	}
	```

3. Using bet API `print_id` one can get the content of the dealer ID and from which we can see the tables hosted by that dealer.
	   
	```
	root@sg777-3 ~/bet/privatebet # ./bet print_id sg777_d dealer
	[print.c:print_dealer_id:84] {
	        "max_players":  2,
	        "big_blind":    0.00100000,
	        "min_stake":    0.20000000,
	        "max_stake":    1,
	        "table_id":     "sg777_t",
	        "dealer_id":    "sg777_d"
	}
	```
4. Once you get the table, using `print_id` one can see the state of the table(`game_state`) and the public info of the game that is currently being played.
	```
	root@sg777-3 ~/bet/privatebet # ./bet print_id sg777_t table
	[print.c:print_table_id:94] game_id::13a37fd1a16a5d339100dd1f51d90fb354c78939899da366f023ba31ec013731
	[print.c:print_table_id:98] t_player1 :: {
	        "id":   1,
	        "pubkey":       "51a28b8ceb1dfb1076c8b603272cdfa7a23ae8ca3c02f39dfd0ac5b4739ffb4b",
	        "cardinfo":     ["889ed5713e42785200bf18cdea9c1eb295915470322bdb8eaea974e605117c75", "c4d2ea299a2157f7385155606ce8e02d9f6a899f987bd364c7677dc7421e7452", "d773f7b34c6121b675f91745c5b908b40bcb359dcf1d183cfb388ed5a3120e15"]
	}
	[print.c:print_table_id:98] t_player2 :: {
	        "id":   2,
	        "pubkey":       "5a5e1611041bef3cab1b89e6338e2d9c3bb7e0fa69db3c4f445a85688357a556",
	        "cardinfo":     ["0c9d5d83449ffee4858ffc0c9c0672a13366e2a42ba07e1ceea79e08c3413635", "8192068f42cf1874af3c95e1db82c073ac27f94998e52528f11076ba27128155", "8e6682caab4e8552ece3d7b901df2f10210fd82b69cf82607a6b31f2d0714912"]
	}
	[print.c:print_table_id:98] t_table_info :: {
	        "max_players":  2,
	        "big_blind":    0.00100000,
	        "min_stake":    0.20000000,
	        "max_stake":    1,
	        "table_id":     "sg777_t",
	        "dealer_id":    "sg777_d"
	}
	[print.c:print_table_id:98] t_player_info :: {
	        "num_players":  2,
	        "player_info":  ["RUDCNptNJZrzFErgRXgPcfEcWXdY7Rn7x2_94960563187ebb17f825edde1a12ab262c3f1a0341cf8e7138a94a0bf63432e4_1", "RMVaEaXzMcwCuNVL3rkgt8i8j94DVfFAht_83d6a7c49d2822fb20fd88eac0cc6753e9f97253da06ddc5bd17b5b1b88fb113_2"]
	}
	[print.c:print_table_id:104] t_d_deck :: ["a80ffb25eefab5279af2f388f24744dc782935ebee6738030fd1ceb278dd2018", "c325de70d42d32dd7dd3685423e2df8c6388ad98453d6df0e4d3da66855a9d7e", "1f380df2c328b51e9143aa7bbb087c244f47b72109693a2f6b7721204ef0e705"]
	[print.c:print_table_id:104] t_d_p1_deck :: ["903e7f8784abe57808eb2e6c3ee8a6d5b4b305a274f0567c473ba168de2ddc56", "01a30726d99137613f0e277313556a205833420f3d5d56ddc1f9850a08d52972", "49c36f37459d0bbe5ebc69c7d793db80829f3e5fd798e7280febdaf5b7b4e85f"]
	[print.c:print_table_id:104] t_d_p2_deck :: ["a541ef87e24d7caad877b33d8a8e0b4415752b7b27b2b12502c163ca7127ff10", "87f1146747f5809ad7bae05f7da26c0483fa8533e40dc90d1bf69fd76ad43041", "3996298f6664dfa976592f99af7f4e75cd2216220073bef0f253a8764daad95a"]
	[print.c:print_table_id:114] chips.vrsc::poker.t_game_info :: {
	        "game_state":   5
	}
	```
### What goes into table
------------------------
Off all the ID's we have table ID is the very complex ID and this is where all the game info goes into. The complete map of contents that goes into the table is not yet fully identified and as the development of the game progresses I'll update the details of table ID contentmultimap here. 

Since the table ID is the only place where all the game information is held, its natural that all the entities involved in the game must make updates to this ID. So players, dealers, and cashiers/bvv make updates to the table ID during the game on their turn. The crucial actions in the game like deck shuffling, game play and game settlement are done based on the information held by table ID. Every action that takes place during the game requires different kind of data, so due to which we grouped the information under table ID contentmultimap using the following keys:
```
- T_GAME_ID_KEY "chips.vrsc::poker.t_game_ids"
- T_TABLE_INFO_KEY "chips.vrsc::poker.t_table_info"
 
- T_PLAYER_INFO_KEY "chips.vrsc::poker.t_player_info"
- T_PLAYER1_KEY "chips.vrsc::poker.t_player1"
- T_PLAYER2_KEY "chips.vrsc::poker.t_player2"
- T_PLAYER3_KEY "chips.vrsc::poker.t_player3"
- T_PLAYER4_KEY "chips.vrsc::poker.t_player4"
- T_PLAYER5_KEY "chips.vrsc::poker.t_player5"
- T_PLAYER6_KEY "chips.vrsc::poker.t_player6"
- T_PLAYER7_KEY "chips.vrsc::poker.t_player7"
- T_PLAYER8_KEY "chips.vrsc::poker.t_player8"
- T_PLAYER9_KEY "chips.vrsc::poker.t_player9"
 
- T_D_P1_DECK_KEY "chips.vrsc::poker.t_d_p1_deck"
- T_D_P2_DECK_KEY "chips.vrsc::poker.t_d_p2_deck"
- T_D_P3_DECK_KEY "chips.vrsc::poker.t_d_p3_deck"
- T_D_P4_DECK_KEY "chips.vrsc::poker.t_d_p4_deck"
- T_D_P5_DECK_KEY "chips.vrsc::poker.t_d_p5_deck"
- T_D_P6_DECK_KEY "chips.vrsc::poker.t_d_p6_deck"
- T_D_P7_DECK_KEY "chips.vrsc::poker.t_d_p7_deck"
- T_D_P8_DECK_KEY "chips.vrsc::poker.t_d_p8_deck"
- T_D_P9_DECK_KEY "chips.vrsc::poker.t_d_p9_deck"
 
- T_B_P1_DECK_KEY "chips.vrsc::poker.t_b_p1_deck"
- T_B_P2_DECK_KEY "chips.vrsc::poker.t_b_p2_deck"
- T_B_P3_DECK_KEY "chips.vrsc::poker.t_b_p3_deck"
- T_B_P4_DECK_KEY "chips.vrsc::poker.t_b_p4_deck"
- T_B_P5_DECK_KEY "chips.vrsc::poker.t_b_p5_deck"
- T_B_P6_DECK_KEY "chips.vrsc::poker.t_b_p6_deck"
- T_B_P7_DECK_KEY "chips.vrsc::poker.t_b_p7_deck"
- T_B_P8_DECK_KEY "chips.vrsc::poker.t_b_p8_deck"
- T_B_P9_DECK_KEY "chips.vrsc::poker.t_b_p9_deck"
 
- T_B_DECK_BV_KEY "chips.vrsc::poker.t_b_deck_bv"
- T_GAME_INFO_KEY "chips.vrsc::poker.t_game_info"
```
The point to note here is, since there is a single utxo attached to an ID, so only one can spend that ID. If multiple updates to an ID needs to happen in the same block, then while updating the ID we need to check in the mempool if there is any spend tx exists for the given ID, if so then that utxo needs to be spent to make an update to the ID. With the latest updates that is made to `getidentity` we now can able to view and update the IDs by taking mempool transactions into consideration. By passing `-1` to `getidentity` it will fetch the information from mempool. 

Since `getidentity` only returns the latest updated value of contentmultimap. So in order to retain the previous updates what we doing during the updates is, we read the existing contentmultimap data and then we simply appending the value that needs to be updated to it and in this way using `getidentity` we can view all the information that is updated. But later `getidentitycontent` API was introduced which provides all the updates made to the ID during a specific block interval. With the introduction of this API, the moment the table is started dealer updates the table with block number at which the table is started, so that during the game we take the value of the data from the block number updated in table ID to current block number. Once the game is done, the end block number is updated to the table ID.

### How Cashiers are updating the players info
-----------------------------------------------
When player makes the payin_tx by depositing funds to the cashier address, the cashier does the following thing upon receiving the payin_tx.
1. Cashier reads the data part of payin_tx, the data part contains dealer_id, table_id and primaryaddress.
2. Cashier reads the `t_table_info` and `t_player_info` keys from the table_id and checks if there exists any vacant spot left on the table and also checks if the player has deposited required funds to join the table and based on these checks cashier do one of the following:
  a. If there exists a vacant seat on the table, then the cashier adds the primaryaddress of the player to the `primaryaddresses` of the table and also updates the  `t_player_info` key with the information about the player by assigning a spot(with the pos_num) on the table. Now the cashier on incremental basis assigning the position number to the players, going forward players can choose where to sit on the table. 
  b. If the table is full or if the player hasn't made enough funds deposited to the cashiers address, then the cashier simply deposit funds back to the primaryaddress of the player. 
 3. No multiple join requests from the same primaryaddress are accepted, if by any reason player makes multiple join requests to the table only one is accepted. Ofcourse this is not a good enough check to prevent the player using multiple primaryaddress and competing to join the multiple spots on the same table. To avoid single player using multiple primaryadddress to join the table, going forward we will allow the players to register and provide an ID to the players and also provide an option to the dealer to allow only players with specific ID can join the table, that way we elimincate the possibility of same player taking multiple seats.

The next important aspect is how the player be communicated about the outcome of player_join. For which after making the payn_tx, the player continuously polls on the table_id for about five blocks to see if its primaryaddress is added to the primaryaddresses of the table_id and its information is updated in the `t_player_info` key of the `table_id` and if that happens player is joined the table. This can be done even more efficiently but for now I'm bruteforcing the search on table_id in the code.

### How multiple cashiers coordinate in updating the players info
------------------------------------------------------------------
While updating the players info in `t_table_info` key of the `table_id`, cashiers append 4 byte hash of the tx to the primary addresses as mentioned below:
```
{
	no_of_players: 2;
	primaryaddress_4_byte_tx_hash:0;
	primaryaddress_4_byte_tx_hash:1;
}
```
When cashiers update the `t_player_info` for a given `payin_tx` first they check for the duplicacy of the primaryaddress, if the primaryaddress is already found in the `primaryaddresses` key of the talbe_id, then the cashiers computes the tx hash and compare it with the tx hash appended to the primaryaddress. If tx hash match found, it simply means that the cashier is trying to update the `t_player_info` for the tx whose details are already been updated(by another cashier) and in that case the cashier node simply drop its updation process and does nothing and incase if the tx hash is different then the cashier simply deposit back the funds in that tx to the primaryaddress that the data part of that tx contains.

