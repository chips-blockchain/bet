Here we see detailed info about compiling, configuring, playing and raising disputes with the player node.

## Compiling player node
In order to setup the player node follow either the [basic compilation guide](./ubuntu_compile.md) or [detailed compilation guide](./compile.md). 

## Configuring the player node

All the configurable options for the player can be set in `player_setup.ini` file, which is located at `bet/privatebet/config` path. The default content of this file is as follows:
```
[player]
max_allowed_dcv_commission = 5 # This is the max percentage of the dealer commision that the player is willing to pay, if the dealer sets the commission higher than this then the backend node will exit.
type = torv3 # This will enforce LN and CHIPS run on onion address, atm this will be ignored.
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
./bet game solve
```
