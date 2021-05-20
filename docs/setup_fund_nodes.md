# Fund nodes

In order to start playing you need to fund your LN node and your CHIPS node.

## Fund lightning

1. Get a new lightning address

    ```
    root@server:~/bet/privatebet$ lightning-cli newaddr

    {
        "address": "bGq2Nnze7rxYuc8pExa6z7TiUoGGUac2hG",
        "p2sh-segwit": "bGq2Nnze7rxYuc8pExa6z7TiUoGGUac2hG"
    }
    ```

2. Send 0.25 CHIPS to the newly generated address. It is `bGq2Nnze7rxYuc8pExa6z7TiUoGGUac2hG` in our example.

3. Check the balance

    ```
    root@server:~/bet/privatebet$ lightning-cli listfunds
    {
    "outputs": [
        {
            "txid": "70476e471465024f2f3fd5ff304300609969cdf1be17b23dc5e01b0962e28e41",
            "output": 0,
            "value": 100000000,
            "amount_msat": "100000000000msat",
            "redeemscript": "0014ae825f48e7fb9717dd824bfd0bb177327fb47186",
            "scriptpubkey": "a9142cf70c047563ea49b40d4613470140da4eea787087",
            "address": "bGq2Nnze7rxYuc8pExa6z7TiUoGGUac2hG",
            "status": "confirmed",
            "blockheight": 7822164,
            "reserved": false
        }
    ],
    "channels": []
    }
    ```

## Fund CHIPS

1. Get new address

    ```
    root@server:~/bet/privatebet$ chips-cli getnewaddress
    bbd7L2LuapPhw18DQZUnw25v1N3F3nrHB2
    ```
2. Send a few CHIPS there (1-3?). 

3. Check the balance
    
    ```
    root@server:~/bet/privatebet$ chips-cli getbalance
    5.00000000
    ```




