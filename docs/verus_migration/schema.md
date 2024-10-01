# Verus Migration Schema

## Overview

This document outlines the schema for the tables used in our SQLite3 database, `pangea.db`, located at `~/.bet/db/pangea.db`.

## Table Schemas

### 1. player_deck_info

Stores information about the player's deck.

```sql
CREATE TABLE player_deck_info (
    game_id VARCHAR(100) PRIMARY KEY,
    pa_tx_id VARCHAR(100) NOT NULL UNIQUE,
    player_id INT,
    player_priv VARCHAR(100),
    player_deck_priv VARCHAR(4000)
);
```

### 2. dealer_deck_info

Stores information about the dealer's deck.

```sql
CREATE TABLE dealer_deck_info (
    game_id VARCHAR(100) PRIMARY KEY,
    perm VARCHAR(100),
    dealer_deck_priv VARCHAR(4000)
);
```

### 3. cashier_deck_info

Stores information about the cashier's deck.

```sql
CREATE TABLE cashier_deck_info (
    game_id VARCHAR(100),
    player_id INT,
    perm VARCHAR(100),
    cashier_deck_priv VARCHAR(4000),
    CONSTRAINT game_id PRIMARY KEY(game_id, player_id)
);
```

## Database Usage

We use an SQLite3 database for storing and retrieving information. The database file, `pangea.db`, is located at `~/.bet/db/pangea.db`.

## Table Updates and Disconnection Handling

Further discussion is required to detail when these tables get updated and how the information stored in them is used to handle disconnections.

Note: Since we are moving to use VDXF IDs for bet, we are not storing any information in the local database.# Verus Migration Schema
