# GAME PLAY

Once the deck shuffling is done, revealing of shuffled deck, game movement, betting during the rounds, verification of game state and game info and settling of funds are controlled by the gmae play logic. At very high level there exists multiple states and the game progresses as the pariticipating entitites transition from one state to another. 

All the game state info is updated in the table ID by the dealer, like the way dealer keeps on polling on the player IDs for the player updates, players keeps on polling on table ID for any updates to it. As and when the player reads the table ID and from the game state if it finds out that player needs to take an action then the player updates that corresponding action to the player ID.

This game state info is very crucial to handle the player disconnections. 

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
