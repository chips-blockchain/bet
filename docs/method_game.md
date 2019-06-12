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
The significance of each field is mentioned as follow:
* __tocall__: optional
* __seats__: This value describes the number of seats. Here number of seats is Hardcoded to 2 in the backend logic. Ideally this value is something *DCV* should be chosen from the front end GUI. But the GUI which this backend repo using doesn't have that provision, for those reasons this value is hardcoded.
* __pot__: This is actually an array, It holds the values of mainpot and sidepots, Initially all these values are set to zero.
* __gametype__: This is a string which gets updates in the top right corner of the GUI as shown below. Ideally this value is constant, here we are dealing with Texas Hold'em poker the value is **NL Hold'em<br>Blinds: 3/6**

![image](./images/after_seats.png)

[__prev__](./messageFormats.md)  [__next__](./method_seats.md)
