Deck Shuffling
# Deck Shuffling and Security in Verus Migration

The security of the game hinges on the deck shuffling process and the card revelation during gameplay. This document outlines the deck shuffling process using VDXF IDs.

## Deck Representation

Section 3 of the [Curve25519 paper](https://cr.yp.to/ecdh/curve25519-20060209.pdf) offers guidelines for generating random numbers that serve as private keys.

```
Legitimate users generate independent uniform random secret keys. For example, generate 32 uniform random bytes, clear bits 0, 1, 2 of the first byte, clear bit 7 of the last byte, and set bit 6 of the last byte.
```

Each card is represented by a 32-byte random number, with the 31st byte storing the card index (ranging from 1 to 52). This index maps to the corresponding card in the deck. Thus, a 52-card deck requires 52 unique 32-byte random numbers, totaling 1664 bytes.

## Deck Shuffling Process

The deck shuffling process involves multiple parties to ensure security:
1. **Player**: Chooses the deck, blinds it, and stores the deck information in the content multimap of the player ID.
2. **Dealer**: Reads, shuffles, blinds, encrypts the deck with the cashier's public keys, and updates the table ID with this deck information.
3. **Cashier**: Reads, decrypts, shuffles, blinds the deck, and updates the table ID with this deck information.

This process repeats for all players. If multiple cashiers are involved, each cashier shuffles and blinds the deck. At least one cashier must participate to enhance trust in the shuffling process. 

## Deck Revelation

After shuffling, all players have the deck in the same shuffled order. To reveal the deck, players must obtain the blinded values. The public values after shuffling are:

1. Shuffled deck.
2. Dealer's blinding values on Curve25519.
3. Cashier's Merkel hash of blinding values for each set of random numbers.

Steps to reveal a card:

1. Dealer announces the player's turn and the deck index.
2. Cashier reveals the blinding value for that player and index.
3. The player combines public and private information to reveal the card.
4. For board cards, all players reveal the card to each other.

Player shuffling/unshuffling and Shamir secret key sharing for revealing cashier's blinding values are omitted as they do not enhance security.

## Data Estimation

Estimate of data updated to ID during deck shuffling:

```
Deck size: 1664 bytes (52 cards * 32 bytes each)

For Players:
Each player has their own deck, so for n players: n * deck_size bytes.

For Dealers:
Dealer shuffles, blinds, encrypts each deck, and publishes blinding values: (n + 1) * deck_size bytes.

For Cashiers:
Cashiers shuffle, blind each deck, and publish Merkel proof of blinding values: (n + 1) * deck_size bytes.

Total:
Total bytes needed = n * deck_size + (n + 1) * deck_size + (n + 1) * deck_size = (3n + 2) * deck_size bytes.

For n = 9 players:
Total bytes = (3 * 9 + 2) * 1664 = 48,256 bytes ≈ 47.124 KB.
```

## Handling Deck Shuffling with VDXF IDs

Deck shuffling data is stored in the contentmultimap of the table ID. Each player has a dedicated key on the table for storing data. Identified keys and their updates:

1. `chips.vrsc::poker.t_table_info`: Table info, updated by the dealer.
2. `chips.vrsc::poker.t_player_info`: Player info (spot, status), updated by cashier upon receiving payin_tx.
3. `chips.vrsc::poker.t_player1` to `chips.vrsc::poker.t_player9`: Blinded deck info for each player.
4. `chips.vrsc::poker.t_dealer`: Encrypted dealer's blinded deck and Curve25519 points for blinding values.
5. `chips.vrsc::poker.t_blinder`: Cashier's blinded deck and Merkel proof of blinding values.

Note: In the current design, each player updates their deck information in their corresponding ID rather than updating the table ID. This approach avoids the hassle of adding the player's primary addresses to the table ID.
