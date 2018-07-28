# Pangea/Privatebet - decentralized and generalized shuffled decks with faceup and facedown support and recovery from minority nodes disconnecting

When money depends on the accuracy of the numbers, it is important to make sure it is the right number. When it comes to cards, making sure that the deck was not tampered in anyway is most important. To achieve this, PANGEA uses a method similar to coin shuffle and all participants are involved in the process and if just a single player is honest then the deck is properly shuffled.

The problem with the other known methods for mental poker protocols are that they require a lot of bandwidth and are not as fast as PANGEA which uses a single curve25519 field multiply for the vast majority of its encrypt and decrypt functions. This creates an order of magnitude speedup and reduction of bandwidth required.

The final requirement is to be able to recover from a player disconnect. Since all players are involved in the shuffling and decrypting process, if any player disconnects, there is no way to decrypt any more cards. Unless there is a backup method. By making commitment to the deck's pubkeys before start, it can be verified that the recovered deck was indeed the proper deck. Using Shamir's Shared Secret to distribute the shards to all the players, as a group MofN of the players are able to reconstruct any (or all) player's cards. Play can continue, albeit at a slower pace.

Each hand (re)distributes the total funds the table has put into chips according to the result of each game. Under normal conditions, the majority of players sign an MofN multisig transaction to release the funds at the conclusion of a table. However, if more than N-M players refuse to sign, then the funds are not only not distributed correctly, they would be stuck. In the event where more than N-M players end up without any chips, they have no financial incentive to stay online to approve the MofN multisig transaction.

One way to mitigate this is to have an MofN transaction signed after each round, so at most the result of a single game is unaccounted for. However, we still have the case of there being nobody left to cosign with the last man standing from financial self-interest. Also, it is common for online game players to simply disconnect out of frustration (admit it, you have done this too), so relying on the losing players to approve a payout to the winner is not likely to be reliable. Even if the MofN values are reduced as the number of players at the table is reduced can this be avoided as we certainly do not want to get to a 1of2 multisig for obvious reasons.

The ideal solution is to have a blockchain enforced payout. This requires each change of gamestate to be recorded in the blockchain and the blockchain to be able to determine the proper allocation of funds. The method of doing a MofN after each game will reduce the blockchain bloat as only a game that doesnt have sufficient signers needs to be blockchain interpreted. The optimal is to use bi-directional multisig payment channels for normal play and have a backup blockchain mechanism that can be invoked by any single player in the event of the funds being stuck due to not enough signers.

A totally separate issue is privacy, which in the case of online games is needed as some govts have decided that to protect women and children they need to make online gaming illegal. This has nothing to do with hundreds of millions of dollars in campaign financing and other funds paid by Las Vegas casinos to politicians. I am assured that it is purely to protect the innocents from, well, not sure what the online players are being protected from. So, I think a way to play privately is quite important. To that effect, the psock capability allows a single node to publish an IP address and if that node is not playing, but just participating in the creation of the card deck, it will allow all the other players to play in realtime without posting their IP address. By using JUMBLR secret funds to purchase chips, the identity of the source of the funds are not linked. If you are in a totalitarian regime that is monitoring your IP traffic, then unfortunately you would need to take further protective actions, ie. dont play from any IP address that can be correlated to you.

As can be seen from the above, decentralized card games are one of the most difficult challenges in the crypto world and was unsolved, until PANGEA was released. The critical tech is divided between the card deck handling and the funds handling and each will be described independently.

## Card Deck Creation
In order to understand the cryptomath PANGEA does, it is required to understand a bit of the curve25519 internals.
``` C
bits256 curve25519(bits256 mysecret,bits256 basepoint)
{
    bits320 bp,x,z;
    mysecret.bytes[0] &= 0xf8, mysecret.bytes[31] &= 0x7f, mysecret.bytes[31] |= 0x40;
    bp = fexpand(basepoint);
    cmult(&x,&z,mysecret,bp);
    return(fcontract(fmul(x,crecip(z))));
}
```
The above is the fundamental curve25519 operation that takes a 256 bit scalar and a compressed field element. Not all 256 values are valid, so a few bits need to be hardcoded. Then the compressed field element is expanded, a complex multiplication is done to create X and Z values. Finally X/Z is calculated by doing a field multiplication between X and the reciprocal of Z and this field element is compressed.
``` C
bits256 xoverz_donna(bits256 a)
{
    limb x[10],zmone[10],z[10],bp[10],out[11]; bits256 result,basepoint;
    memset(basepoint.bytes,0,sizeof(basepoint)), basepoint.bytes[0] = 9;
    fexpand32(bp,basepoint.bytes);
    cmult32(x,z,a.bytes,bp);
    crecip32(zmone,z);
    fmul32(out,x,zmone);
    fcontract32(result.bytes,out);
    return(result);
}
```
The above is a 32bit CPU compatible equivalent function, where the basepoint is hardcoded to the generator { 9, 0, 0, 0, 0, 0, 0, 0 }

This creates the pubkey result out of the privkey a.

The key mathematical aspect that is utilized is that a curve25519 pubkey is based on a field division of the two parts of the complex number: X/Z

The assumption is that calculating the reverse is mathematically hard, ie. going from the value of X/Z to find the individual X and Z elements requires a lot of bruteforce (or a Quantum Computer from the future)

The above assumption is what the curve25519 encryption is based on, so it is a safe assumption that it is valid. PANGEA makes a further assumption that Z/X is equally mathematically hard. A simple proof that this is true is as follows:

If Z/X is not mathematically hard, we can calculate the reciprocal of X/Z to get Z/X and solve curve25519 pubkeys. Since the reciprocal that converts between X/Z and Z/X does not change the curve25519 from being mathematically hard, it follows that we could use Z/X form and have the same security level.

We also know that a curve25519 shared secret provides a secure way for two independent keypairs to communicate with each other.

```
curve25519(privA,pubB) == curve25519(privB,pubA)

X/Z.privA*pubB == X/Z.privB*pubA
```

We will designate specially selected keypairs as cards, we will require that the second byte of the privkey is the card index 0 to 51. To create a deck, we need 52 privkeys such that no two have the same second byte (offset 1).

Further, we will designate players by their pubkeys (which should be a session based keypair). This allows encoding each card for each player. Essentially each "card" is a vector of field elements with the special property that only the designated player can decode the privkey to determine what the second byte is.
``` C
bits256 cards777_initcrypt(bits256 data,bits256 privkey,bits256 pubkey,int32_t invert)
{
    bits256 hash;
    hash = curve25519_shared(privkey,pubkey);
    if ( invert != 0 )
        hash = crecip_donna(hash);
    return(fmul_donna(data,hash));
}
```
The fundamental card encryption is a field multiplication between data and a standard curve2559 shared secret.

The creator of the deck will thus generate a set of vectors such that all card index values 0 to 51 are present once and only once and all cards are field element vectors such that the corresponding player can decode the vector element and determine the value of the second byte (card index).

	cards777_initcrypt(privkey,privkey,playerpubs[j],0);

Player j can determine which card a specific card is by iterating through all the card pubkeys (known to all) and finding one it can properly decode:
``` C
    for (i=0; i<numcards; i++)
    {
        cardpriv = cards777_initcrypt(cipher,playerpriv,cardpubs[i],1);
        checkpub = curve25519(cardpriv,curve25519_basepoint9());
        if ( bits256_cmp(checkpub,cardpubs[i]) == 0 )
            return(cardpriv);
	}
```
Notice the initcrypt function is called with the inverse flag so the Z/X is used in the field multiplication. This cancels out the X/Z to end up with the card's private key. Of course, only the card pubkey for the right card would generate a matching pubkey from the reverse calculated cardpriv.

Now we have all the building blocks needed to create and validate the cards. One additional thing is needed for the deck and that is a commitment value that allows all to know that the correct deck was recreated. To achieve this a product of all the cards is calculated. An sha256 hash of the full deck could also be used, but the odds of a deck with all the correct properties (other than having whatever undetected violation) also having the same field product of all cards, is quite low.

Since the creator of the deck could record which card is in what slot, we need to add layered encryption and rounds of shuffling to prevent the deck creator from being able to decode any of the cards. In the source code, the all important privkey is not even stored, but we must assume ill intent is present to create a modified version and be immune from it.

Any player that gets a hold of the raw deck will be able to fully decode it as all cards are vectors having an element custom encrypted for that player. In order to prevent such a simple "attack", a blinding value is created from a high entropy field element that is multiplied into each card vector element. 

This raw deck will be what the MofN shamir's secret will encode for each of the players and since we have shown that all players are able to decode any card vector in its raw form, being able to reconstruct each card from the MofN shamir's secret shards is sufficient to allow MofN reconstruction of any card, assuming it is known how the originally created deck was shuffled.

It is possible that the deck creator falsifies the MofN backup deck, so it is necessary to obtain decks from a financially disinterested source who has more to lose from having created a faulty deck than to gain from whatever havoc a faulty deck would create. Since not being able to reconstruct a deck from the backup is a failure of a secondary mode, it is not something that can be relied upon by a deck creating attacker and the gains would only be by a participant that wanted to make null and void the result of a specific game. If the consequence of a faulty deck was a forfeit of all funds and the liability of this fell onto the deck creator, we have not only removed all financial incentive to create a faulty deck, but rather created a strong financial incentive to create only valid decks.

The other half of the required data to reconstruct a group shuffled deck is the permutation applied by each player. The requirement to obtain valid MofN data from each player is avoided if the deck creator also adds the blinding values to the MofN data.

The published deck is essentially the facedown cards that all players know the exact shuffling permutation each player applied. However, without knowning the actual card index encoded in the privkey, knowing the original location of a card in the blinded deck does not provide any benefit.

There is still the problem of possible collusion between the deck creator and any specific player. In order to remove this possibility, an independent vendor of blinding factors is utilized. These blinding values are put into the MofN shards and can be reconstructed after a hand to verify proper usage by all the players.

By knowing the entire set of blinding factors each player was assigned, it would be possible to bruteforce the possible encrypted values for the entire deck, so any single player disconnecting can thus be recovered from. If two players disconnect, it would still be possible, but at a drastically increased computational cost. With current day processors, it is estimated that two player disconnects can be reconstructed but a three player disconnect will be very time consuming to reconstruct. 

In order to minimize the workload at the blockchain level, all nodes will provide MofN shards for its blinding factors and applied permutations so that the deck used can be the decoded form for the blockchain to apply the game logic to. This divides the work required between the p2p table nodes and the blockchain so that the blockchain only deals with fully transparent decks and the p2p table nodes need to ensure that such data can be reconstructed, with the assumption of non-cheating deck creators and blinding value vendors.

The deck creating node will not have an easy time participating in any cheating as it wont know what blinding values are being used.

The blinding value vendor will not have an easy time participating in any cheating, as it has no control over the permutation applied by each node, not to mention that it wont know what card deck is being shuffled.

## Finally, we can put together the entire deck shuffling and card dealing protocol:

0. A fixed number of players at a table provide pubkeys for the deck.

1. Independent deck creating vendor creates a shuffled deck as specified, along with MofN shards for backup reconstruction.

2. Independent blinding value vendor generates high entropy blinding values for each player, along with MofN shards for backup reconstruction.

3. player 0 sends encrypted messages to each player for all the blinding values for player 0 (B0j).

4. Players apply their blinding values and permutations to the deck from 1. to arrive at the final shuffled deck that is published. The encrypted values that are sent to the subsequent node is stored.

To deal a card, it is taken in sequence from the final shuffled deck and sent to the designated player (or face up for table). In order to support a broadcast to all protocol for table communications, it is necessary to provide one final encryption layer prior to the last player receiving

1. For all but the last player: Each player searches the encrypted values it sent to find a matching value and multiplies by the inverse of the blinding value to send to the prior player.

The second to last player will receive an encrypted card that is of the form:

 B1 * B0 * card vector element

When the second player receives this value, the B0 * card vector element will be known and if a collusion between the deck creator and the first players is in effect, they would be able to decode the card.

To avoid this, the second player will create a tweetnacl encrypted message to the destination player the value of B0 * card vector element. Separately, the destination player will receive from the blinding values vendor an encypted message with all the possible B0j blinding values. Having both the B0 * card vector element and all the possible B0j values used, allows the destination node to properly decode the card, while no other player is able to.

Let us analyze the impact of all the players knowing the B0j values for them. Only the second player can benefit directly from the knowledge of the B0j values as all other players wouldnt be able to reverse the B1 blinding value. However, the B0j values that are disclosed are only for cards that are meant for each player, so the second player has the B0j values that were applied to all its vector elements, but not for any other player. Thus, while it could decode any B1 * B0 * card vector element meant for him, that is actually how it should be.

Face up cards are matter for the destination player to be all players, or for efficiency, the first player can simply disclose the decrypted card to the table. The constraints of the privkey second byte and its pubkey known is sufficient to be able to know it is valid when the deck is fully reconstructed. To prevent such cheating, either all faceup cards should be sent to all players and/or any payouts be after the deck is reconstructed and its checksum/hash value verified.

Now we able to create card decks that are shuffled by all players and are able to recover from player disconnects. Also, notice that the encoding method allows random "card" values of 0 to 255 and by extension of the privkey bytes, to an arbitrarily large number, with the cost being linear to the range of the "card" index values. 

Thus, a multi-deck blackjack chute can be group shuffled using the same method. 

Roullette is a straightforward 37 value deck with everything "faceup".

All dice based games are decks of power of 6 values, based on how many dice, ie. 6*6 = 36 for two dice, 6*6*6*6*6 = 7776 for five dice.

A 21 bell slot machine https://wizardofodds.com/games/slots/appendix/5/ would be 20*20*20 sized deck or 8000

Other usages, would be a "guess the number" type of game that is verifiable by all players as to who guessed the closest. By having multiple decks (one for each ball), a realtime lotto can be done. 

So while the originating context is texas holdem poker's requirements of a standard card deck with faceup, facedown and table cards, the PANGEA "deck" is actually a far more general method to have N players be able to verify a random number selected from a predetermined set of known size.

## There needs to be the following API(s) or equivalent:

### Deck Creation vendors:
createdeck(pubkeys[], range) range is the number of possible vals
returns blinded shuffled deck encrypted to player0, N MofN shards of non-blinded deck and blinding values, deck 256bit value

### Blinding value vendors:
createblinders(pubkeys[], range)
returns numplayer encrypted messages with the blinding values for that player, along with B0j and also MofN shards for all the blinding values

### Table API:
createtable(pubkeys[])
returns table 256bit value and psock endpoint

newdeck(pubkeys[], decks[])
player 0: random deck creator selected to createdeck()

player 1: random blinding value vendor selector to createblinds() 

player i (< n-1): upon receiving the blind values and blinded deck, player 0 shuffles and add blinds to deck and sends to player i+1

player n-1: broadcasts final shuffled deck

all players: broadcast secp signed table 256bit ready when final shuffled deck is received.

Timed state machine, each player has specified time for turn:
pass/check
bet/raise
fold

if no response after timeout, and majority post timedout, player folds

dealcard(tableid, cardi, destplayer) dest is player1 if faceup
player i (> 1): decodes and sends to player i-1
player 1: encrypts to destination player or broadcasts if faceup

all players: broadcast secp signed cardi received (with value if faceup)

bet(tableid, cardi, playeri, amount) secp signed bet

payouts(pubkeys[], amounts[])

### Roullette state machine:
Open for bets
deck is created and the first card is decrypted

if no disconnect: payouts are done
else: reconstruct enough to decrypt first card, payout via payment release or blockchain via reconstruction + gamerules + bets



 ## References
https://petsymposium.org/2014/papers/Ruffing.pdf

https://cr.yp.to/ecdh.html

http://www.cs.tau.ac.il/~bchor/Shamir.html

https://people.xiph.org/~greg/confidential_values.txt

http://www.tik.ee.ethz.ch/file/716b955c130e6c703fac336ea17b1670/duplex-micropayment-channels.pdf

https://people.csail.mit.edu/rivest/ShamirRivestAdleman-MentalPoker.pdf

https://crypto.stanford.edu/~pgolle/papers/poker.pdf


