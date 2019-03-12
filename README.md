# Own Container
Original container runtime.

Own Container is simple container runtime.

It can use User-Namespace.

I would appreciate it if you would help improve.

## Repository name
Repository name may change.

## TODO
Immediate:
- User specification

More:
- Original entrypoint
- Seccomp
- CRIU
- Network namespace
- Container image

## Usage
```
# build with debug option.
./build.sh --debug
# get alpine linux rootfs. It is unnecessary if arbitrary rootfs exists.
./get-alpine.sh
# spawn container. Arbitrary rootfs can be specified as an argument.
./bin/container ./alpine-root/

```

Options
```
* [-t] trace systemcall
* [-P] execute privilege container
* [-b <host-absolute-path>:<container-absolute-path>] mount host path into container
```

