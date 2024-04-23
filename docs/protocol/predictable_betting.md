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

### Prorata bets
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
Lets say the commision taken by host is 5% and validator nodes is 5% for simplicity. In that case the remaining 90% of the bet amount is distributed to the winners.
```
winning_bet_amount  = bet_amount+ 90%(all_bets_lost_amount) * (bet_amount/all_bets_win_amount)
```
Lets say **A wins on B** is validated to **true** meaning that A won over B. In that case users **[U1, U3, U4]** won the bet and users **[U2, U5]** lost the bet. The winning amount distributed to **U1** is as follows:

```
all_bets_win_amount = 800
all_bets_lost_amount = 300
U1_win_amount = 100 + [90%(300) * (100/800)] = 133.75
```

SO the final bet settlement amoutn for all the users based on the outcome A won over B is as follows:

| User        | A wins on B | bet_amount | bet_settlement |
| ----------- | ----------- | ---------- | -------------- |
| U1	      | true	    |  100	 | 133.75	  |
| U2	      | false	    |  200	 | 0		  |
| U3	      | true	    |  400	 | 535		  |
| U4	      | true	    |  300	 | 401.25	  |
| U5	      | false	    |  100	 | 0		  |


If you see above total amount collected is **1100** and the amount distributed to winners is **1070(133.75+535+401.25)** and the remaining **30** is distributed as commission to host and validator nodes.

### Odds based bets
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
Here the host can make a counterbet to the existng bets to change the status from pending to accepted or the host can just wait for the matching bet from the other users to confirm the pending bets.

## Evaluation of bet query

The most important thing in the predictable betting is the outcome of the event, and how this outcome of the event is legitimately provided to evaluate the bets. Here we are relying on the validator nodes to feed the outcome of bets and in return the validator nodes gets incentives in answering the bet queries. Its important to note here that the validator nodes are elected by the bet community who are trustworthy in the bet environment and in order to become a validator node one must need to meet certain constraints and guidelines set by the bet community.

To become a validator node one must
* Go through the election prcess, where the chips holders vote for a specific validator node applicant.
* The validator node must show the proof that they possess 50k CHIPS.

Whenever the host posts the bets, those bets are visible to all the validator nodes. The job of the validator nodes is based on the outcome of bet the validator nodes answers the queries and store them in the local in the local DB against the specific bet ID. 

Once the outcome of the event is revealed in the real world, the host sends requests to the validator nodes to get the outcome of the bet and evaluate the bet accordingly to settle the betting amounts. If the host fail to process the bet with in the time window specified, the users can raise dispute requests with the validator nodes where in which the validator nodes will process the outcome of the bet and settle the funds. The validator nodes gets incentivised for being honest in answering the bet queries and for being available in responding to the host nodes.

The incentives to the validator nodes are based on the queries it answers. The more queries it answers the more incentives it gets. We discuss in detail about how the system is resilient against the dishonest validator and host nodes, and how the dishonest will be moved out of the network and costs incurred for being dishonest in later sections.

## Lets talk about API's

### Listing the bets by host
Host nodes are the ones who are responsible in creating the prediction markets by posting the bets. In `bets.ini` host lists the bets and publish them to the players when the player node connect to the corresponding host. The sample congiguration data in the `bets.ini` looks as follows:
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
