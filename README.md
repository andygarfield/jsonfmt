# jsonfmt
A Simple JSON formatter. Not crazy fast, but 2-3 times faster than [jq](https://jqlang.org/).

## building
### mac
Requires clang
```sh
make jsonfmt_mac
mv jsonfmt_mac jsonfmt
```

### linux
Requires gcc
```sh
make jsonfmt_linux
mv jsonfmt_linux jsonfmt
```

## usage
```sh
jsonfmt my_file.json
```

## future improvements
* Use non-blocking syscalls (kqueue / kevent on mac; io_uring on linux)
