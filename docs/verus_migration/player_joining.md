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

All the actors like players, dealers, cashiers/bvv updates the table ID at different stages during the game. The main tasks that get accomplished here are deck shuffling, game play and final settlement. The nature of the data that flows to handle all these tasks is significantly different and data that is used to accomplish one task may not be relevant on other task. For these reasons we need to define some keys that are very specific to accomplish a specific task and some keys which may be relavant across all the tasks.

For incremental updates of the same key, we read the existing info from contentmultimap and we append our data publish whole data again. But those incremental updates are not efficient here and since there is going to be tens of updates among tens of keys and these updates happen concurrently, so its just not reliable to read all, append and update. For these reasons we are specifically looking for an API from verus and if that doesn't exists we need to figure it out a way where in which we can only retrive the data associated with the specific key value.

Here is an example about what we are looking to have. Lets say I have the table ID `sg777_t` and to which lets say using five keys im storing the data as shown below, and the keys names used in this example are `table_info`, `player_info`, `cashier_id`, `player_1` and `player_2`. All these keys are updated by different entities either at the same time or at differnt times. So basically if I do getidentity I can only retrive the last updated key value, but what I'm looking is to get the last updated value of all these keys or any specific key that I pass.
```
verus -chain=chips10sec updateidentity '{"name": "sg777_t", "parent":"i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs", "contentmultimap":{
      "table_info": [
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "max_players:2",
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "players_occupied:2"
        }
      ],
      "player_info": [
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "id:1",
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "id:2"
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
