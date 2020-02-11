For Deposit

Backend sends wallerInfo to the GUI, to this address the player deposits the funds, the JSON message looks as follows:
```
{
    method: "walletInfo", 
    addr: "bQepVNtzfjMaBJdaaCq68trQDAPDgKnwrD", 
    balance: 0.13269034
}
```
For withdraw

GUI sends withdrawRequest to the backend
```
{
   method: withdrawRequest
}
```
Backend sends withdrawResponse to the GUI
```
{
    method: withdrawResponse,
    balace: xxxx,
    addr: [addr1,addr2,...,addrn]
}
```
GUI sends withdraw to the backend
```
{
    method: withdraw,
    amount: yyyy,
    addr: some_addr_from_the_above_list
}
```
Backend sends withdrawInfo to the GUI
 ```
 {
   method: withdrawInfo,
   txid: tx_id,
   balance: zzzz
}
```
