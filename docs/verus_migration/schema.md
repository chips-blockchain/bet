Here we see what info we need to store locally in order to handle two main functionalities of bet, i.e disconnections and disputes. The information that we need to store is mainly either be the data confidential to the corresponding entity or the info that is needed to track back the leftout position or to raise and resolve the disputes. 

At high level here is the schema of the tables where in which we store this info.
```
1. player_deck_info
CREATE TABLE player_deck_info (game_id varchar(100) primary key, pa_tx_id varchar(100) NOT NULL UNIQUE, player_id int, player_priv varchar(100), player_deck_priv varchar(4000))

2.dealer_deck_info
CREATE TABLE dealer_deck_info (game_id varchar(100) primary key, perm varchar(100), dealer_deck_priv varchar(4000))

3. cashier_deck_info
CREATE TABLE cashier_deck_info (game_id varchar(100), player_id int, perm varchar(100), cashier_deck_priv varchar(4000), CONSTRAINT game_id PRIMARY KEY(game_id, player_id))
```

We further disucss about where these tables gets updated and how the info stored in them used to handle the disconnections and also see in detail in the following sections.
