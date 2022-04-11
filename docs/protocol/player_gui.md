## Playing with the player node

So you now have player backend node is running and by now you decided either to use the cashier/dealer hosted GUI or your own. 

By accessing the weblink from the player logs, or by accessing the weblink hosted by you at `http://localhost:1234/` you see the landing page like this. Here in this im using the GUI hosted by the dealer at `http://159.69.23.30:1234`.

![image](https://user-images.githubusercontent.com/8114482/139268469-57240190-1be5-4624-a911-b417e1d7f94e.png)

You see two tabs, public tables and private tables. For now by default click on `Private Table` and then you see Player and Dealer tabs, simply click on Player tab. The flow of events is like this:
```
Private Table --> Player
```
![image](https://user-images.githubusercontent.com/8114482/162731946-ea68ce5e-ca9c-4908-9cbe-14898c7becf3.png)

If you see in the text box it's asking you to enter the player IP address. This is the IPV4 address of the node in which the player node is running. If you are accessing the GUI from the node on which the player node is running then provide localhost IP `127.0.0.1` else provide the IP on which your player backend is running and in this case this IP must be public(Since im using websockets to communicate from GUI to backend, there are some issues in connecting if the IP is not public so please make sure the IP is public else try accessing the GUI from the node on which the player node is running).


Then player clicks on `Private Table --> Player` and enters the IP on which the backend node is running in the checkbox.
> This IP must be public, if the IP is not public then the GUI must be accessed from the local machine on which the player backend node is running and from GUI player connect to backend by entering the localhost IP.

After entering the IP, click on Set Nodes. You see the table with the number of seats set by the dealer, here in this case the dealer sets table size to 3. 
![image](https://user-images.githubusercontent.com/8114482/139269790-59f53e2b-11da-4bc6-a506-2ecefd0ac114.png)

Then you can choose any seat with `SIT HERE`, basically these are empty seats available and you can pick any of them, at this point I joined the table and simply wait for the other players. There isn't any time limit at the time of joining the table(so does that mean you have to wait indefinitely until the table gets filled, will add timer on it soon), once the table is joined there is a time limit of 10s in which the players has to make a move.

![image](https://user-images.githubusercontent.com/8114482/139270244-3c218efc-fde0-4fb4-b097-5eb14bce9c81.png)

Once all the players joins the table, one can able to view the cards and betting actions and from here on its just a poker which you know.
