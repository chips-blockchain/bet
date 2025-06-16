# GAME PLAY
Once the deck shuffling is complete, the game play logic controls the revealing of the shuffled deck, game movements, betting during the rounds, verification of game state and game information, and the settling of funds. At a high level, the game progresses through multiple states as the participating entities transition from one state to another.

All game state information is updated in the table ID by the dealer. Similar to how the dealer polls player IDs for updates, players poll the table ID for any updates. When a player reads the table ID and determines from the game state that an action is required, the player updates the corresponding action to their player ID.

This game state information is crucial for handling player disconnections.

## Different Game States

Currently, the following states have been identified, and this information will be updated as development progresses:

```
G_ZEROIZED_STATE      --> Zeroized state, Table is not initialized yet...
G_TABLE_ACTIVE        --> Table is active
G_TABLE_STARTED       --> Table is started
G_PLAYERS_JOINED      --> Players joined the table
G_DECK_SHUFFLING_P    --> Deck shuffling by players is done
G_DECK_SHUFFLING_D    --> Deck shuffling by dealer is done
G_DECK_SHUFFLING_B    --> Deck shuffling by cashier is done
G_REVEAL_CARD_B       --> Waiting for cashier to reveal blinding value
G_REVEAL_CARD_P       --> Waiting for player(s) to reveal the card
G_REVEAL_CARD_P_DONE  --> Player(s) received the card
```
