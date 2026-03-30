# jsonfmt
A Simple JSON formatter written in C99, without the C standard library. Not
crazy fast, but roughly 2-10 times faster than [jq](https://jqlang.org/),
depending on what you're doing.

## building
Currently things only work on x86 because we're rolling our own `start`
function, and I haven't written the assembly for other platforms yet.

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

### windows
Not supported yet.

## usage
```sh
jsonfmt my_file.json
```

## future improvements
- [x] JSON streams (instead of just a single value)
- [x] Parity with JQ formatting
- [ ] Support for arm64
- [ ] Windows support
- [ ] Do proper number parsing
- [ ] Minifying
- [ ] Configuration
- [ ] Use non-blocking syscalls (kqueue / kevent on mac; io_uring on linux)
