# Verus Migration Documentation

## Overview

In the current setup, communication between nodes/entities occurs over sockets. Game information is stored in process memory, a local database, and on-chain. Process memory contains the game state and card information necessary for gameplay. The local database holds transaction information and game state for bookkeeping and dispute resolution. On-chain information allows players to join tables, settle funds during the game, and resolve disputes.

### Issues with Socket Communication

The primary issue with socket communication is its dependency on IP addresses. Static public IP addresses are costly, and NATing often causes node discovery issues. Player disconnections are problematic due to poorly defined data sets for rejoining, and publisher-side disconnections (e.g., dealer) complicate game state recovery. Using VDXF IDs to store data on the blockchain simplifies maintaining and retrieving game state, improving reliability in handling player disconnections and disputes. Unlike IP-based node discovery, VDXF IDs use registered identities on the Verus blockchain.

### Costs and Latency with VDXF IDs

Relying on VDXF IDs incurs costs, as each update requires a blockchain transaction, incurring a transaction fee and block latency. For poker, a player node may make 20-25 data updates per game, costing `25 * tx_fee`. Although CHIPS transaction fees are low, the cost per game is minimal. Verus APIs help mitigate block latency by fetching transaction info from the mempool and updating IDs considering relevant mempool transactions.

## Benefits of Verus VDXF IDs

VDXF IDs function as a well-organized, restricted public database for storing and retrieving data. Data is organized under IDs and sub-IDs with multiple nesting levels, stored in key-value pairs. Each ID is associated with a UTXO, and updating an ID involves spending the associated UTXO. When multiple parties attempt to update an ID, the first submitted transaction is accepted, and others are rejected.

### Characteristics of ID Updates

1. Concurrent updates to the same or different keys of an ID in the same block result in only the first transaction being accepted.
2. For concurrent updates, check for spent transactions in the mempool and make a spend transaction on top of it to update the ID in the same block. 
3. Design data structures to handle concurrent updates with multiple IDs or sub-IDs to avoid blocktime delays.
4. `getidentity` returns data updated with the latest UTXO. Use  [`getidentitycontent`](./getidentitycontent.md) to retrieve data updated over a period.

### Data Representation and Storage

In BET, data is represented in JSON format and exchanged over sockets. With VDXF IDs, data storage on the blockchain must be space-efficient. Data is encoded as compact structures to minimize storage space. Currently, structures are used to encode data for some APIs. Once the contents of each update are clear, data can be mapped to a structure, encoded in hex, and stored.

### ID Creation and Management

BET aims to support various card games and betting activities. Game-specific IDs are created to group related data. For example, `poker.chips@` for poker, `blackjack.chips@` for blackjack, and `bet.chips@` for betting. Sub-IDs under these main IDs organize data, such as `dealer.poker.chips` for dealer information and `cashier.poker.chips` for cashier information.

#### ID Creation and Key Management

1. [ID Creation Process](./id_creation_process.md)
2. [Keys and Data Management](./ids_keys_data.md)

### Updating IDs

Verus provides APIs to list all information attached to an ID along with its history (`getidentitycontent`). Currently, `getidentity` fetches the latest information updated to the `contentmultimap`. For incremental updates, read the latest value using `getidentity`, append the new value, and update using `updateidentity`. With `getidentitycontent`, incremental updates can be made without appending to previously updated data.

#### Example

To add a new IP to the cashier list, follow these steps:

1. **Current cashier list:**
    ```json
    chips.vrsc::bet.cashiers
    {
        "ips": ["1.2.3.4", "a.b.c.d"]
    }
    ```

2. **Add the new IP (`w.x.y.z`) to the list:**
    ```json
    chips.vrsc::bet.cashiers
    {
        "ips": ["1.2.3.4", "a.b.c.d", "w.x.y.z"]
    }
    ```

This updated list now includes the new IP address.
### Conversion Between CHIPS and Other Currencies
```markdown
If you use liquidity baskets on-chain, you can convert between CHIPS and any currencies that were defined on any chain in the Verus PBaaS network and Ethereum + any ERC20 as well. You can do all of this on the CHIPS chain and all conversion fees will be split 1/2 to LPs and 1/2 to miners/stakers. You can use CHIPS or any other currency you want to support in your game, even a liquidity basket currency. Any currency definition can be exported from one chain to another, converted at the fairest possible rate on-chain with arbitrage hooks for miners and stakers, and used anywhere on the network. New currencies or liquidity basket currencies defined on CHIPS, just like those defined on Verus, can be sent over to Ethereum as well and will automatically be ERC20s on Ethereum. All the conversions will leave 1/2 the fees in the liquidity basket currencies, raising their values relative to reserves, and the other 1/2 will automatically buy CHIPS from the liquidity basket to pay miners and stakers.
```

### Challenges

Current challenges include implementing a heartbeat protocol. Previously, nodes sent or responded to `live` messages over sockets. With VDXF IDs, options for implementing the heartbeat protocol are being explored. The existing timer functionality in the GUI may be incorporated with VDXF IDs to enforce response times, allowing nodes to monitor each other's status.
