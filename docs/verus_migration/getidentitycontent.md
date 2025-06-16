# Verus API: `getidentitycontent`

The Verus API `getidentitycontent` was introduced to address the needs of BET. In BET, we have a situation where the `contentmultimap` of an ID gets updated by multiple entities. While reading, all the updated information in the `contentmultimap` should be retrieved in one go.

Initially, for incremental updates of the same key, we read the existing info from `contentmultimap`, append our data, and publish the whole data again. However, those incremental updates are not efficient. There will be tens of updates among tens of keys, and these updates might happen in the same block when the support to spend transactions from the mempool is available. For these reasons, we need an API to read a specific key of an ID that is not the latest. There is an API like `getaddresstxids`, which lists all the transactions associated with a specific ID. We need to do some workaround by parsing all these transactions associated with an ID to get the value of a specific key of a given ID.

Here is an example of what we are looking to achieve. Let's say I have the table ID `sg777_t`, and using four keys, I am storing the data as shown below. The key names used in this example are `player_info`, `cashier_id`, `player_1`, and `player_2`. All these keys are updated by different entities either at the same time or at different times. If I use `getidentity`, I can only retrieve the last updated key value. However, what I'm looking for is to get the last updated value of all these keys or for any specific key.

```json
verus -chain=chips10sec updateidentity '{"name": "sg777_t", "parent":"i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs", "contentmultimap":{
  "player_info": [
    {
      "num_players": 2,
      "primary_address_1": "id_1", # position of a player on the table
      "primary_address_2": "id_2"  # position of a player on the table
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

Now, with `getidentitycontent`, by mentioning the block interval, we retrieve all the updates that happened to the ID during this block interval.

