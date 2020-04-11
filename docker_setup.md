## The Docker Image

There are number of dependancies and limitations in running the docker node. This docker image [sg777/bet:v1.1](https://hub.docker.com/repository/docker/sg777/bet) contains all the depencies along with chips,ln and bet repos are pre-compiled.

## Dependencies

### Port Dependancies

1. All the player nodes, DCV and BVV are connected in the backend on the ports `7797` and `7798`

    1. port `7797` is used to establish `pub-sub` relationship between the `DCV` and `{players, BVV}`.
    2. port `7798` is used to establish `pull-push` relationship between the `DCV` and `{players, BVV}`.
    
2. The player node needs to communicates with the cahsier node in order to lock and unlock the funds. The cahsier node runs on the port `7901`.

3. The Player, DCV and BVV nodes connects to the backend via GUI using the port `9000` by establishing the websocker connection.

### IP Address Dependancy

The backend node on which any of the entities of the game running must possess an IP address which is reachable over the internet. The condition to have a public ip address is mentioned below.

1. The Player, DCV and BVV nodes connects to the backend via GUI, by entering the IP address in the GUI. 
2. All the entities in the game must connect to DCV, i.e the Player and BVV nodes which are part of the game are connected to DCV node using the network sockets in the backend. For this to happen when the Player nodes starts in the backend we must mention the DCV IP address for which this Player node willing to connect.

## Running the docker

1. Pull the docker image
```
docker pull sg777/bet:v1.1
```
2. In case you already having chips and ln nodes running in your machine and if you would like to mount the already loaded blocks into the docker then run the docker as below else go to step 3.
 ``` 
 docker run --net=host --name bet -t -i -v /$HOME/.chips:/root/.chips:rw  -v /$HOME/.chipsln:/root/.chipsln:rw  sg777/bet:latest
 ```
3. Run the docker image
```
docker run --net=host --name bet -t -i sg777/bet:v1.1        
```
4. Run the `chipsd` and `lighningd`, followed by it run the player node.


