History of getidentitycontent
------------------------------
The verus API `getidentitycontent` was introduced to addressed the needs of bet. In bet we have a situation where the contentmultimap of an ID get updated by multiple entities but while reading all the updated information to the contentmultimap should be retrieved in one go. 

Initially for incremental updates of the same key, we read the existing info from contentmultimap and we append our data publish whole data again. But those incremental updates are not efficient here and since there is going to be tens of updates among tens of keys and these updates might going to happen in the same block when the support to spend tx from mempool is available. For these reasons we need to have an API's to read a specific key of an ID which is not the latest. There is an API like `getaddresstxids` which list all the tx's associated with a specific ID, need to do some workaround which by parsing all these tx's associated with an ID to get the value of a specifc key of a given ID. 

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

Now, with `getidentitycontent` by mentioning the block interval we retrive all the updates that happen to the ID during this block interval.
