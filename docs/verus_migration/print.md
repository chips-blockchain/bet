# PRINT
We store the information in the IDs by encoding them into hex, making the data retrieved via Verus APIs like `getidentity` difficult to read. To address this, we have developed Bet APIs to parse this data and present it in a human-readable format. This document discusses the APIs available to print the data of an ID or key in a human-readable format.

#### Printing Complete ID
To print the complete ID, use the following command:
```
./bet print_id <id_name> <id_type>
```
- `id_name`: A valid ID name.
- `id_type`: The high-level functionality of the ID, such as table, dealer, dealers, etc.

For example, to print the information of a table named `sg777_t`, use the following command:
```
./bet print_id sg777_t table
```

#### Printing Specific Key Data of a Table
To print specific key data of a table, use the following command:
```
./bet print_table_key <table_id> <table_key_name>
```
For example, to print the table information of the table `sg777_t`, use the following command:
```
./bet print_table_key sg777_t t_table_info
```

#### Printing Specific Key Data of a Specific ID
To print specific key data of a specific ID, use the following command:
```
./bet print <id_name> <key_name>
```
- `id_name`: A valid existing ID name.
- `key_name`: Any key name that has associated data in that ID.

For example, to print specific key data of an ID, use the following command:
```
./bet print <id_name> <key_name>
```
