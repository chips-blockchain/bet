# Common errors

First make sure the basics are working.

1. Check your Lightning node.

    Is Lightning running?

    `ps aux | grep lightning`


    Is Lightning synced?

    ```
    lightning-cli getinfo

    { "id" : "03f9b8668793f885e6c46bb5c37101d35d91f41791dec5157d9bde0db988b45271", "port" : 9735, "address" :
        [
            { "type" : "ipv4", "address" : "45.77.139.155", "port" : 9735 } ], "version" : "chipsln.0.0.0", "blockheight" : 7526989, "network" : "chips" }
    ```


2. Check CHIPS.

    Is chips running?

    `ps aux | grep chipsd`


    Is Lightning synced?
    
    ```
    chips-cli getinfo

    { "id" : "03f9b8668793f885e6c46bb5c37101d35d91f41791dec5157d9bde0db988b45271", "port" : 9735, "address" :
        [
            { "type" : "ipv4", "address" : "45.77.139.155", "port" : 9735 } ], "version" : "chipsln.0.0.0", "blockheight" : 7526989, "network" : "chips" }
    ``


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

## corrupted size vs. prev_size

**./bet player/dcv**

```
ln is in sync with chips
sqlite3_init_db_name: :36: :db_name: :/root/ .bet/db/pangea.db
corrupted size vs. prev_size
Aborted
```

--> Ehhm, you are not supposed to get that as it has been already fixed. If you receive this error please report it in the CHIPS Discord. Thank you

## Chips keep on failing

remove ~/.chips folder and perform the setup again

