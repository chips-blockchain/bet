## TX's flow during the game
Based on the context the tx's happen during the game are classified into [various types](./tx_types.md). Here we see in a typical two player poker game how the transactions happen and will also see the understand the details about what the data part of the tx tells.

Each and every poker game that is played is associated with a unique table_id which is same for all the players during the game. Here i'm taking the example of game that is already played and walk through the details.

### The local DB
All the tx's happen during the game along with the table_id are stored in a local DB which is located at `~/.bet/db/pangea.db`. The `game_info` table stores the tx and table_id details and schema of `game_info` is as follows:
```
sqlite> .schema game_info
CREATE TABLE game_info (tx_id varchar(100) primary key,table_id varchar(100));
```

### The game played

Let's take a look into the game which is already played, the table_id of the game is `7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89`.

The `player1` tx's:

| tx_id                                                            |  table_id                                                          |
|:-----------------------------------------------------------------|:-------------------------------------------------------------------|
| 23b6f1b8170bf62397222a233f949e4672e1bfd57e919aa407ff8d357f8b67cf |  7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| 07a5e8b04a3b9165e188650af3e387af2387fc9fee55980f9b97dea4913d758e |  7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| 3f02176f3eb1ca4975e5022ef98d01d5dd6de1f1eb5078539efabf7df4707d8f |  7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  | 
| 7d58e742acbb2b3bc5a1bba0ca36887d80ca52455ea487eaa4154061b6b62680 |  7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  | 
| 7ba82e800b6c9a22a3b576e85fd326a19bbb550dbc0107960f99cfe63504ab25 |  7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| d420a2a2639dfcec5ba2a37e303ca86c6bb6861787f780580e1ee911941c5df9 |  7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| **8c0c074f0dac06f7675926a4ad3e6abe51a7f0ad28a417e0d23090550b3a6a28** |  7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |

The `player2` tx's:

| tx_id                                                             | table_id                                                          |
|:------------------------------------------------------------------|:------------------------------------------------------------------|
| 3e0d381b3d8fd3811b48717c9fef26b97eec91b5520536ab7ea25b60d440b201  | 7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| a287c5baa97a60523917a3ef69ec59370176ca38195c3a2759a1e0f56584f113  | 7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| 71d5406f00f52954e24071d7f95e79b9c1bbb07c4d968a7385aedfdf911477db  | 7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| ea55e851a5f5cde07fe67d079dbf3c315eacb75f639a7f0bd2b0dc2c832826db  | 7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| 7326071fdfdabbdaa500b1a010c995e3c5d4ffbcf961451074f316fe65e10fb5  | 7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| 58ec6cbcef7a223405d77b78e6fbf0425d2c3333105a586f99160f479e4ca26b  | 7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |
| **8c0c074f0dac06f7675926a4ad3e6abe51a7f0ad28a417e0d23090550b3a6a28**  | 7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89  |

>Note: If you notice the last tx for both players is same(which I highlighted in bold) which is the payout tx.

### The payin tx's

Player1 joins the table by making the payin_tx `23b6f1b8170bf62397222a233f949e4672e1bfd57e919aa407ff8d357f8b67cf` and in the data part of this tx contains the info about cashier nodes and info needed to resolve the disputes in case.

```
~/bet/privatebet$ ./bet extract_tx_data 23b6f1b8170bf62397222a233f949e4672e1bfd57e919aa407ff8d357f8b67cf
[bet.c:bet_start:449] Data part of tx 
 {
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"msig_addr_nodes":	"[\"141.94.227.65\", \"141.94.227.66\", \"141.94.227.67\", \"141.94.227.68\"]",
	"min_cashiers":	2,
	"player_id":	"8a6c0e142d392132134f2efc211f30fdda6e451627e7bddd1f5a55112790f336",
	"dispute_addr":	"bHx15agAepVag8xH4Ryw7e4rXiDVeRhCfu",
	"msig_addr":	"bJpRNkKYXweiLDzR1kNYpgZRwYaakMdeSb"
}
```

Same goes for player2
```
~/bet/privatebet$ ./bet extract_tx_data 3e0d381b3d8fd3811b48717c9fef26b97eec91b5520536ab7ea25b60d440b201
[bet.c:bet_start:449] Data part of tx 
 {
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"msig_addr_nodes":	"[\"141.94.227.65\", \"141.94.227.66\", \"141.94.227.67\", \"141.94.227.68\"]",
	"min_cashiers":	2,
	"player_id":	"9b60cf6e672d7bdc7d7c5b6792d8334b644108ba3b7ae3d117f1e5e31b7d408e",
	"dispute_addr":	"bXfLHQF51PcGrUhtX2nFjVAX2MUJbfeNDs",
	"msig_addr":	"bJpRNkKYXweiLDzR1kNYpgZRwYaakMdeSb"
}
```

### The dust tx's to record game moves

After making the payin tx's dealer verifies whether if the payin tx's are valid or not and based on which the dealer allows the player nodes to join the table. Once players joins the table, deck shuffling will take place and then players can play the game with betting. All the betting moves that happen during the game are recorded in the chips blockchain by making dust tx's or zero output tx's. Keeping the betting information in the data part of the dust tx solves two purposes for us
* Non-repudiation, i.e player can't deny its move at later point it time and also player can use this info to raise disputes.
* Easy to showcase the games played in the explorer.

Below we see lots of magic numbers in the tx data part, so here im keeping the corresponding arrays to signify what these numbers are, GUI devs can take the below enums and arrays as reference in showcasing them on the user.
```
enum action_type { small_blind = 1, big_blind, check, raise, call, allin, fold };
enum round_type {pre_flop = 0, flop = 1, 3rd_round = 2, 4th_round = 3};
char *cards[52] = { "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "10C", "JC", "QC", "KC", "AC",
          			    "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "10D", "JD", "QD", "KD", "AD",
			              "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "10H", "JH", "QH", "KH", "AH",
			              "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "10S", "JS", "QS", "KS", "AS" };
So card_value 0 means it is 2C, i.e 2 Club.              
```
Now lets go through the player moves during each round of betting

#### Preflop betting round.

Player1 - Small blind
```
~/bet/privatebet$ ./bet extract_tx_data 07a5e8b04a3b9165e188650af3e387af2387fc9fee55980f9b97dea4913d758e
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	0,
	"playerID":	0,
	"betAmount":	1,
	"action":	1
}
```

Player2 - Big blind
```
~/bet/privatebet$ ./bet extract_tx_data a287c5baa97a60523917a3ef69ec59370176ca38195c3a2759a1e0f56584f113
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	0,
	"playerID":	1,
	"betAmount":	2,
	"action":	2
}
```

Player1 - Call
```
~/bet/privatebet$ ./bet extract_tx_data 3f02176f3eb1ca4975e5022ef98d01d5dd6de1f1eb5078539efabf7df4707d8f
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	0,
	"playerID":	0,
	"betAmount":	1,
	"action":	5
}
```

Player2- Check
```
~/bet/privatebet$ ./bet extract_tx_data 71d5406f00f52954e24071d7f95e79b9c1bbb07c4d968a7385aedfdf911477db
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	0,
	"playerID":	1,
	"betAmount":	0,
	"action":	3
}

```

#### Flop betting round


Player2 - Raise
```
~/bet/privatebet$ ./bet extract_tx_data ea55e851a5f5cde07fe67d079dbf3c315eacb75f639a7f0bd2b0dc2c832826db
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	1,
	"playerID":	1,
	"betAmount":	2,
	"action":	4
}
```

Player1 - Call
```
~/bet/privatebet$ ./bet extract_tx_data 7d58e742acbb2b3bc5a1bba0ca36887d80ca52455ea487eaa4154061b6b62680
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	1,
	"playerID":	0,
	"betAmount":	2,
	"action":	5
}
```

#### 3rd round betting
Player2 - Check
```
~/bet/privatebet$ ./bet extract_tx_data 7326071fdfdabbdaa500b1a010c995e3c5d4ffbcf961451074f316fe65e10fb5
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	2,
	"playerID":	1,
	"betAmount":	0,
	"action":	3
}
```

Player1 - Check
```
~/bet/privatebet$ ./bet extract_tx_data 7ba82e800b6c9a22a3b576e85fd326a19bbb550dbc0107960f99cfe63504ab25
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	2,
	"playerID":	0,
	"betAmount":	0,
	"action":	3
}
```

#### 4th round betting

Player2 - Raise
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

Player1 - Call
```
~/bet/privatebet$ ./bet extract_tx_data d420a2a2639dfcec5ba2a37e303ca86c6bb6861787f780580e1ee911941c5df9
[bet.c:bet_start:449] Data part of tx 
 
	"method":	"bet",
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"round":	3,
	"playerID":	0,
	"betAmount":	2,
	"action":	5
}
```

#### Showdown and payout tx

Payout tx
```
~/bet/privatebet$ ./bet extract_tx_data 8c0c074f0dac06f7675926a4ad3e6abe51a7f0ad28a417e0d23090550b3a6a28
[bet.c:bet_start:449] Data part of tx 
 {
	"table_id":	"7e6520211912ac0e1e5c593c93c50991a3408d9adc983b6f81a5d319c1ddda89",
	"maxplayers":	2,
	"rounds":	3,
	"game_state":	[{
			"bet_actions":	[5, 5, 3, 5],
			"player_cards":	[5, 14, 46, 26, 4, 43, 37]
		}, {
			"bet_actions":	[3, 4, 3, 4],
			"player_cards":	[3, 13, 46, 26, 4, 43, 37]
		}],
	"player_ids":	["8a6c0e142d392132134f2efc211f30fdda6e451627e7bddd1f5a55112790f336", "9b60cf6e672d7bdc7d7c5b6792d8334b644108ba3b7ae3d117f1e5e31b7d408e"],
	"threshold_value":	2,
	"msig_addr_nodes":	["141.94.227.65", "141.94.227.66", "141.94.227.67", "141.94.227.68"]
}
```

### TX Scanner

So far we seen how various tx's happen during the game. **Here comes the question like how can these be grouped and be available to all the players in the chips explorer?**
For that we are writting the blockchain scanner, which has be to be run by the explorer node in the backend. What this scanner will do is it groups by all the tx's based on the table_id. So that way all the tables that played will be visible on the explorer. More details of this scanner will be shared soon.
