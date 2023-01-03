# PRINT

We store the info on to the ID's by encoding them into hex, so the info that you see by using verus API;s like getidentity is not human readable. Here we discuss about the API's we have in order to print the data of an ID or key in a human readable form.

#### To print complete ID
```
./bet print_id id_name id_type

where id_name is the any name valid ID name.
id_type is basically the highlevel functionality of that ID, like table, dealer, dealers, etc...

For example if I want print the info of a talbe sg777_t, here is the following command:
./bet print_id sg777_t table
```

#### To print specific key data of a table
```
./bet print_table_key talbe_id table_key_name

For example, the following command prints the table info of the table sg777_t.
./bet print_table_key sg777_t t_table_info
```
#### To print specific key data of a specific ID
```
./bet print id_name key_name

Where id_name is the any existing valid ID name and 
where as key_name is the any key name that has the data associated to it in that ID.
