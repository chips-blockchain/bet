Players Joining
# Player Joining Process

This phase is crucial in the game setup. In the [ID Creation Document](./id_creation_process.md), we discussed how cashiers and dealers are registered by the Registration Authority (RA). Here, we delve into the details of how players find and join tables.

## Table Setup

Any dealer can host multiple tables using the same ID repeatedly. Key points to note:

1. Dealers control the creation and registration of table IDs. While there are concerns about ID spam, more IDs can enhance the economy and reduce hassle.
2. Players must specify the table name in their `verus_player.ini` config file to join a specific table. Reusing the same ID allows players to avoid modifying their config file.
3. Players can list all available tables using the command `./bet list_tables`.

## Finding Tables

Players can find registered tables using the following methods, with the recommended approach being the `list_tables` API (`./bet list_tables`):

## Approach 1

### Step 1: Get the List of Dealers

You can obtain the list of available dealers using the `list_dealers` command or by parsing dealer IDs.

- **List available dealers using `list_dealers`:**
	```sh
	./bet list_dealers
	```

- **Parse dealer IDs using `print_id`:**
	```sh
	./bet print_id dealers dealers
	```

### Step 2: Parse the Dealer ID

Once you have the dealer ID from Step 1, you can parse it using the `print_id` command:
	```sh
	./bet print_id <dealer_id> dealer
	```

## Approach 2

### List All Available Tables

All the steps mentioned above can be accomplished using the `list_tables` command, which lists all available tables:
	```sh
	./bet list_tables
	```

### View Table State and Game Information

You can view the contents of a specific table using the `print_id` command:
	```sh
	./bet print_id sg777_t table
	```

## Table Content

The table ID is complex and holds all game information. As development progresses, more details will be updated. Key actions like deck shuffling, gameplay, and settlement rely on the table ID. Information is grouped under the table ID content multimap with keys detailed in [All About IDs and Keys](./ids_keys_data.md#table-id).

## Updating Player Info

When a player chooses a dealer and table, they make a pay-in transaction to the cashier's address with the following details:
- dealer_id
- player_id
- `primaryaddress` (address owned by the player)

The cashier node validates the transaction and updates the table ID if all checks pass. If any check fails, the transaction is reversed, and funds are returned to the player.

## Player Confirmation

After making the pay-in transaction, the player checks if their primary address has been added to the table ID within 5 blocks. If added, the player fetches their player ID from the `t_player_info` key. If not, the player realizes they were not allowed to join and can raise a dispute if funds are not returned.

## Cashier Coordination

Cashiers append a 4-byte hash of the transaction to the primary addresses in the `t_table_info` key. They check for duplicacy and update the `t_player_info` key accordingly. If a duplicate is found, the cashier drops the update process.

### Example

While updating the players' info in `t_table_info` key of the `table_id`, cashiers append a 4-byte hash of the transaction to the primary addresses as mentioned below:
```
{
	no_of_players: 2;
	primaryaddress_4_byte_tx_hash:0;
	primaryaddress_4_byte_tx_hash:1;
}
```
When cashiers update the `t_player_info` for a given `payin_tx`, they first check for the duplicacy of the primary address. If the primary address is already added to the `primaryaddresses` key of the table_id, then the cashiers compute the transaction hash and compare it with the transaction hash appended to the primary address in the `t_player_info` key of the table_id. If a matching transaction hash is found, it means that the player details have already been updated by another cashier, and the cashier node drops its update process. If the transaction hash is different, the cashier deposits the funds in that transaction back to the primary address present in the data part of the same transaction.

## Handling Disconnections

On disconnections, players can either rejoin or discontinue. If a player gets disconnected and cannot rejoin within a stipulated time, the dealer decides the next steps based on the game's stage. The dealer may pause the game, wait for the player to reconnect, or proceed by removing the disconnected player.

To rejoin, players can use the command:
```sh
./bet rejoin player
```
This command attempts to reconnect the player to their previous table, restoring their state to continue playing. More details are available in [Player Rejoining](./player_rejoining.md).


