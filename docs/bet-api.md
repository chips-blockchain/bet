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

