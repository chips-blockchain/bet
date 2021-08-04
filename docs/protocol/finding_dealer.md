# Finding the dealer by the playing node

In this article we findout the details about how the players discover the dealers. Since we operating in a fully decentralized environment there isn't any prior communication or knowledge about which other nodes existed in the system.

### The steps involved in this process are
1. Dealer advertises its presence to the cashier nodes.
2. Cashier nodes store this info.
3. Player fetch this info from the cashiers

### Dealer advertising its presence
In the system the cashier nodes are predetermined ones and this info is stored in the `cashier_nodes.json` file which is present at the path `/bet/privatebet/config`. And the contents of this file looks as below:
```
[{
		"pubkey":	"0377653051fe9dd919ad2d70422f692a43c46ac72fcfe2dfd3302ec30dd16aaf40",
		"ip":	"159.69.23.28"
}, {
		"pubkey":	"039ee4a07033df0add41e23ddc3865061e8eb97d719d843d5db2f3371b61a4eb34",
		"ip":	"159.69.23.29"
}, {
		"pubkey":	"0342e186006c560fe9bdc7bc0440b6fbfa46a4a852f5d4b28d67ab712ebf2e4744",
		"ip":	"159.69.23.30"
}, {
		"pubkey":	"03d0a6326bcf918aed07557462963953f2125a11cfcb9a7b05630f930e8e554956",
		"ip":	"159.69.23.31"
}]
```
So when the dealer starts this information is already been available to the dealer. Since we need to run the dealer on the node with static IP for time being while running the delaer node we pass the IP of the node in which the dealer is running from the command line.
Here is  how we run the dealer
```
cd
cd bet/privatebet
./bet dcv ipv4_address_of_this_node
```
And this dealer info is advertised to all the cashier nodes, the json content of the message which the dealer sends to cashier nodes looks as below:
```
{
	"method":	"dealer_info",
	"ip":	"159.69.23.30"
}
```
With this the dealer part is done in starting the node.

### Cashier node storing the dealer info

Once the cashier nodes receives this info, it stores this info in the `dealers_info` table of the `pangea.db` say for example when the dealer is started on the node `159.69.23.30` and this ip info of the dealer is available at all cashier nodes after stroing this info into the DB.
Below are the commands which I ran on the cashier node and you can see this dealer info
```
root@sg777-1 ~/.bet/db # sqlite3 pangea.db 
SQLite version 3.22.0 2018-01-22 18:45:57
Enter ".help" for usage hints.
sqlite> .tables
c_tx_addr_mapping   cashier_tx_mapping  dcv_tx_mapping      player_game_state 
cashier_game_state  dcv_game_state      dealers_info        player_tx_mapping 
sqlite> select * from dealers_info
   ...> ;
sqlite> select * from dealers_info;
159.69.23.30
sqlite> .quit
```
### Player fetch this info from the cashiers
From step2 we now know that cashier nodes contains all the active dealers info. When the player node gets started it sends the `rqst_dealer_info` to cashier node and the cashier node takes the data from `dealers_info` table and checks with the dealer if it's active or not and then cashier node sends list of all the active dealers info to the player node.

The playing node picks the dealer and sends the `player_join` request to join the table hosted by the dealer.
