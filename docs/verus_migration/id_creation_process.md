# Creation of ID's

All ID's in the bet ecosystem are created by the authorized entitites. Who are those authorized entities and on what basis they be chosen and how they bring trust into the ecosystem will be discussed in detail at later stages. 

There are few ID's which are created once and they be used for the lifetime, and there are some other ID's which are created in the context of game and once the game is played they serve no purpose and they only remain as archives. 

Those few standard ID's are created at the time of launch of a specific game type and token. In the context of poker initially the following ID's are created and this list will be updated based on the requirements as we progree into the development.
1. cashiers
2. dealers

In the following sections of this document we will in detail about each of these ID's, like how they be created, what these ID's will be holding, who are the actors that manage these ID's, etc...

In the context of development we been using `revoke_2(idaddress: iSCt7uQBePbTSJUSPAuQqv3Qjw1YZmj6FX)` and `recovery_2(idaddress: iGXhgDHN7GBmbPPXcvNoj4Lc99pQEoA8Fj)`, which we created [here](./rec_rev.md) as the revocation and recovery authorities for all the ID's which we create for the development purposes. 

For all these ID's to create there needs to be a control address, typically we need to figure it out who can owns that controladdress that initially been used to create the ID's. But this control address may not have much significance in all the ID's after the initial creation because the ID's primaryaddresses are going to be updated with the addresses of the corresponding actors that involve in updating the ID's.

## Cashiers

### What contains cashiers ID and who updates it

The cashers ID primaryaddresses contains the list of cashier nodes addresses owned by the cashier nodes. The `cashiers.poker.chips10sec@` is the multisig address that holds the funds during the game and handles the settlements and disputes of those funds by validating the game.
The `minimumsignatures` value is equal to `n/2+1`, where `n` is the number of cashier nodes or we can say number of `primaryaddresses`.

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
All the dealer ID names should end with `_d` for readability and to avoid any potential conflicts with ID's. Lets say you have two dealers named `sg777_d` and `biz_d` then the contentmultimap of the dealers ID looks as follows:


