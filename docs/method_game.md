## Method: game
Once the homepage loads, as per the current game logic, DCV clicks on **game** which sends the below JSON message to the backend.
```json
 {"method":"game"}
```
Once the DCV backend receives the JSON method **game** it provides the below response.

```json
{
	"method":	"game",
	"game":	{
		"tocall":	0,
		"seats":	2,
		"pot":	[0],
		"gametype":	"NL Hold'em<br>Blinds: 3/6"
	}
}
```
Note here number of seats is Hardcoded to 2, **tocall** is optional here which I'm not using anywhere in the front end logic.

