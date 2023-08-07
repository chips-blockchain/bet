# Creation of ID's

All ID's in the bet ecosystem are created by the authorized entitites. Who are those authorized entities and on what basis they be chosen and how they bring trust into the ecosystem will be discussed in detail at later stages. 

There are few ID's which are created once and they be used for the lifetime, and there are some other ID's which are created in the context of game and once the game is played they serve no purpose and they only remain as archives. 

Based on the knowledge of the ecosystem we have and on how we going to organize the data in it we define the ID's and mostly create them at the launch of the specific game type and token. In the context of poker initially the following subID's are created under `poker.chips10sec@` and this list will be updated based on the need as we progress into the development.
1. cashiers
2. dealers

In the following sections of this document we discuss in detail about each of these ID's, like how they be created, what these ID's will be holding, who are the actors that manage these ID's, etc...

In the context of development we been using `revoke_2(idaddress: iSCt7uQBePbTSJUSPAuQqv3Qjw1YZmj6FX)` and `recovery_2(idaddress: iGXhgDHN7GBmbPPXcvNoj4Lc99pQEoA8Fj)`, which are created [here](./rec_rev.md) and these act as the revocation and recovery authorities for all the ID's which we create for the development purposes. 

For all these ID's to create there needs to be a control address, on mainenet launch we need to figure it out who can own this controladdress which is initially used to create the ID's or at the time of creation of ID's if the actors who owns the specific IDs in the bet ecosystem are identified then at the time of creating these IDs itself they will be created with the corresponding actors address as the control address for these IDs. Only the actors who has the corresponding private key of this control/primary address can update these IDs. 

## Cashiers

### What contains cashiers ID and who updates it

The cashers ID primaryaddresses contains the list of cashier nodes addresses owned by the cashier nodes. The `cashiers.poker.chips10sec@` is the multisig address that holds the funds during the game and handles the settlements and disputes of those funds by validating the game.
The `minimumsignatures` value is equal to `(n/2)+1`, where `n` is the number of cashier nodes or we can say number of `primaryaddresses`. On Verus the default tx expiration time is 20 blocks, so when spending from multisig IDs/addresses all the required co-signs must happen with in the 20 blocks time, else tx expires. For chips 20 blocks is approx to 200 seconds, atm we think this time window is sufficient for chips to complete msig txs.

When a specific node wants to be a cashier node it has to submit the request using some means(atm, via discord) to the community owners and the community owners can approve or deny that specific node be a cashier or not. The request should contain the address which is owned by the corresponding entity that make the request, and if the request is approved then that particualr address is added to the list of primaryaddresses of cashiers.

Here is example cashiers ID, which has 4 nodes and minimumsignatures is set to one, so any of the cashier can spend funds associated with this address.
```
# verus -chain=chips10sec getidentity cashiers.poker.chips10sec@
{
  "identity": {
    "version": 3,
    "flags": 0,
    "primaryaddresses": [
      "RNuB5mmZgPtsBLc2cX3DvubFEsbd84Thzi",
      "R9xBwrUH6kZPT86ysM7atYQU3CZSwz4TQY",
      "RJbFpxWSVer6WkFaHftSsR1TDyW4RXMkCt",
      "RRWRfU6EqgioiCbrxBD8uwsSRyK8Jxw8wh"
    ],
    "minimumsignatures": 1,
    "name": "cashiers",
    "identityaddress": "i4vGd5Aa23prxkPQbkZ7rHAoA7k7jRc5XY",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "systemid": "iLThsqsgwFRKzRG11j7QaYgNQJ9q16VGpg",
    "contentmap": {
    },
    "contentmultimap": {
    },
    "revocationauthority": "iSCt7uQBePbTSJUSPAuQqv3Qjw1YZmj6FX",
    "recoveryauthority": "iGXhgDHN7GBmbPPXcvNoj4Lc99pQEoA8Fj",
    "timelock": 0
  },
  "status": "active",
  "canspendfor": true,
  "cansignfor": true,
  "blockheight": 133110,
  "txid": "a0da9fa50c0b440a781beb11af982a3198e7da9e5dd3990d20d44a7e61278b9c",
  "vout": 0
}
```

## Dealers

The identity of the dealers looks as follows:
```
# verus -chain=chips10sec getidentity dealers.poker.chips10sec@
{
  "identity": {
    "version": 3,
    "flags": 0,
    "primaryaddresses": [
      "RNZFJQfWSwAu4QhM4AiPxLdBK9bFKb24n5"
    ],
    "minimumsignatures": 1,
    "name": "dealers",
    "identityaddress": "iAvw8ebtNggc2k6YHSG3rdbh75W9UHaVNT",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "systemid": "iLThsqsgwFRKzRG11j7QaYgNQJ9q16VGpg",
    "contentmap": {
    },
    "contentmultimap": {
    },
    "revocationauthority": "iAvw8ebtNggc2k6YHSG3rdbh75W9UHaVNT",
    "recoveryauthority": "iAvw8ebtNggc2k6YHSG3rdbh75W9UHaVNT",
    "timelock": 0
  },
  "status": "active",
  "canspendfor": true,
  "cansignfor": true,
  "blockheight": 133448,
  "txid": "9c480c29fb2afab94a1c105ad4270216f3087cda0a8ac3da172b221a64e15b6c",
  "vout": 0
}
```

Typically dealers hold the information about the list of the dealers available that are registered in the eco system. Right now we only be storing the dealer ID names as a string array, going forward the idea is to store more authorized statistics about the dealer and this info is updated by either the cashiers or by the authorized entities. 

If someone wants to become a dealer, they should make a request to register as a dealer. Since this registration is one time activity and dealer may get charged a fee for getting registered. 
All the dealer ID names should end with `_d` for readability and to avoid any potential conflicts with other ID's. The contentmultimap of dealer stores the array of strings and key using which this array of dealers stored is `chips.vrsc::poker.dealers`. The vdxfid for `chips.vrsc::poker.dealers` is shown as below:
```
 # verus -chain=chips10sec getvdxfid chips.vrsc::poker.dealers
{
  "vdxfid": "iSgEvATbNF3ZR6Kyj6nn8zVp3adPQxPnFJ",
  "indexid": "xXWMNxtgDZGE3GD1anSw7P2M5EeQH9biyX",
  "hash160result": "bca2ee47a59af321a73ed79907d0db8c111982fe",
  "qualifiedname": {
    "namespace": "iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP",
    "name": "chips.vrsc::poker.dealers"
  }
}
```

Lets say you have two dealers named `sg777_d` and `biz_d`, upon verification these dealer names updated to contentmultimap of the dealers ID as follows:
```
verus -chain=chips10sec updateidentity '{"name": "dealers", "parent":"i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs", "contentmultimap":{
      "iSgEvATbNF3ZR6Kyj6nn8zVp3adPQxPnFJ": [
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "sg777_d"
        },
		{
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "biz_d"
        }
      ]
    }
}'
```

After updating the dealer names the contentmultimap of dealers looks as follows:
```
# verus -chain=chips10sec getidentity dealers.poker.chips10sec@
{
  "identity": {
    "version": 3,
    "flags": 0,
    "primaryaddresses": [
      "RNZFJQfWSwAu4QhM4AiPxLdBK9bFKb24n5"
    ],
    "minimumsignatures": 1,
    "name": "dealers",
    "identityaddress": "iAvw8ebtNggc2k6YHSG3rdbh75W9UHaVNT",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "systemid": "iLThsqsgwFRKzRG11j7QaYgNQJ9q16VGpg",
    "contentmap": {
    },
    "contentmultimap": {
      "iSgEvATbNF3ZR6Kyj6nn8zVp3adPQxPnFJ": [
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "sg777_d"
        },
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "biz_d"
        }
      ]
    },
    "revocationauthority": "iAvw8ebtNggc2k6YHSG3rdbh75W9UHaVNT",
    "recoveryauthority": "iAvw8ebtNggc2k6YHSG3rdbh75W9UHaVNT",
    "timelock": 0
  },
  "status": "active",
  "canspendfor": true,
  "cansignfor": true,
  "blockheight": 138846,
  "txid": "8c0bc36196941c141ac8b19fe8e6aab8a6d0ee9504acee9dfde7dbab601ea0bc",
  "vout": 0
}
```

After optimization the data of contentmultimap contains only the bytearray, and the underlying bet has the logic to decode and encode that bytearray.

Once the dealer name gets registered with the dealers ID, the corresponding ID for that dealer name will be created and handover control of that ID to that specific dealer and where in which dealer updates information about its status, about the tables its hosting, about the fee it charges, etc... We will update the template of information what dealers store in its ID and the this template of dealer information is mapped to the key `chips.vrsc::poker.dealer`.

## Dealer
Upon the registration request from the dealer, the `registration authority(RA)` verifies the information about the dealer and upon acceptance, RA updates the dealer name to the dealers ID and creates the dealer ID with that specific name. 
Lets say for example a dealer with the name `sg777_d` applies to be a dealer, then RA adds `sg777_d` to dealers and creates the ID `sg777_d` with the `primaryaddress` provided by `sg777_d`.

After the creation of an ID with the name  of dealer with the name `sg777_d` by RA, it looks as follows:
```
# verus -chain=chips10sec getidentity sg777_d.poker.chips10sec@
{
  "identity": {
    "version": 3,
    "flags": 0,
    "primaryaddresses": [
      "RGgmpgoQcnptWEshhuhg3jGkYiotvLnswN"
    ],
    "minimumsignatures": 1,
    "name": "sg777_d",
    "identityaddress": "iHChMSKzFU7TcvURCjFwGzkBYLQ1MvrkYL",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "systemid": "iLThsqsgwFRKzRG11j7QaYgNQJ9q16VGpg",
    "contentmap": {
    },
    "contentmultimap": {
    },
    "revocationauthority": "iHChMSKzFU7TcvURCjFwGzkBYLQ1MvrkYL",
    "recoveryauthority": "iHChMSKzFU7TcvURCjFwGzkBYLQ1MvrkYL",
    "timelock": 0
  },
  "status": "active",
  "canspendfor": true,
  "cansignfor": true,
  "blockheight": 138976,
  "txid": "3276b236dff02e08988d6566d2244616c034af02bc3be740cf9b7662be504f35",
  "vout": 0
}
```
Here in the above example the primaryaddress `RGgmpgoQcnptWEshhuhg3jGkYiotvLnswN` is belongs to the dealer, i.e `sg777_d`, so once after the creation of dealer ID now the dealer can update it with the information specific to it.

### Dealer Contentmultimap Update
When the bet node starts as a dealer, it reads the information from `verus_dealer.ini` and update this information to the dealer contentmultimap. Basically here in this configuration file the dealer mention its ID and the information about the tables its hosting. Right now we limit our discussion to one dealer hosting one table.
The sample contents of `verus_dealer.ini` looks as follows:
```
[verus]
id = sg777_d   #This is the ID for which the dealer owns the primaryaddress and to which it updates the info.

[table]
max_players          = 2               #This is the maximun number of players that dealer will allow them to join the table. Atm, dealer waits for this number of players to join before starting hand.
big_blind            = 0.001           #If this value is set the table stake size is calculated based on this which is 200BB, if this value is not set the default value is 0.01.
min_stake            = 20              #The min table stake size is 20BB.
max_stake            = 100             #The max table stake size is 100BB.
table_id 	     = sg777_t         #This is the table ID to which all the game info is to be committed. This table info is controlled by the players. 	
```

How this information is processing from the configuration file to the ID, for every configuration data there mostly be an underlying structure that holds the information. Here in this the `struct` named `table` holds this information and is defined in the code as below:
```
struct table {
	uint8_t max_players;
	struct float_num min_stake;
	struct float_num max_stake;
	struct float_num big_blind;
	char table_id[16];
};
```
Note, here there isn't any support to store the float values into the ID's, so we represent the float values using `struct float_num` which basically defined as follows:
```
struct float_num {
	uint32_t mantisa : 23;
	uint32_t exponent : 8;
	uint32_t sign : 1;
};
```
Once the information is stored from the config file to the underlying structure, then the struct data is converted to hex and is stored into the ID. After storing the above information into the dealer ID `sg777_d` it looks as follows:
```
# verus -chain=chips10sec getidentity sg777_d.poker.chips10sec@
{
  "identity": {
    "version": 3,
    "flags": 0,
    "primaryaddresses": [
      "RGgmpgoQcnptWEshhuhg3jGkYiotvLnswN"
    ],
    "minimumsignatures": 1,
    "name": "sg777_d",
    "identityaddress": "iHChMSKzFU7TcvURCjFwGzkBYLQ1MvrkYL",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "systemid": "iLThsqsgwFRKzRG11j7QaYgNQJ9q16VGpg",
    "contentmap": {
    },
    "contentmultimap": {
      "iSgEvATbNF3ZR6Kyj6nn8zVp3adPQxPnFJ": [
        {
          "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c": "02cf30c7cdcc4c3e0000803f6f12833a"
        }
      ]
    },
    "revocationauthority": "iHChMSKzFU7TcvURCjFwGzkBYLQ1MvrkYL",
    "recoveryauthority": "iHChMSKzFU7TcvURCjFwGzkBYLQ1MvrkYL",
    "timelock": 0
  },
  "status": "active",
  "canspendfor": true,
  "cansignfor": true,
  "blockheight": 144569,
  "txid": "1abd56c4135fc498695c87ac6c90402eacb6fbee9d0fcd2c6b184e6fd7c555bc",
  "vout": 0
}
```
Since we storing only data into ID, the data may not look readable but anyways that's not our aim at this point. Possibly we can have our own explorer where in which we integrate our data decoders but that's for later. 

I'll break this page here, will discuss more details about tables and how players joins the tables [here](./player_joining.md).
