## Predicatable Betting
Here we discuss using bet how one can host and place bets over a predictable outcomes of an event.

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
