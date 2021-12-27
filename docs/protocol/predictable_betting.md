## Predicatable Betting
Here we discuss using bet how one can host and place bets over a predictable outcomes of an event.

## Actors in the system
The betting market comprises queries, answers to those queries and a mechanism to validate them. The actors in the system are as follows:
1. Host [Dealer in Poker] 
2. Validator Nodes [Cashier nodes]
3. Users

### Host
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
	"predictions":	"[win,lose]",
	"range":	"[1, 500]"
	"time_lapse":   dd/mm/yyyy:ss/mm/hh[UTC]
}
```
Lets say five users make bets on this query and their bets are as follows:

| User        | A wins on B | bet_amount |
| ----------- | ----------- | ---------- |
| U1	      | win	    |  100	 | 	
| U2	      | lose	    |  200	 |
| U3	      | win	    |  400	 |
| U4	      | win	    |  300	 |
| U5	      | lose	    |  100	 |

The cumulative bet amount collected on various possible predictions of the bet is as follows:
| Predictions | Cumulative bet_amount |
| ----------- | --------------------- |
| win	      | 800(100+400+300)      |
| lose	      | 300(200+100)  	      |

Here in the prorata bets, the winning bet amount is calculated as follows:
\delta
winning_bet_amount  = bet_amount+ 




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
