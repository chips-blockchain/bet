# Heartbeat Protocol

As the name suggests, this protocol periodically checks and updates the every node in the game aout the status of all other nodes in the game. It's specially useful in identifying the scenarios where a player gets disconnected due to any of the reasons and based on this information `dealer` will wait for some pre-defined `threshold time` for the player to reconnect and if the player fails to reconnect with in that threshold time then the `dealer` considers the player move as `drop` and consider continuing the game with the rest of the active players on the game.

## How the protocol works

As it was defined in the `pangea` protocol that all the communications in the game must pass through the `dealer`.  In short dealer and all other entities in the game connected using `pub-sub` sockets over the port `7797` and `pull-push` sockets over the port `7798`.

The `dealer` publishes the `beacon` messages periodically at the intervals of defined `threshold_time`, here at the moment `threshold_time` is defined as `30 seconds` so for every `30 seconds` a `beacon` message is published.

The format and contents of the `beacon` message is as follows:
```
{
	"method":	"live"
}
```
The `playing nodes` and the `bvv` responds with the status, if the `dcv` didn't get any response from any of the node within the `threshold_time` then `dcv` assumes that the corresponding node got disconnected.

The response message from `bvv` is as follows:
```
{
	"method":	"live",
	"node_type":	"bvv"
}
```

```
Note: The heartbeat protocol gets triggered when the table is filled, it means by then each joined player in the game is associated with the playerid. For that reason you can see playerid in the response message from the player
```

The response message from the `player` is as follows:
```
{
	"method":	"live",
	"node_type":	"player",
	"playerid":	1
}

```

After summarizing the responses from all the nodes, `dcv` publishes the `status_info`. The contents of the `status_info` is as follows:
```
{
	"method":	"status_info",
	"bvv_status":	1,
	"no_of_players":	2,
	"players_status":	[1, 1]
}
```
The same `status_info` is forwarded to the respective GUI from the corresponding player node backends.
