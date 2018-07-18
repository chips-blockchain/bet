# Usage guide of the bet-api's
## Create a player
In the context of the game the identity of the player is a point on the Curve25519 which is known to every participating entity in the game.
./bet-cli create-player
'{
	"command":	"create-player",
	"PrivKey":	"60fdabae9655c77cd3cfe464009ac6f832c9b89ccaccdbec931f88977f04ec59",
	"PubKey":	"d71680c191006e136532526971afd0b07514f0e79284f1585c56542834d43258"
}'
## Create a deck
Every asset in the game is a point the Curve25519, whether it could a card in the deck, a number in the dice or a side of the coin. In the context of the game the identity
of an asset is only revealed during the game when and where it is needed.
./bet-cli create-deck numOfCards
For example: ./bet-cli create-deck 6(sides on a dice or a deck of six cards numbered 0-5)
'{
	"command":	"create-deck",
	"Number Of Cards":	6,
	"Card Number":	0,
	"PrivKey":	"582de1147a5fcbc99ce6c327338a9d85d372040aa4ed667b55d956c06e380046",
	"PubKey":	"736dde64d2975e3fdf5b5f419921087dcc8b4a6adf1754f900ca2e7d3e1b2742",
	"Card Number":	1,
	"PrivKey":	"58116b47713f8ad810a86bb571bc66fd67ce1df663df256a1a3d47cf5e27014d",
	"PubKey":	"69564c54f1821f8d1f9b1be1db1381976b3c77ebb25a84aa0f8b79f9ae823156",
	"Card Number":	2,
	"PrivKey":	"4075d76aff6f595798d9fd0c33734625f8563d3b36e36979c7d3e320d7320270",
	"PubKey":	"23acd2336d7279eec3eeb2ecd97a2e3f8570ac38ed2dcbef8c881b0a5f3f8f34",
	"Card Number":	3,
	"PrivKey":	"78cbdfdc42de5deeffa729edf8bfffe64d884faba011bca142c84c2ba5610379",
	"PubKey":	"b03a2b4247a1ee6ce0886c33534f2264e3aa70cf08d22d7031202b90ab68c374",
	"Card Number":	4,
	"PrivKey":	"f80efb5b180d242a74d932422d8bc63b8e97f07e015f0721573346fd40390463",
	"PubKey":	"3e5a98e2c677be59aab9efefe223b8975d33c011d669b06b0c8c72bce2ecb465",
	"Card Number":	5,
	"PrivKey":	"d879a2befbe27e1f9749be3691b665b9e846f04aaabeaff48714d60e94e70541",
	"PubKey":	"8ce71224f82a4bc11e3dbe9654c1f2224f9ca4760f2eb8ee87fb86029600b872"
}'

