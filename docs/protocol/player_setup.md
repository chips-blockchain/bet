Here we see detailed info about compiling, configuring, playing and raising disputes with the player node.

## Compiling player node
In order to setup the player node follow either the [basic compilation guide](./ubuntu_compile.md) or [detailed compilation guide](./compile.md). 

## Configuring the player node

All the configurable options for the player can be set in `player_setup.ini` file, which is located at `bet/privatebet/config` path. The default content of this file is as follows:
```
[player]
max_allowed_dcv_commission = 5      #This is the max percentage of the dealer commision that the player is willing to pay, if the dealer sets the commission higher than this then the backend node will exit.
type                       = torv3  #This will enforce LN and CHIPS run on onion address, atm this will be ignored.

[gui]                               
#These are GUI's hosted by the cashier nodes, player can pick any of them in order to connect to its backend from these.
cashier-1 = http://141.94.227.65:1234/
cashier-2 = http://141.94.227.66:1234/
cashier-3 = http://141.94.227.67:1234/
cashier-4 = http://141.94.227.68:1234/
cashier-5 = http://159.69.23.30:1234/

[private table]
is_table_private     = N           #By default this is set to "N", meaning that player can only join the public tables run by the dealer. If this is set to "Y" then it means player only joins the private tables from the dealer.
table_password       = pangea      #If the player wishes to join the private table, then the player has to set the password that is received from the dealer by any offline/online escrow mechanism, which is outside scope of bet. 
```

## Running the player node
```
cd bet/privatebet
./bet player
```
## Playing with the player node

Once the backend is started, it look for the dealer and get the table stack from the dealer and make the payin_tx which is locked at the cashier nodes. All of this happen without any user intervention.

Now to connect to the backend, either the player can use the GUI hosted by the dealer or the player can run its own GUI and through which it can communicate with the backend. It's badically the responsibility of the dealer to run the GUI backend and share the weblink with the players to join. 

Say suppose in my case when I ran the player backend, in the console log I see a message `The dealer is :: 159.69.23.30`. Player can take this and from the GUI player enters `http://159.69.23.30:1234/` which pops up the GUI like below:

![image](https://user-images.githubusercontent.com/8114482/139268469-57240190-1be5-4624-a911-b417e1d7f94e.png)

Then player clicks on `Private Table --> Player` and enters the IP on which the backend node is running in the checkbox.
> This IP must be public, if the IP is not public then the GUI must be accessed from the local machine on which the player backend node is running and from GUI player connect to backend by entering the localhost IP.

In my case `141.94.227.65` is the node IP on which i'm running the player node, so I enter this IP in the checkbox and click on `Set Nodes`. Since in this scenario dealer set `max_players = 3` I'm able to view a table with 3 empty seats as below:

![image](https://user-images.githubusercontent.com/8114482/139269790-59f53e2b-11da-4bc6-a506-2ecefd0ac114.png)

Then I can choose any seat and click on `SIT HERE`, at this point I joined the table and simply wait for the other players to join the table like in the below image:

![image](https://user-images.githubusercontent.com/8114482/139270244-3c218efc-fde0-4fb4-b097-5eb14bce9c81.png)

Once if the other players join, one can able to view the cards and betting actions and from here on its just a poker which you know.

## Raising disputes
Due to any reason if the game didn't happen, one can reverse the payin_tx by raising the dispute by running the following commands:
```
cd bet/privatebet
./bet game solve # To resolve all the undisputed tx's
./bet game raise tx_id # To resolve a specific disputed tx's
./bet game info fail #To list the disputed games along with disputed tx id's
./bet game info success #To list the games that played successfully
./bet game info #To list all the successful and unsuccessful games
```

## Joining the private table
In order to join the private table player must set the value `is_table_private` to true by setting it to `[Y/y/1/t]` and then player has to set the `table_password` with the value that it gets from dealer.

So basically when dealer starts the private tables, by some means it shares the password with the players and the players has to set this password in the configuration file at `table_password` and the player will share this password with the dealer when it sends `stack_info_req` and if the dealer sees match in the password, then respond with `stack_info_resp` else `game_abort`.
