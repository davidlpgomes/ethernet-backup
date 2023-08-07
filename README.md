# Ethernet Backup
A server-client backup program that runs on the ethernet interface using a raw socket. 

## Build
Run `make`.

To print debug logs, run `make debug`.

## Usage
`sudo` is needed in order to create a raw socket on Linux.

``` 
usage: sudo ./backup [-c] [-s] [-l]

options:
  -c        run as client (default)
  -s        run as server
  -l        run with the loopback interface (used for debugging)
```
