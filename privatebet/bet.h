/******************************************************************************
 * Copyright © 2014-2018 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/


/*
privatebet - decentralized and generalized shuffled decks with faceup and facedown support and recovery from minority nodes disconnecting

When money depends on the accuracy of the numbers, it is important to make sure it is the right number. When it comes to cards, making sure that the deck was not tampered in anyway is most important. To achieve this, PANGEA uses a method similar to coin shuffle and all participants are involved in the process and if just a single player is honest then the deck is properly shuffled.

The problem with the other known methods for mental poker protocols are that they require a lot of bandwidth and are not as fast as PANGEA which uses a single curve25519 field multiply for the vast majority of its encrypt and decrypt functions. This creates an order of magnitude speedup and reduction of bandwidth required.

The final requirement is to be able to recover from a player disconnect. Since all players are involved in the shuffling and decrypting process, if any player disconnects, there is no way to decrypt any more cards. Unless there is a backup method. By making commitment to the deck's pubkeys before start, it can be verified that the recovered deck was indeed the proper deck. Using Shamir's Shared Secret to distribute the shards to all the players, as a group MofN of the players are able to reconstruct any (or all) player's cards. Play can continue, albeit at a slower pace.

Each hand (re)distributes the total funds the table has put into chips according to the result of each game. Under normal conditions, the majority of players sign an MofN multisig transaction to release the funds at the conclusion of a table. However, if more than N-M players refuse to sign, then the funds are not only not distributed correctly, they would be stuck. In the event where more than N-M players end up without any chips, they have no financial incentive to stay online to approve the MofN multisig transaction.

One way to mitigate this is to have an MofN transaction signed after each round, so at most the result of a single game is unaccounted for. However, we still have the case of there being nobody left to cosign with the last man standing from financial self-interest. Also, it is common for online game players to simply disconnect out of frustration (admit it, you have done this too), so relying on the losing players to approve a payout to the winner is not likely to be reliable. Even if the MofN values are reduced as the number of players at the table is reduced can this be avoided as we certainly do not want to get to a 1of2 multisig for obvious reasons.

The ideal solution is to have a blockchain enforced payout. This requires each change of gamestate to be recorded in the blockchain and the blockchain to be able to determine the proper allocation of funds. The method of doing a MofN after each game will reduce the blockchain bloat as only a game that doesnt have sufficient signers needs to be blockchain interpreted. The optimal is to use bi-directional multisig payment channels for normal play and have a backup blockchain mechanism that can be invoked by any single player in the event of the funds being stuck due to not enough signers.

A totally separate issue is privacy, which in the case of online games is needed as some govts have decided that to protect women and children they need to make online gaming illegal. This has nothing to do with hundreds of millions of dollars in campaign financing and other funds paid by Las Vegas casinos to politicians. I am assured that it is purely to protect the innocents from, well, not sure what the online players are being protected from. So, I think a way to play privately is quite important. To that effect, the psock capability allows a single node to publish an IP address and if that node is not playing, but just participating in the creation of the card deck, it will allow all the other players to play in realtime without posting their IP address. By using JUMBLR secret funds to purchase chips, the identity of the source of the funds are not linked. If you are in a totalitarian regime that is monitoring your IP traffic, then unfortunately you would need to take further protective actions, ie. dont play from any IP address that can be correlated to you.

As can be seen from the above, decentralized card games are one of the most difficult challenges in the crypto world and was unsolved, until PANGEA was released. The critical tech is divided between the card deck handling and the funds handling and each will be described independently.

Card Deck Creation
In order to understand the cryptomath PANGEA does, it is required to understand a bit of the curve25519 internals.

bits256 curve25519(bits256 mysecret,bits256 basepoint)
{
    bits320 bp,x,z;
    mysecret.bytes[0] &= 0xf8, mysecret.bytes[31] &= 0x7f, mysecret.bytes[31] |= 0x40;
    bp = fexpand(basepoint);
    cmult(&x,&z,mysecret,bp);
    return(fcontract(fmul(x,crecip(z))));
}

The above is the fundamental curve25519 operation that takes a 256 bit scalar and a compressed field element. Not all 256 values are valid, so a few bits need to be hardcoded. Then the compressed field element is expanded, a complex multiplication is done to create X and Z values. Finally X/Z is calculated by doing a field multiplication between X and the reciprocal of Z and this field element is compressed.

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

The above is a 32bit CPU compatible equivalent function, where the basepoint is hardcoded to the generator { 9, 0, 0, 0, 0, 0, 0, 0 }

This creates the pubkey result out of the privkey a.

The key mathematical aspect that is utilized is that a curve25519 pubkey is based on a field division of the two parts of the complex number: X/Z

The assumption is that calculating the reverse is mathematically hard, ie. going from the value of X/Z to find the individual X and Z elements requires a lot of bruteforce (or a Quantum Computer from the future)

The above assumption is what the curve25519 encryption is based on, so it is a safe assumption that it is valid. PANGEA makes a further assumption that Z/X is equally mathematically hard. A simple proof that this is true is as follows:

If Z/X is not mathematically hard, we can calculate the reciprocal of X/Z to get Z/X and solve curve25519 pubkeys. Since the reciprocal that converts between X/Z and Z/X does not change the curve25519 from being mathematically hard, it follows that we could use Z/X form and have the same security level.

We also know that a curve25519 shared secret provides a secure way for two independent keypairs to communicate with each other.

curve25519(privA,pubB) == curve25519(privB,pubA)

X/Z.privA*pubB == X/Z.privB*pubA

We will designate specially selected keypairs as cards, we will require that the second byte of the privkey is the card index 0 to 51. To create a deck, we need 52 privkeys such that no two have the same second byte (offset 1).

Further, we will designate players by their pubkeys (which should be a session based keypair). This allows encoding each card for each player. Essentially each "card" is a vector of field elements with the special property that only the designated player can decode the privkey to determine what the second byte is.

bits256 cards777_initcrypt(bits256 data,bits256 privkey,bits256 pubkey,int32_t invert)
{
    bits256 hash;
    hash = curve25519_shared(privkey,pubkey);
    if ( invert != 0 )
        hash = crecip_donna(hash);
    return(fmul_donna(data,hash));
}

The fundamental card encryption is a field multiplication between data and a standard curve2559 shared secret.

The creator of the deck will thus generate a set of vectors such that all card index values 0 to 51 are present once and only once and all cards are field element vectors such that the corresponding player can decode the vector element and determine the value of the second byte (card index).

cards777_initcrypt(privkey,privkey,playerpubs[j],0);

Player j can determine which card a specific card is by iterating through all the card pubkeys (known to all) and finding one it can properly decode:

for (i=0; i<numcards; i++)
{
    cardpriv = cards777_initcrypt(cipher,playerpriv,cardpubs[i],1);
    checkpub = curve25519(cardpriv,curve25519_basepoint9());
    if ( bits256_cmp(checkpub,cardpubs[i]) == 0 )
        return(cardpriv);
}

Notice the initcrypt function is called with the inverse flag so the Z/X is used in the field multiplication. This cancels out the X/Z to end up with the card's private key. Of course, only the card pubkey for the right card would generate a matching pubkey from the reverse calculated cardpriv.

 Each player broadcasts a hash of hash of the player's data.
 
 deterministic sort of hash of player's data determines order.
 
 player's data has the permutation vector.
 
 players broadcast their data. if any player doesnt submit permutation or it doesnt match hash of hash, they are folded and permutations skipped.
 
 all nodes arrive at same combined permutation matrix, which allows to know the index of each card in the deck. For all face up applications, it is complete.
 
 For facedown cards, the Cards[cardindex][j] MofN data is sent to the player j

Finally, we can put together the entire deck shuffling and card dealing protocol:

0. A fixed number of players at a table provide pubkeys for the deck.

1. Independent deck creating vendor creates a shuffled deck as specified, along with MofN shards.

2. player 0 sends encrypted messages to each player for all MofN Cards.

3. permutation vector commitment, deterministic prioritization, consensus permutation vector
 
To deal a card, the top of deck is the first unused entry for the consensus permutation vector. For facedown cards, that is used as the cardindex and the MofN data is sent to the destination player j

Now we able to create card decks that are shuffled by all players and are able to recover from player disconnects. Also, notice that the encoding method allows random "card" values of 0 to 255 and by extension of the privkey bytes, to an arbitrarily large number, with the cost being linear to the range of the "card" index values.

Thus, a multi-deck blackjack chute can be group shuffled using the same method.

Roullette is a straightforward 37 value deck with everything "faceup".

All dice based games are decks of power of 6 values, based on how many dice, ie. 6*6 = 36 for two dice, 6*6*6*6*6 = 7776 for five dice.

A 21 bell slot machine https://wizardofodds.com/games/slots/appendix/5/ would be 20*20*20 sized deck or 8000

Other usages, would be a "guess the number" type of game that is verifiable by all players as to who guessed the closest. By having multiple decks (one for each ball), a realtime lotto can be done.

So while the originating context is texas holdem poker's requirements of a standard card deck with faceup, facedown and table cards, the PANGEA "deck" is actually a far more general method to have N players be able to verify a random number selected from a predetermined set of known size.

There needs to be the following API(s) or equivalent:

Deck Creation vendors:
createdeck(table_endpoint, pubkeys[], decks[]) each deck[i] is numcards, [52]
returns blinded shuffled deck encrypted to player0, N MofN shards of non-blinded deck and blinding values, deck 256bit value

Blinding value vendors:
createblinds(table_endpoint, pubkeys[], numcards)
returns numplayer encrypted messages with the blinding values for that player, along with B0j and also MofN shards for all the blinding values

Table API:
createtable(pubkeys[])
returns table 256bit value for the psock and endpoint

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

Roullette state machine:
Open for bets
deck is created and the first card is decrypted

if no disconnect: payouts are done
else: reconstruct enough to decrypt first card, payout via payment release or blockchain via reconstruction + gamerules + bets



References
https://petsymposium.org/2014/papers/Ruffing.pdf

https://cr.yp.to/ecdh.html

http://www.cs.tau.ac.il/~bchor/Shamir.html

https://people.xiph.org/~greg/confidential_values.txt

http://www.tik.ee.ethz.ch/file/716b955c130e6c703fac336ea17b1670/duplex-micropayment-channels.pdf

https://people.csail.mit.edu/rivest/ShamirRivestAdleman-MentalPoker.pdf

https://crypto.stanford.edu/~pgolle/papers/poker.pdf



*/


#include <stdio.h>
#include <stdint.h>

#include "../../SuperNET/crypto777/OS_portable.h"
#include "../../SuperNET/iguana/exchanges/LP_include.h"
//#include "../../SuperNET/iguana/exchanges/LP_nativeDEX.c"


#if defined(WIN32) || defined(USE_STATIC_NANOMSG)
#include "../../SuperNET/crypto777/nanosrc/nn.h"
#include "../../SuperNET/crypto777/nanosrc/bus.h"
#include "../../SuperNET/crypto777/nanosrc/pubsub.h"
#include "../../SuperNET/crypto777/nanosrc/pipeline.h"
#include "../../SuperNET/crypto777/nanosrc/reqrep.h"
#include "../../SuperNET/crypto777/nanosrc/tcp.h"
#include "../../SuperNET/crypto777/nanosrc/pair.h"
#else
#include "/usr/local/include/nanomsg/nn.h"
#include "/usr/local/include/nanomsg/bus.h"
#include "/usr/local/include/nanomsg/pubsub.h"
#include "/usr/local/include/nanomsg/pipeline.h"
#include "/usr/local/include/nanomsg/reqrep.h"
#include "/usr/local/include/nanomsg/tcp.h"
#include "/usr/local/include/nanomsg/pair.h"
#endif

#define CARDS777_MAXCARDS 255 //52    //
#define CARDS777_MAXPLAYERS 10 //9   //
#define CARDS777_MAXROUNDS 3 //9   //
#define CARDS777_MAXCHIPS 1000
#define CARDS777_CHIPSIZE (SATOSHIDEN / CARDS777_MAXCHIPS)
#define BET_PLAYERTIMEOUT 15
#define BET_GAMESTART_DELAY 10
#define BET_RESERVERATE 1.025
#define LN_FUNDINGERROR "\"Cannot afford funding transaction\""

struct BET_shardsinfo
{
    UT_hash_handle hh;
    int32_t numcards,numplayers;
    uint8_t key[sizeof(bits256) + 2*sizeof(int32_t)];
    bits256 recover,data[];
};

struct gfshare_ctx
{
    uint32_t sharecount,threshold,size,buffersize,allocsize;
    uint8_t sharenrs[255],buffer[];
};

struct privatebet_info
{
    char game[64];
    bits256 MofN[CARDS777_MAXCARDS * CARDS777_MAXPLAYERS],cardpubs[CARDS777_MAXCARDS],playerpubs[CARDS777_MAXPLAYERS+1],tableid,deckid;
    int32_t numplayers,maxplayers,numrounds,range,myplayerid,maxchips,chipsize;
    int32_t pullsock,pubsock,subsock,pushsock;
    uint32_t timestamp;
    char peerids[CARDS777_MAXPLAYERS+1][67];
};

struct privatebet_rawpeerln
{
    uint64_t msatoshi_to_us,msatoshi_total;
    uint32_t unique_id;
    char peerid[67],channel[64],netaddr[64],state[32];
};

struct privatebet_peerln
{
    int32_t numrhashes,numpaid;
    bits256 hostrhash,clientrhash,clientpubkey;
    struct privatebet_rawpeerln raw;
};

struct privatebet_vars
{
    bits256 myhash,hashes[CARDS777_MAXPLAYERS+1][2];
    int32_t permis[CARDS777_MAXPLAYERS+1][CARDS777_MAXCARDS];
    uint32_t endround[CARDS777_MAXPLAYERS+1],evalcrcs[CARDS777_MAXPLAYERS+1],consensus;
    cJSON *actions[CARDS777_MAXROUNDS][CARDS777_MAXPLAYERS+1];
    int32_t mypermi[CARDS777_MAXCARDS],permi[CARDS777_MAXCARDS],turni,round,validperms,roundready,lastround,numconsensus;
};

struct pair256 { bits256 priv,prod; };

struct privatebet_share
{
	int32_t numplayers,range,myplayerid;
	int32_t pullsock,pubsock,subsock,pushsock;
	bits256 bvv_public_key;
	struct pair256 player_key;
};

//added by sg777

struct deck_player_info
{
	struct pair256 player_key;
	bits256 cardpubkeys[CARDS777_MAXCARDS],cardprivkeys[CARDS777_MAXCARDS];
	int32_t permis[CARDS777_MAXCARDS];
	bits256 cardprods[CARDS777_MAXPLAYERS][CARDS777_MAXPLAYERS];
	bits256 bvvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 dcvpubkey,bvvpubkey,deckid;
	uint32_t numplayers,maxplayers,numcards;
};

struct deck_dcv_info
{
	bits256 deckid;
	struct pair256 dcv_key;
	int32_t permis[CARDS777_MAXCARDS];
	bits256 cardpubkeys[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 dcvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 cardprods[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	bits256 peerpubkeys[CARDS777_MAXPLAYERS];
	uint32_t numplayers,maxplayers;
};

struct deck_bvv_info
{
	bits256 deckid;
	int32_t permis[CARDS777_MAXCARDS];
	struct pair256 bvv_key;
	bits256 bvvblindcards[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
	uint32_t numplayers,maxplayers;
};

bits256 *BET_process_packet(bits256 *cardpubs,bits256 *deckidp,bits256 senderpub,bits256 mypriv,uint8_t *decoded,int32_t maxsize,bits256 mypub,uint8_t *sendbuf,int32_t size,int32_t checkplayers,int32_t range);
cJSON *BET_hostrhashes(struct privatebet_info *bet);
bits256 BET_clientrhash();

int32_t BET_clientupdate(cJSON *argjson,uint8_t *ptr,int32_t recvlen,struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_hostcommand(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars);
//void BET_cmdloop(bits256 privkey,char *smartaddr,uint8_t *pubkey33,bits256 pubkey,struct privatebet_info *bet);
cJSON *BET_statemachine_gamestart_actions(struct privatebet_info *bet,struct privatebet_vars *vars);
cJSON *BET_statemachine_turni_actions(struct privatebet_info *bet,struct privatebet_vars *vars);
//void BET_statemachine_deali(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t deali,int32_t playerj);

void BET_statemachine_joined_table(struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_statemachine_unjoined_table(struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_statemachine_roundstart(struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_statemachine_roundend(struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_statemachine_gameend(struct privatebet_info *bet,struct privatebet_vars *vars);
void BET_statemachine(struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_client_turni(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
int32_t BET_statemachine_validate(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t round,int32_t senderid);
int32_t BET_client_gameeval(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);
void BET_statemachine_consensus(struct privatebet_info *bet,struct privatebet_vars *vars);
int32_t BET_statemachine_outofgame(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t round,int32_t senderid);
int32_t BET_statemachine_turnivalidate(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t round,int32_t senderid);

int cli_main(char *buffer,int32_t maxsize,int argc, char *argv[]);
struct privatebet_peerln *BET_peerln_find(char *peerid);

void stats_rpcloop(void *args);
bits256 xoverz_donna(bits256 a);
bits256 crecip_donna(bits256 a);
bits256 fmul_donna(bits256 a,bits256 b);
bits256 card_rand256(int32_t privkeyflag,int8_t index);
struct pair256 deckgen_common(struct pair256 *randcards,int32_t numcards);
struct pair256 deckgen_common1(struct pair256 *randcards,int32_t numcards);
struct pair256 deckgen_player(bits256 *playerprivs,bits256 *playercards,int32_t *permis,int32_t numcards);
int32_t sg777_deckgen_vendor(int32_t playerid, bits256 *cardprods,bits256 *finalcards,int32_t numcards,bits256 *playercards,bits256 deckid); 
struct pair256 sg777_blinding_vendor(struct pair256 *keys,struct pair256 b_key,bits256 *blindings,bits256 *blindedcards,bits256 *finalcards,int32_t numcards,int32_t numplayers,int32_t playerid,bits256 deckid);
bits256 t_sg777_player_decode(struct privatebet_info *bet,int32_t cardID,int numplayers,struct pair256 key,bits256 public_key_b,bits256 blindedcard,bits256 *cardprods,bits256 *playerprivs,int32_t numcards);

struct pair256 p2p_bvv_init(bits256 *keys,struct pair256 b_key,bits256 *blindings,bits256 *blindedcards,bits256 *finalcards,int32_t numcards,int32_t numplayers,int32_t playerid,bits256 deckid);

bits256 curve25519_fieldelement(bits256 hash);


