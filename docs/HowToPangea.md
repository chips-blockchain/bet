# Pangea setup

## How the game works

To play the game serveral entities have to run - cashier nodes, player node (you), dealer node (you or someone else). Check in our Discord in chips-chat to see if we are running a dealer and cashier nodes for you to connect to.

## Dealer or a Player or both?

You need to decide if you want to be a player or you want to host a game (be a dealer). You can also do the setup for both if you want.

## Server Requirements for a Pangea node

At least 4GB RAM
abt 20 GB of free space should be plenty

The following setup has been tested on 
Ubuntu 18.04 x 64
RAM: 4GB
SSD: 80GB

## The setup

1. [Server setup](./DigitalOceanServerSetup.md)

2. Setup backend

3. Fund your nodes

4. Setup GUI

5. Play

### 2. Setup backend

Now you have 3 options: Use installation script OR use Docker OR compile and install manually


#### Option 1. Use Docker (NOT READY FOR TESTING YET), see [Docker repo](https://github.com/chips-blockchain/docker) for info

#### Option 2. Manually compile and install everything, see [Compile.md](./compile.md)

#### Option 3. Use the script to perform the setup.

   3.1. Run the installation script under the user created at the intial server setup

   ```
   curl -L https://github.com/chips-blockchain/bet/blob/master/scripts/install.sh | bash -
   ```

   3.2. Run CHIPS daemon
   ```shell
   chipsd &
   ```

   Check if CHIPS is running

   ```shell
   chips-cli getinfo
  ```

   The output might be an error followed by `Loading blocks...`. CHIPS is loading our bootstrapped blocks. Just give it some time and try again later. Successful result is an object with info.

   **Wait for CHIPS to fully sync.**

   Can take about 1-2 hours.

   CHIPS is fully synced when the number of `blocks` and `headers` match.

   ```bash
   chips-cli getinfo
   {
     ...
     ...
     "chain": "main",
     "blocks": 7709392,
     "headers": 7709392,
     ...
     ...
   } 
   ```

  3.3. Run the lightning node in tmux

   Create a tmux session
   ```
   tmux new -s lightning
   ```

   Then inside the tmux session you've just created
   ```
   lightningd --log-level=debug &
   # CTRL + B, then D to detach from the tmux session, to attach to the session again `tmux a -t lightning`
   ```

   Get chain info - If it returns your node’s id, you’re all set.
   ```
   lightning-cli getinfo
   ```

   If you don't see output `lightning-cli` of this command formatted, you can add `jq` with pipe command like following:
   ```
   lightning-cli getinfo | jq .
   ```

### 3. Fund your nodes

Once you setup your nodes, you need to fund them. See [Fund your nodes instructions](./setup_fund_nodes.md).

### 4. Setup GUI

GUI is a separate project under the CHIPS blockchain repository. It does not require a backend setup aka player node for it. GUI only needs its repo cloned, dependencies installed and project run. You can have the GUI running locally on your computer. It will be the easiest for you to have it locally.

There is also a deployed version of GUI at http://dev.chips.cash.

**Local GUI setup**

```
git clone https://github.com/chips-blockchain/pangea-poker.git

npm install && npm start

```

### 5. How to Play

  See [Tutorial](https://github.com/chips-blockchain/pangea-poker/blob/dev/tutorial/Tutorial.md)


## Its not working! Omg!

  Check out our ever growing list of [Common errors](./setup_common_errors.md). If your issue is not on the list, please report it to Discord and we will attend to it as soon as we can.
