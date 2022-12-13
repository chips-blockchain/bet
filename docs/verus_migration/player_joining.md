Players Joining
---------------

This is an important phase in game setup. In the ID creation document we discussed about how cashiers and dealers are registered by the RA(Registration Authority). Here we see more details about the tables and how player finds and joins the tables. 

### Table

The initial thought was dealers create the tables whenever the dealer want to host the table. Some drawbacks we saw with that approach are:
1. The control should be given to the dealer to create the ID's, and we definetely don't any spam of ID's incase if the intentional behavior of any entity changed to malicious.
2. If a new table ID gets created for game that is played then it won't be possible to use catchy names for the tables. The fixed table names provides lots flexibility either in hosting them for the dealers and for the players in configuring them.
3. With new table ID's for every game its difficult host the private tables.

So by observing the limitations, first we taken away(not given) the ability to create the ID's to the dealers, instead dealer can make a request to the cashiers about the creation of table ID's at the time dealer registration or later. Dealer can make five such requests to the cashiers, meaning dealer can host five tables at any given time. Ofcouse `5` is not the hard cap we setting on the number of tables and it can be subject to revision based on the feedback we receive. Likewise we will also allow dealer to make a request to revoke and revocer the table ID.

The good thing with the fixed table names is that, players can configure them in their `verus_player.ini` configuration file and can be choosy about the table the player wants to join. 

### How players find the table

With that intro about tables, here are the steps that player follows to finds out and joins the table:

1. Players get to know about the list of avaiable from dealers ID `dealers.poker.chips10sec@`.
2. After getting the dealers ID's, players fetch the information about the tables that a specific is hosting from the ID `<dealer_name>.poker.chips10sec@`.
3. After going through each table information, player comes to know about the table details like min stake, big blind, empty or not, etc... If player finds the table suitable then player deposit the funds needed to join the table to the `cashiers.poker.chips10sec@` and also in data part of the transaction the player mentions the following details:
```
{
  table_id:"some_table_id";
  primaryaddress: "The address which is owned by the player"; #This address can alse be configured in verus_player.ini config file.
}
```
4. Cashier nodes periodically checks if any deposits are made to the address `cashiers.poker.chips10sec@` using `blocknotify`. The moment cashiers detect any deposits made to the cashiers address, they immediately parse the data part of the tx, and add players `primaryaddress` mentioned in the data part of tx to the `table_id` which is also mentioned by the player in the same data part. Once after cashier adds the players primaryaddress to the `primaryaddresses` of the table_id, from that moment the player can be able to update corresponding `table_id`.

### What goes into table

Off all the ID's we have table is a very complex ID and this is where all the game info goes into. The complete map of contents that goes into the table is not yet fully identified and at this moment I'll write about the initial thought process and we improve further upon it.

All the actors like players, dealers, cashiers/bvv updates the table ID at different stages during the game. The main tasks that get accomplished with the data on table ID are deck shuffling, game play and final settlement. The nature of the data that flows to handle all these tasks is significantly different and data that is used to accomplish one task may not be relevant on other task. For these reasons we need to define some keys that are very specific to accomplish a specific task and some keys which may be relavant across all the tasks. But there is catch here, since there is a single utxo attached to an ID, so only one can spend that ID. If multiple updates to an ID needs to happen in the same block, then while updating the ID we need to check in the mempool if there is any spend tx exists for the given ID, if so then that utxo needs to be spent to make an update to the ID. Since soon we going to have an API that spends the ID from the utxo's of mempool that enables us to make multiple updates to the ID in the same block.

For incremental updates of the same key, we read the existing info from contentmultimap and we append our data publish whole data again. But those incremental updates are not efficient here and since there is going to be tens of updates among tens of keys and these updates might going to happen in the same block when the support to spend tx from mempool is available. For these reasons we need to have an API's to read a specific key of an ID which is not the latest. There is an API like `getaddresstxids` which list all the tx's associated with a specific ID, need to do some workaround which by parsing all these tx's associated with an ID to get the value of a specifc key of a given ID. 

Here is an example about what we are looking to have. Lets say I have the table ID `sg777_t` and to which lets say using four keys im storing the data as shown below, and the keys names used in this example are `player_info`, `cashier_id`, `player_1` and `player_2`. All these keys are updated by different entities either at the same time or at differnt times. So basically if I do getidentity I can only retrive the last updated key value, but what I'm looking is to get the last updated value of all these keys or for any specific key.
```
verus -chain=chips10sec updateidentity '{"name": "sg777_t", "parent":"i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs", "contentmultimap":{
      "player_info": [
        {
          num_players : 2;
          "primary_address_1": id_1; #position of a player on the table.
          "primary_address_2": id_2; #position of a player on the table.
        }
      ],
      "cashier_id": [
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "sg777_c"
        }
      ],
      "player_1": [
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "some_data_of_player1"
        }
      ],
      "player_2": [
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "some_data_of_player2"
        }
      ]
    }
}' 
```  

When player makes the payin_tx by depositing funds to the cashier address, the cashier does the following thing upon receiving the payin_tx.
1. Cashier reads the data part of payin_tx, the data part contains table_id and primaryaddress.
2. Cashier reads the `table_info` and `player_info` keys from the table and figure it out whats the max players allowed and see if this player can be accomodated in the table or not.
  a. If the seats are left on the table, then the cashier adds the primaryaddress of the player to the `primaryaddresses` of the table and in `player_info` cashier updates the information about the player along with its position number on the table. 
  b. If the table is full, then the cashier simply deposit funds back to the primaryaddress mentioned by the player. 
 3. No two join requests from the same primaryaddress are accepted, so that way if a player makes multiple join requests accidentally only one gets accepted. To avoid single player using multiple primaryadddress to join the table, going forward we will allow the players to register and provide an ID to the players and also provide an option to the dealer to allow only players with specific ID can join the table, that way we can elimincate the possibility of same player taking multiple seats.

In either case we should communicate that outcome player_join to the player. 