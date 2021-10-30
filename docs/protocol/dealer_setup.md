Here we see detailed info about compiling, configuring, running backend, resolving disputes, hosting GUI and most importanly the constraints to meet and hosting private tables.

## Compiling the dealer node
The compilation instructions for any node of a bet is same, please follow the compilation guide for [ubuntu](./ubuntu_compile.md) and more detailed guide with [docker](./compile.md) 
>Note : The docker setup is tested only on ubuntu.

## Configuring the dealer node
All the configurable options for the player can be set in `dealer_setup.ini` file, which is located at `bet/privatebet/config` path. The default content of this file is as follows:
```
[dealer]
max_players =	3 #This is the maximun number of players that dealer will allow them to join the table. Atm, dealer waits for this number of players to join before starting hand.
table_stack_in_chips=	0.01000000 #This is the amount of CHIPS that needed to join the table.
dcv_commission= 0.5 #This is the percentage of the pot amount that dealer takes home as commission.
chips_tx_fee=	0.00050000 #The default tx_fee is `0.0001` dealer can set any amount greater than minimum tx_fee.
type= torv3 #When this set dealer only allows LN nodes running on torv3 to establish channels with it. This feature is not activated yet.
gui_host = ""#If the GUI is hosted on the same backend node then it can be left blank, else the URL on which the GUI is hosted must be provided.
```

## Running the dealer node
As a dealer along with running the `bet` backend node, dealer should also be responsible in installing and running the `pangea-poker` GUI backend.

### Running the dealer bet backend
```
cd bet/privatebet
./bet dcv <ipv4_addr> #Here ipv4_addr is the public ipv4 address on which the dealer backend is running. 
```
For example im running all my backend setup is on the node `159.69.23.30`, so i start my dealer backend is as follows:
> ./bet dcv 159.69.23.30

### Running the dealer pangea-poker GUI backend
The instructions to build & run `pangea-poker` GUI is [here](https://github.com/chips-blockchain/pangea-poker/tree/master#development). I'm copying them them the same below:

```
git clone https://github.com/chips-blockchain/pangea-poker.git
npm install
npm start
```
If you find any issues related to npm, please install latest npm package and try
```
sudo npm cache clean -f
sudo npm install -g n
sudo n stable
sudo npm install npm@latest
```
Since i installed it on the node `159.69.23.30`, I'm able to access it using `http://159.69.23.30:1234/` and the same will be shared with the players in the console log when the dealer accepts the player join.

## Dealer during the gameplay
Once the dealer node is started, no intervention of dealer is needed during the gameplay. 

If dealer wants to observe whats happening on the dealer bet backend from the GUI the dealer can connect to the backend using following steps, but this is **optional**.

Access the GUI backend weblink from the browser, which looks as follows:

![image](https://user-images.githubusercontent.com/8114482/139513656-357cfa09-54e2-4976-a1c5-94d84b54fb7b.png)

Then click on Private Table --> Dealer --> Enter the dealer IP -->Click on Set Nodes as shown below:

![image](https://user-images.githubusercontent.com/8114482/139513716-1c564a48-c44f-4703-b75a-6bc826581d9d.png)

After connecting to dealer backend,  from GUI dealer view the same interface as players except the cards.

## Raising disputes
Due to any reason if the game didn't happen, the dealer can help in players in raising the disputes on behalf of the player. Even if dealer raises the disputes, no need to worry about losing the funds since the funds in the payin_tx are credited to the address which is present in the data part of the payin_tx and which is belongs to the player.

```
cd bet/privatebet
./bet game solve
./bet game dispute "disputed tx_id" #This is for a specific tx_id.
```

## Running private tables
At the moment there are no constraints to be a dealer. Anyone can follow the above instructions and set a dealer node to play with closed friends groups. Going forward to incentivise the people holding CHIPS, to become a dealer one should possess `100xBB` or `50K` CHIPS whichever is lower. Even after enforcing this still one can host their own dealer but we limit the table stack size to < 0.01 CHIPS, meaning that one can still host the dealer but can only play with small amounts.
