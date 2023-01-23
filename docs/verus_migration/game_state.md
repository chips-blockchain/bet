# GAME PLAY

Once the deck shuffling its done, how the shuffled deck gets revealed, how the game moves and betting happens, how the verification of game state and settlement of funds happen is controlled by game play logic. At very high level there exists multiple states and the game progresses as the pariticipating entitites transition from one state to another. All the game state info is updated in the table ID, the game state info is crucial to handle the player disconnections. Any information that is verifiable is stored in the ID's and the information that is confidential to the player is stored in the local DB. The combination of info on local DB along with the info that was stored in the ID's is used for player rejoin and to raise the disputes. 

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
