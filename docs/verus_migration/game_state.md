# GAME PLAY

Once the deck shuffling its done, in what order the shuffled deck is revealed and how the game is moved and betting is done is controlled by the game play logic. At high level there exists multiple states and the game progresses as the pariticipating entitites transition from one state to another. This game state info is also plays a crucial role in handling the player disconnections. The combination of game_state info with the local DB can be used by the players to reconnect and to raise the disputes.

## Different Game States

Atm, here are the following states that are identified and this info is updated as we progress our development.
```
G_ZEROIZED_STATE      --> Zeroized state, Table is not initialized yet...
G_TABLE_ACTIVE        --> Table is active
G_TABLE_STARTED       --> Table is started
G_PLAYERS_JOINED      --> Players joined the table
G_DECK_SHUFFLING_P    --> Deck shuffling by players are done
G_DECK_SHUFFLING_D    --> Deck shuffling by dealer is done
G_DECK_SHUFFLING_B    --> Deck shuffling by cashier is done
G_REVEAL_CARD_B       --> Waiting for cashier to reveal blinding value
G_REVEAL_CARD_P       --> Waiting for player(s) to reveal the card
G_REVEAL_CARD_P_DONE  --> Player(s) got the card
```
