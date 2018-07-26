# Usage guide of the bet-api's
## Create a player
In the context of the game the identity of the player is a point on the Curve25519 which is known to every participating entity in the game.
```
./bet-cli create-player
Player PubKey: "{\n\t\"command\":\t\"create-player\",\n\t\"PubKey\":\t\"67f48ceef325897d9282091e27b03fa289899cab5758e61767ee619f21b2e465\"\n}"
{
	"command":	"create-player",
	"PubKey":	"67f48ceef325897d9282091e27b03fa289899cab5758e61767ee619f21b2e465",
	"PrivKey":	"287286ed2ce994ff4a2013b0e459c8023c35d8c9105b97aa37aee4fbcd3b6362"
}
```
## Create a deck
Every asset in the game is a point the Curve25519, whether it could a card in the deck, a number in the dice or a side of the coin. In the context of the game the identity of an asset is only revealed during the game when and where it is needed.
```
./bet-cli create-deck numOfCards
For example: ./bet-cli create-deck 2(sides on a coin or a deck of two cards numbered 0-1)
{
	"command":	"create-deck",
	"Number Of Cards":	2,
	"CardsInfo":	[{
			"Card Number":	0,
			"PrivKey":	"50cc3735750111c40222fdea12817e223766e784dff259fb03ecddf8eb41005d",
			"PubKey":	"139ca3f6c01d7eeb33f67ebf80605b795b7f8180b7f69d3b22f603c2b0c78f70"
		}, {
			"Card Number":	1,
			"PrivKey":	"98bca13dc70582c32bc85c4f096a31567c01b53c684ee10bd2abc38fb9aa0167",
			"PubKey":	"6b2b54d56ba49cdf47aee403f98ed18bfc13cae2ca7cc38a2a5640aebea8095c"
		}]
}

"{\n\t\"command\":\t\"create-deck\",\n\t\"Number Of Cards\":\t2,\n\t\"CardsInfo\":\t[{\n\t\t\t\"Card Number\":\t0,\n\t\t\t\"PrivKey\":\t\"50cc3735750111c40222fdea12817e223766e784dff259fb03ecddf8eb41005d\",\n\t\t\t\"PubKey\":\t\"139ca3f6c01d7eeb33f67ebf80605b795b7f8180b7f69d3b22f603c2b0c78f70\"\n\t\t}, {\n\t\t\t\"Card Number\":\t1,\n\t\t\t\"PrivKey\":\t\"98bca13dc70582c32bc85c4f096a31567c01b53c684ee10bd2abc38fb9aa0167\",\n\t\t\t\"PubKey\":\t\"6b2b54d56ba49cdf47aee403f98ed18bfc13cae2ca7cc38a2a5640aebea8095c\"\n\t\t}]\n}"
```
## Blind Deck
This player takes the deck as input and blinds the privkey of deck by multiplying it with the pubkey of the player. This way a player can conceal the identity of the deck.
```

./bet-cli blind-deck deckOfCards playerPubKey

./bet-cli blind-deck "{\n\t\"command\":\t\"create-deck\",\n\t\"Number Of Cards\":\t2,\n\t\"CardsInfo\":\t[{\n\t\t\t\"Card Number\":\t0,\n\t\t\t\"PrivKey\":\t\"50cc3735750111c40222fdea12817e223766e784dff259fb03ecddf8eb41005d\",\n\t\t\t\"PubKey\":\t\"139ca3f6c01d7eeb33f67ebf80605b795b7f8180b7f69d3b22f603c2b0c78f70\"\n\t\t}, {\n\t\t\t\"Card Number\":\t1,\n\t\t\t\"PrivKey\":\t\"98bca13dc70582c32bc85c4f096a31567c01b53c684ee10bd2abc38fb9aa0167\",\n\t\t\t\"PubKey\":\t\"6b2b54d56ba49cdf47aee403f98ed18bfc13cae2ca7cc38a2a5640aebea8095c\"\n\t\t}]\n}" "{\n\t\"command\":\t\"create-player\",\n\t\"PubKey\":\t\"67f48ceef325897d9282091e27b03fa289899cab5758e61767ee619f21b2e465\"\n}"

Blinded Deck:
{
	"command":	"blind-deck",
	"BlindDeck":	{
		:	{
			"Card Number":	0,
			"PrivKey":	"50cc3735750111c40222fdea12817e223766e784dff259fb03ecddf8eb41005d",
			"BlindPrivKey":	"fafed6f30d85b6e132c56b4306bd03bcd3625f3a87fce6d7f07bdc30fba75873"
		},
		:	{
			"Card Number":	1,
			"PrivKey":	"98bca13dc70582c32bc85c4f096a31567c01b53c684ee10bd2abc38fb9aa0167",
			"BlindPrivKey":	"8efc6d83f53d26954bbc238198e785046d8b1f9f191b68c782630cfe55013a47"
		}
	}
}

"{\n\t\"command\":\t\"blind-deck\",\n\t\"BlindDeck\":\t{\n\t\t:\t{\n\t\t\t\"Card Number\":\t0,\n\t\t\t\"PrivKey\":\t\"50cc3735750111c40222fdea12817e223766e784dff259fb03ecddf8eb41005d\",\n\t\t\t\"BlindPrivKey\":\t\"fafed6f30d85b6e132c56b4306bd03bcd3625f3a87fce6d7f07bdc30fba75873\"\n\t\t},\n\t\t:\t{\n\t\t\t\"Card Number\":\t1,\n\t\t\t\"PrivKey\":\t\"98bca13dc70582c32bc85c4f096a31567c01b53c684ee10bd2abc38fb9aa0167\",\n\t\t\t\"BlindPrivKey\":\t\"8efc6d83f53d26954bbc238198e785046d8b1f9f191b68c782630cfe55013a47\"\n\t\t}\n\t}\n}"
```
## Player Join Request
This API allows the player to make join request to the Deck Creating Vendor(Dealer) and in response it gets peerid assigned by DCV. The prerequisite for this is to make sure a Dealer is already up and running to get the response.
```
./bet-cli join-req playerPubKey srcBindAddr destBindAddr

In the below example player node and dealer node are communicating using IPC.

./bet-cli join-req "{\n\t\"command\":\t\"create-player\",\n\t\"PubKey\":\t\"5094b1e04e91b16d1a099fcaa25f5618c912a0b53cd196c16050875843eb095f\"\n}" "ipc:///tmp/bet.ipc" "ipc:///tmp/bet1.ipc"
nntype.80 connect to ipc:///tmp/bet1.ipc connectsock.1
nntype.33 connect to ipc:///tmp/bet.ipc connectsock.2

Response Received:{
	"method":	"join_res",
	"peerid":	0,
	"pubkey":	"5094b1e04e91b16d1a099fcaa25f5618c912a0b53cd196c16050875843eb095f"
}
```
