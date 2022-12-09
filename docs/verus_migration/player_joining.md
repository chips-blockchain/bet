Players Joining
---------------

This is an important phase in game setup. In the ID creation document we discussed about how cashiers and dealers are registered by the RA(Registration Authority). On high level here is how player get to the tables hosted by the dealer.

1. Players get to know about the list of avaiable dealers from `dealers.poker.chips10sec@`.
2. After getting the dealer names, players fetch the information about the tables its hosting from `<dealer_name>.poker.chips10sec@`.
3. After going through each table information, player comes to know about the table stats and details about whether if the table is empty or not. If player finds the table suitable then player deposit the funds needed to join the table to the `cashiers.poker.chips10sec@` and also in data part of the transaction the player mentions the following details:
```
{
  table_id:"some_table_id";
  primaryaddress: "The address which is owned by the player"; #This address can alse be configured in verus_player.ini config file.
}
```
4. Cashier nodes periodically checks if any deposits are made to the address `cashiers.poker.chips10sec@` using `blocknotify`. The moment cashiers detect any deposits made to the cashiers address, they immediately parse the data part of the tx, and add players `primaryaddress` mentioned in the data part of tx to the `table_id` which is also mentioned by the player in the same data part. Once after cashier adding the players `primaryaddress` to the `primaryaddresses` of the `table_id`, from that moment the player can be able to update corresponding `table_id`.
