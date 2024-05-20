# gamescope-control-protocol
A protocol thingy to use with gamescope stuff

## INFO
You will need to use wayland-scanner and the gamescope-control.xml from your gamescope source.

## Generate header and private code
```
wayland-scanner client-header < gamescope-control.xml > gamescope-control-client-protocol.h
wayland-scanner private-code < gamescope-control.xml > gamescope-control-client-protocol.c
```

## BUILD

```
gcc gamescope-control.c gamescope-control-client-protocol.c -o gamescope-control -lwayland-client
```
