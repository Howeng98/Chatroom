# Chatroom
socket and multithread to implement a multiple clients chatroom

## Compile
```
gcc -o server server.c -lpthread
```
```
gcc -o client client.c -lpthread
```
Or you can just use make command for compile
```
make
```

## Execute
```
./server 
```
Please execute a few clients for implement multi-clients
```
./client
```

After create the server and clients is binding and connecting,please enter the username for each client to configure the identity in the chatroom
