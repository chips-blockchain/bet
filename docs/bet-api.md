# Usage guide of the bet-api's
## Create a player
In the context of the game the identity of the player is a point on the Curve25519 which is known to every participating entity in the game.
```
./bet-cli create-player
{
	"command":	"create-player",
	"PrivKey":	"60fdabae9655c77cd3cfe464009ac6f832c9b89ccaccdbec931f88977f04ec59",
	"PubKey":	"d71680c191006e136532526971afd0b07514f0e79284f1585c56542834d43258"
}
```
## Create a deck
Every asset in the game is a point the Curve25519, whether it could a card in the deck, a number in the dice or a side of the coin. In the context of the game the identity
of an asset is only revealed during the game when and where it is needed.
```
./bet-cli create-deck numOfCards
For example: ./bet-cli create-deck 6(sides on a dice or a deck of six cards numbered 0-5)
{
	"command":	"create-deck",
	"Number Of Cards":	6,
	"CardsInfo":	[{
			"Card Number":	0,
			"PrivKey":	"604bb6942334f6902f5f6b599cf47bb2db04707e5399b5dadb94c9fec7090043",
			"PubKey":	"c740539581980369d5aed65c30a23f2b3fd02f439a5f09d958731718f78b836a"
		}, {
			"Card Number":	1,
			"PrivKey":	"e810d9255bd1286fa58865922bf443b6bd9904808f0e87527a2c608ae8fb0173",
			"PubKey":	"4cb4dc4ef49b6340f81dc32d21a50e2311cc65691d13ce8d0b987b96262df557"
		}, {
			"Card Number":	2,
			"PrivKey":	"b8ffcae92e652efeb9b60a1c1123bf4e040814916ee488bf301b2b55c3230261",
			"PubKey":	"f5c388cabb37a95b8dc99cc8415d83c2e0983f117791e9df68e174375595fe3b"
		}, {
			"Card Number":	3,
			"PrivKey":	"7896ef0a31aefc693fb09dcc20bd32855e41b326761961388f59ae011afe0342",
			"PubKey":	"2ecb90b543f83d34071cf4ac36b93e2170cb9fccb53991ff1ffdf7cf4af49006"
		}, {
			"Card Number":	4,
			"PrivKey":	"60ae2822ca90b27b3c96e2715ea557bc125326ffd1b6ac16f4682e3a06600459",
			"PubKey":	"73fff75b7bae128a2da6669af2092c0e6063b9991d6b1438fe5be6456acfe04c"
		}, {
			"Card Number":	5,
			"PrivKey":	"f01da8326892c08605ba95eec75b81e711233a106c3e44ee1927b2184a860553",
			"PubKey":	"6eca0d2f01cb07728431f633caa7d0afa7e26d42f3d169e359883171e9770c5d"
		}]
}
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
