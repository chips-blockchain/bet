## Configuring the dealer node
All the configurable options for the dealer are set in `dealer_setup.ini`, which is located at the path `bet/privatebet/config`. The default configuration settings are as follows: 
```
[dealer]
dcv_commission       = 0.5		   #This is the percentage of the pot amount that dealer takes home as commission.				
chips_tx_fee		 = 0.00050000  #The default tx_fee is `0.0001` dealer can set any amount greater than minimum tx_fee.
type				 = ipv4	   #When this set dealer only allows LN nodes running on torv3 to establish channels with it. This feature is not activated yet.
gui_host 			 = ""		   #If the GUI is hosted on the same backend node then it can be left blank, else the URL on which the GUI is hosted must be provided.
bet_ln_config        = N           # Set to N(no) if the dealer intends to record the game info on the CHIPS blockchain, else Y is the dealer intends to use lightning network to record game info in ln invoice.

[table]
max_players			 = 2  		   #This is the maximun number of players that dealer will allow them to join the table. Atm, dealer waits for this number of players to join before starting hand.
big_blind 			 = 0.001	   #If this value is set the table stake size is calculated based on this which is 200BB, if this value is not set the default value is 0.01.		
min_stake            = 20		   #The min table stake size is 20BB.
max_stake            = 100		   #The max table stake size is 100BB.	

[private table]
is_table_private     = N           #By default the dealer tables are public anyone can join the dealer table, if is_table_private set to "Y" then the dealer table is considered private and players has to provide the table_password in order to join the table.
table_password       = pangea      #If the table is private dealer must share this password with the players by any offline/online mechanism which is outside bet scope.
```
### To become a dealer
There are no conditions/constraints to become a dealer are set yet. If you have a machine with the static public IP you can simply host your own dealer node and become a dealer. You even can host private poker tables to play with closed groups, friends and family. 

In relation to this there is some discussion going on [here](https://github.com/chips-blockchain/bet/issues/193). It's still open, you can share your views on it.
