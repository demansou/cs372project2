# cs372project2
2-connection client/server network application for file transfer

## ftserver.c
Server written in C code. Upon runtime will print server IP and listening port so that user can connect with ftclient.

### Compile
```
gcc -o ftserver ftserver.c
```

### Run
```
ftserver [listening port]
```
Run from command line. You will need to define a port to listen to incoming data.

### Options for clients
```
-l
```
'-l' will list all txt files in directory of ftserver. Requires client to define a data port so that list can be sent to client.

```
-g
```
'-g' will send an existing .txt file in directory of ftserver to client. Requires client to define txt file and data port so that file can be sent to client

## ftclient.py
Client written in Python2.7 code. Requires ftserver listening on a port.

### Compile
Python does not require compilation prior to running the client program.

### Run
```
List of txt files:
python ftclient.py [ftserver ip] [ftserver port] [-l] [data port]

Retrieve txt file over network:
python ftclient.py [ftserver ip] [ftserver port] [-g] [filename] [data port]
```

### Options for clients
```
-l
```
Requires inputting an open data port on client in command line argument in addition to server ip and server port

```
-g
```
Requires inputting a filename and open data port on client in command line argument in addition to server ip and server port

