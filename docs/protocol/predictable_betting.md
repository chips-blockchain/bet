## Predicatable Betting
Here we discuss using bet how one can host and place bets over a predictable outcomes of an event.

## Actors in the system
The betting market comprises queries, answers to those queries and a mechanism to validate them. The actors in the system are as follows:
1. Host [Dealer in Poker] 
2. Validator Nodes [Cashier nodes]
3. Users

### Placing bets and evaluation
Any node can be a Host, the role of the host is to post and accept the bets. Host can also make private bets where it can only authorize few users to pariticipate in betting. All the bets must have the acceptance time window. 

With respect to evaluation of bets, bets can be classfied into two categories.
1. Evaluating bets based on the prorata basis.
2. Evaluating bets based on choosen odds.

__Prorata bets__ 
Lets say a simple bet **A wins on B**. The description of the bet is as follows:
```
{
	"bet_id":	0,
	"desc":	"A wins on B",
	"predictions":	"[true,false]",
	"range":	"[1, 500]"
	"time_lapse":   dd/mm/yyyy:ss/mm/hh[UTC]
	"type":         prorata
}
```
Lets say five users make bets on this query and their bets are as follows:

| User        | A wins on B | bet_amount |
| ----------- | ----------- | ---------- |
| U1	      | true	    |  100	 | 	
| U2	      | false	    |  200	 |
| U3	      | true	    |  400	 |
| U4	      | true	    |  300	 |
| U5	      | false	    |  100	 |

The cumulative bet amount collected on various possible predictions of the bet is as follows:
| Predictions | Cumulative bet_amount |
| ----------- | --------------------- |
| true	      | 800(100+400+300)      |
| false	      | 300(200+100)  	      |

Here in the prorata bets, the winning bet amount is calculated as follows:
Lets say the commision taken by host is 5% and validator nodes is 5% for simplicity. In that case 97% of the bet amount is distributed to the winners.
```
winning_bet_amount  = bet_amount+ 90%(all_bets_lost_amount) * (bet_amount/all_bets_win_amount)
```
Lets say **A wins on B** is validated to **true** meaning that A won over B. In that case users **[U1, U3, U4]** won the bet and users **[U2, U5]** lost the bet. The winning amount distributed to **U1** is as follows:

```
all_bets_win_amount = 800
all_bets_lost_amount = 300
U1_win_amount = 100 + [90%(300) * (100/800)] = 133.75
```

Like wise if we recompute all for all the users the final bet settelement table looks as follows:

| User        | A wins on B | bet_amount | bet_settlement |
| ----------- | ----------- | ---------- | -------------- |
| U1	      | true	    |  100	 | 133.75	  |
| U2	      | false	    |  200	 | 0		  |
| U3	      | true	    |  400	 | 535		  |
| U4	      | true	    |  300	 | 401.25	  |
| U5	      | false	    |  100	 | 0		  |


If you see above total amount collected is **1100** and the amount distributed to winners is **1070(133.75+535+401.25)** and the remaining **30** is distributed as commission to host and validator nodes.

__Odd based bets__
These are the bets in which the host defines the odds against the outcome of an event and match/map the bets of the users. Lets take a simple bet **A wins on B** in this scenario and its description is as follows:
```
{
	"bet_id":	0,
	"desc":	"A wins on B",
	"predictions":	"[true,false]",
	"range":	"[1, 500]"
	"time_lapse":   dd/mm/yyyy:ss/mm/hh[UTC]
	"type":         odd_based
	"If A wins":    80% of bet_amount
	"If B wins":    110% of bet_amount
}
```
In these type of betting scenarios, bets won't be locked immediately by the host until either the host finds a matching bet or the host himself fulfils the users bet. Lets say there is an user **U1** he places a bet that **A wins on B** with the prediction that A wins. So now the host has the following information, with the betting tx of the user is locked at the validators msig address.
```
{
	"bet_id":	0,
	"desc":	"A wins on B",
	"prediction":	"true",
	"bet_amount":	100
	"type":         odd_based
	"odds": 80% of bet_amount
	"tx": tx funded by the user to the msig_addr held by validator nodes
	"bet_status": pending 
}
```
 




### Listing the bets by host
In `bets.ini` host lists the bets and publish them to the players when the player node connect to the corresponding host. The sample congiguration data in the `bets.ini` looks as follows:
```
[bets]
[bets:0]
desc		= A wins on B
predictions	= [win,loss]
range		= [1, 100]
[bets:1]
desc		= B wins on C
predictions	= [win,loss]
range		= [1, 100]
[bets:2]
desc		= C wins on D
predictions	= [win,loss]
range		= [1, 100]
```

### Flow of events while betting
After connecting to backend from GUI, the GUI receives bet method from the backend which contains the following information
```
{
	"method":	"bets",
	"balance":	0.01770625,
	"open_bets":	[{
			"bet_id":	0,
			"desc":	"A wins on B",
			"predictions":	"[win,loss]",
			"range":	"[1, 100]"
		}, {
			"bet_id":	1,
			"desc":	"B wins on C",
			"predictions":	"[win,loss]",
			"range":	"[1, 100]"
		}, {
			"bet_id":	2,
			"desc":	"C wins on D",
			"predictions":	"[win,loss]",
			"range":	"[1, 100]"
		}],
		"placed_bets":[],
		"confirmed_bets":[],
}
```
From the GUI the player picks one or multiple bets and send the info to the BE, lets say the player selects the bet "A wins on B" in that case the info the GUI send to BE is in the fowm as shown below:
```
{
	"method":	"bets_selected",	
	"bets_info":	[{
			"bet_id":	0,
			"desc":	"A wins on B",
			"prediction":	"win",
			"bet_amount":	10
		}]
}
```
Once the player BE receives the bets chosen by the player, it makes the tx to the msig address held by cashier nodes and share the details with the host. Host then verifies the tx made by the player and change the status of the bets specific to that player and push the following info back to the player GUI via player BE.
```
{
	"method":	"bets",
	"balance":	0.01770625,
	"open_bets":	[{
			"bet_id":	0,
			"desc":	"A wins on B",
			"predictions":	"[win,loss]",
			"range":	"[1, 100]"
		}, {
			"bet_id":	1,
			"desc":	"B wins on C",
			"predictions":	"[win,loss]",
			"range":	"[1, 100]"
		}, {
			"bet_id":	2,
			"desc":	"C wins on D",
			"predictions":	"[win,loss]",
			"range":	"[1, 100]"
		}],
		"placed_bets":[{
			"bet_id":	0,
			"desc":	"A wins on B",
			"prediction":	"win",
			"bet_amount":	10
		}],
		"confirmed_bets":[],
}
```
say if any other player matches the bet player by the player, then the bet info is moved from placed_bets to confirmed_bets.
