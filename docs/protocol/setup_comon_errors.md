# Common issues

First make sure the basics are working.

1. Check your Lightning node.

    Is Lightning running?

    `ps aux | grep lightning`


    Is Lightning synced?

    if you lightning node is running in tmux do

    ```
    tmux a -t lightning
    ```

    and you can see if the chain is still syncing




    ```
    lightning-cli getinfo

    { "id" : "03f9b8668793f885e6c46bb5c37101d35d91f41791dec5157d9bde0db988b45271", "port" : 9735, "address" :
        [
            { "type" : "ipv4", "address" : "45.77.139.155", "port" : 9735 } ], "version" : "chipsln.0.0.0", "blockheight" : 7526989, "network" : "chips" }
    ```


2. Check CHIPS.

    Is chips running?

    `ps aux | grep chipsd`


    Is Chips synced?
    
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


## Errors when running Player node

### lightning-cli: Connecting to 'chipsln-rpc': Connection refused

**./bet player/dcv**

```
root@server:~/bet/privatebet$ ./bet player
lightning-cli: Connecting to 'chipsln-rpc': Connection refused
ln is 7822365 blocks behind chips network
lightning-cli: Connecting to 'chipsln-rpc': Connection refused
ln is 7822365 blocks behind chips network
lightning-cli: Connecting to 'chipsln-rpc': Connection refused
ln is 7822365 blocks behind chips network
lightning-cli: Connecting to 'chipsln-rpc': Connection refused
ln is 7822365 blocks behind chips network
lightning-cli: Connecting to 'chipsln-rpc': Connection refused
ln is 7822365 blocks behind chips network
lightning-cli: Connecting to 'chipsln-rpc': Connection refused
ln is 7822365 blocks behind chips network
```

-> Most probably your lightning crashed

### corrupted size vs. prev_size

**./bet player/dcv**

```
ln is in sync with chips
sqlite3_init_db_name: :36: :db_name: :/root/ .bet/db/pangea.db
corrupted size vs. prev_size
Aborted
```

--> Ehhm, you are not supposed to get that as it has been already fixed. If you receive this error please report it in the CHIPS Discord. Thank you

### bet_player_backend::1828::unable make lock_in transaction

Check if all the entities involved - player node and dealer node, have enough funds. Minimum is 0.01 CHIPS.

## Chips keep on failing

remove ~/.chips folder and perform the setup again

