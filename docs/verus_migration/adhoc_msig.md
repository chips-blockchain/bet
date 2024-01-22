# Adhoc MultiSignatures

A concept where in which the multiple authorities agreed upon change over time to perform a precommited or future operations. I expereinced this concept first time when I'm using vdxf ID's in Verus. 
In Verus we register an ID, which is a human readable name and the updates to it are controlled by primaryaddresses and minimumsignatures needed. So basically if the minimumsignatures needed are greater than 1, then its nothing but a multiple signature concept.

## How it's relevant in bet

In bet eco system we use multisignatures a lot to handle the player funds. In a way even in bet we compute multi signatures for each game based on the availability of the cashier nodes. 
Before the player can take part in any game, it checks for the available cashier nodes and since the cashier pubkey addresses are global and visible to all, the player simply takes the pubkeys of those available cashiers and computes the `m-of-n` multisigaddress, where m is `n/2+1`. So here in bet this multisignature address varies whenever new cashiers come to live or the existing cashiers drops. So for that reasons there isn't any unique multisignature address that we can use all the time. But with vdxf ID's, by keeping the ID name same, the primaryaddress that can update the ID and the number of signatures of that primaryaddrsses needed to update can be modified and I feel in a way that gives flexibility to remove the bad actors or add new actors to the ecosystem without impacting the end users.

