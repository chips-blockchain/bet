Dealer can do the table configuration in the `dealer_config.json` which is located in the `bet/privatebet` directory. 

```
{
max_players: 2
table_stack_in_chips = 0.01;
chips_tx_fee = 0.0005;

}
```

### max_players
Number of players which are allowed on the dealer table.

### table_stack_in_chips
This indicates how much amount of CHIPS needed in order for the players to join the table. Say suppose if the dealer sets `table_stack_in_chips = 0.01`, it means the player node wallet should contain atleast `table_stack_in_chips+chips_tx_fee` of CHIPS in order to join the table, so it means the player node should have atleast `0.0105` CHIPS.

Here one more thing to note down is, there is something called as `chips_to_stack_conversion_factor`, i.e defined as `1000`. Means by depositing `0.01` CHIPS player gets `0.01 * 1000`, i.e `10` table stack and that will be displayed on the GUI.

### chips_tx_fee
The minimum and the default tx_fee is `0.0005`, dealer can set higher chips_tx_fee so that mining nodes will priotize the transactions played on the dealer table. 

