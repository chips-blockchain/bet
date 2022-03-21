There are various types of transactions happen during the game and based on the context we classify them into three types
* Payin_tx
* Dust tx's to record game moves
* Payout_tx

#### Payin_tx
At the time of joining the table player locks funds at msig address owned by cashier nodes and this locking tx is called as payin_tx. The data part of this tx contains the enough info needed by the cashier node to resolve any disputes raised by the player.

Any payin_tx's which are unspent are the indication that game didn't progressed successfully. The payin_tx's which are specific to the player node can be retrieved from the player local DB as follows:
```
./bet game info fail
```
The data part of one such payin_tx looks as follows:
```
$ ./bet extract_tx_data 018f7aa0afd6321768aaac42c5d4dcee12dbf864d1bd534a36e382cbef9075ab
[bet.c:main:482] Data part of tx 
 {
	"table_id":	"0e9d7446df17acdecd9578faf76a746352bdf17cdaa7b3fb37ac6578d5cec0df",
	"msig_addr_nodes":	"[\"141.94.227.65\", \"141.94.227.66\", \"141.94.227.67\", \"141.94.227.68\"]",
	"min_cashiers":	2,
	"player_id":	"ddbade69f3158b761c6bfc97639df9a0e038ccc4ec11a83837beae8a0f1c6928",
	"dispute_addr":	"bM5tVV5c4RTudouRpNDQNkBtVtruuCEBB8",
	"msig_addr":	"bJpRNkKYXweiLDzR1kNYpgZRwYaakMdeSb"
}
```
Lets take a look at what each field in the JSON object refering:
* table_id: The table id player joined during the game.
* msig_addr_nodes: The cashier node IP's which took part in this game.
* min_cashiers: The minimum number of cashier nodes needed to reverse this tx incase of disputes.
* player_id: ID of the player during the game.
* dispute_addr: The address to which these tx funds to be deposited incase of disputes.
* msig_addr: The msig address to which this tx is made.

### Dust tx's to record game moves
The data fields of a sample dust tx that happen during the geme looks as follows
```
~/bet/privatebet$ ./bet extract_tx_data 58ec6cbcef7a223405d77b78e6fbf0425d2c3333105a586f99160f479e4ca26b
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	3,
	"playerID":	1,
	"betAmount":	2,
	"action":	4
}
```
Lets take a look into what these fields signifies
* method - It represents the type of data, genrally to record betting moves during the game the method name should be `bet`.
* table_id - The table_id for which this betting move is linked to.
* round - In poker game this tells about at what round the bet of `betAmount` is happened.
* betAmount - Amount that put in betting.
* action - The corresponding action under which the betting was made like SB, BB, raise, call, check, fold or allin.

### Payout_tx
After each hand, the game will be evaluated and the funds will be settled based across the player after evaluation of each player cards. The tx which is made to settle the funds at the end of each hand is called as payout_tx.
The payout_tx's are the indication of successful games that are played and you can see these tx's attached to dealer_address, dev_fund_address, cashier_msig_address and the corresponding players addresses during that hand.

The payout_tx's which are specific to the player are retrieved as follows:
```
./bet game info success
```
The data part of one such payout_tx looks as follows:
```
$ ./bet extract_tx_data c04da4d720cbaa940f5983134816892176ad0e153443db8e39cbe98082a1311c
[bet.c:main:482] Data part of tx 
 {
	"table_id":	"925beb9843e39782fcedb5988406dc6bbaa39c9fc517765a28cb509d8816eb39",
	"maxplayers":	3,
	"rounds":	3,
	"game_state":	[{
			"bet_actions":	[5, 3, 3, 3],
			"player_cards":	[11, 5, 6, 40, 25, 39, 50]
		}, {
			"bet_actions":	[5, 3, 3, 3],
			"player_cards":	[43, 19, 6, 40, 25, 39, 50]
		}, {
			"bet_actions":	[3, 3, 3, 3],
			"player_cards":	[22, 21, 6, 40, 25, 39, 50]
		}],
	"player_ids":	["80e5b819766e1fd0ed397cd8fa8b2b5a0ad1c77c7935c7415fe0137e6510e2e8", "a0e3b71cf63f2a06570d163ce5c5bdb4b39aeac309cb85af7a45b68609fafef1", "15a2c822cec6f0beb3d05d0e8e370df87c39ff71b410f7482cd3b59c39ace942"],
	"threshold_value":	2,
	"msig_addr_nodes":	["141.94.227.65", "141.94.227.66", "141.94.227.67", "141.94.227.68"]
}
```
Lets take a look at what each field in the JSON object refering which are not described above:
* maxplayers: The number of players who played during that game.
* rounds: till which round the game went on, and is defined in the enum is as follows:
```
enum betting_round { 
preflop = 0, 
flop, 
turn, 
river
};
```
* game_state: An array of players info during the hand.
* bet_actions: Bet actions is defined by the following enum.
```
enum action_type { 
small_blind = 1, 
big_blind, 
check, 
raise, 
call, 
allin, 
fold 
};
```
Lets take `"bet_actions":	[5, 3, 3, 3]`  of player 1 which meaning that in the `preflop round` the players last bet action was `call`, and in the `flop`, `turn` and `river rounds` the players last bet action was `check`.

* Player_cards: This is an array of seven numbers, where each index represents the following cards as defined in the enum:
```
enum card_index { 
hole_card1 = 0, 
hole_card2, 
flop_card_1, 
flop_card_2, 
flop_card_3, 
turn_card, 
river_card 
};
```
The number at each index defines the cards as defined below:
```
	char *cards[52] = { "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC",
			    "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD",
			    "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH",
			    "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS" };
```
The corresponding graphic for each of the cards is defined in SVG is available here: https://github.com/chips-blockchain/pangea-poker/blob/master/src/components/Card/svg-sprite.css
