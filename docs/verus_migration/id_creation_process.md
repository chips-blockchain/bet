# ID Creation Process in the Bet Ecosystem

All IDs in the bet ecosystem are created by authorized entities. The criteria for selecting these entities and how they establish trust within the ecosystem will be discussed in detail in subsequent sections.

There are two types of IDs: 
1. Permanent IDs, which are created once and used for a lifetime.
2. Temporary IDs, which are created for specific games and serve no purpose after the game ends, remaining only as archives.

Based on our understanding of the ecosystem and data organization, we define and create IDs at the launch of specific game types and tokens. For poker, the following sub-IDs are initially created under `poker.chips10sec@` and will be updated as development progresses:
1. cashiers
2. dealers

In the following sections, we will discuss each of these IDs in detail, including their creation, contents, and management.

During development, we use `revoke_2(idaddress: iSCt7uQBePbTSJUSPAuQqv3Qjw1YZmj6FX)` and `recovery_2(idaddress: iGXhgDHN7GBmbPPXcvNoj4Lc99pQEoA8Fj)`, created [here](./rec_rev.md), as the revocation and recovery authorities for all development IDs.

To create these IDs, a control address is required. Upon mainnet launch, we need to determine who will own this control address. If the actors who will own specific IDs in the bet ecosystem are identified at the time of creation, the IDs will be created with the corresponding actors' addresses as the control address. Only actors with the private key of this control/primary address can update these IDs.

## Cashiers

### Contents and Updates of Cashiers ID

The primary addresses of the cashiers ID contain the addresses of cashier nodes. The `cashiers.poker.chips10sec@` is a multisig address that holds funds during the game and handles settlements and disputes by validating the game. The `minimumsignatures` value is atleast `(n/2)+1`, where `n` is the number of cashier nodes. On Verus, the default transaction expiration time is 20 blocks (approximately 200 seconds for chips), which we believe is sufficient for completing multisig transactions.

To become a cashier node, a request must be submitted to the community owners, who can approve or deny the request. The request should include the address owned by the entity making the request. If approved, the address is added to the list of primary addresses of cashiers ID.

Example of a cashiers ID with four nodes and a minimum of three signatures required:
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
    "minimumsignatures": 3,
    "name": "cashiers",
    "identityaddress": "i4vGd5Aa23prxkPQbkZ7rHAoA7k7jRc5XY",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "systemid": "iLThsqsgwFRKzRG11j7QaYgNQJ9q16VGpg",
    "contentmap": {},
    "contentmultimap": {},
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

### Dealers ID Structure

Example of a dealers ID:
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
    "contentmap": {},
    "contentmultimap": {},
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

The `dealers.poker.chips10sec@` sub-ID holds information about registered dealers in the poker ecosystem. Currently, only dealer ID names are stored as a string array. In the future, more statistics about dealers, such as availability and number of tables hosted, will be stored and updated by cashiers or authorized entities.

To become a dealer, a request must be submitted, and a fee may be charged for registration. Dealer ID names should end with `_d` for readability and to avoid conflicts. The `contentmultimap` of dealers stores dealer names as an array of strings, mapped to the key `chips.vrsc::poker.dealers`.

Example of updating dealer names:
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

After updating, the dealer names in the `contentmultimap` of dealers look as follows:
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
    "contentmap": {},
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

After optimization, the `contentmultimap` contains only the byte array, and the bet system has the underlying logic to decode and encode data into a byte array.

If all preconditions (yet to be defined) are met for a dealer request, a sub-ID under `poker.chips10sec@` is created with the name and control address provided in the dealer request. Only the dealer can update this ID, maintaining information about its status, hosted tables, dealer fee, etc. This template of dealer information is mapped to the key `chips.vrsc::poker.dealer`.

## Dealer Registration Process

Upon a dealer's registration request, the Registration Authority (RA) verifies the dealer's information. If accepted, the RA updates the dealer name in the dealers ID and creates the dealer ID with the specified name.

A sample dealer ID is shown below. It resembles a standard ID but includes keys in the `contentmultimap` to store dealer-specific information.
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
    "contentmap": {},
    "contentmultimap": {},
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

In this example, the primary address `RGgmpgoQcnptWEshhuhg3jGkYiotvLnswN` belongs to the dealer `sg777_d`. After the dealer ID is created, the dealer can update it with specific information.

Once registered, a dealer can request to register table names, which should end with `_t`. Each table name registration incurs a nominal fee. When RA creates the table IDs, it grants the dealer revocation/recovery authority over the tables. Players send join requests to the table, and the dealer processes these requests, adding player addresses to the table's primary addresses. During the game, players update the table with game information.

### Dealer Contentmultimap Update

When a bet node starts as a dealer, it reads information from `verus_dealer.ini` and updates this information in the dealer `contentmultimap`. The configuration file includes the dealer ID and information about the tables it hosts.

Sample `verus_dealer.ini`:
```
[verus]
id = sg777_d   # Dealer's ID

[table]
max_players          = 2               # Maximum number of players allowed to join the table
big_blind            = 0.001           # Table stake size based on this value (200BB)
min_stake            = 20              # Minimum table stake size (20BB)
max_stake            = 100             # Maximum table stake size (100BB)
table_id             = sg777_t         # Table ID for game info
```

The configuration data is stored in a `struct` named `table`:
```
struct table {
    uint8_t max_players;
    struct float_num min_stake;
    struct float_num max_stake;
    struct float_num big_blind;
    char table_id[16];
};
```

The `struct float_num` represents float values:
```
struct float_num {
    uint32_t mantisa : 23;
    uint32_t exponent : 8;
    uint32_t sign : 1;
};
```

The struct data is converted to hex and stored in the dealer ID. After storing the information, the dealer ID `sg777_d` looks as follows:
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
    "contentmap": {},
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

Although the stored data may not be readable, our goal is to store the data efficiently. We may develop an explorer to integrate data decoders in the future.

For more details about tables and player joining processes, refer to [this document](./player_joining.md).
