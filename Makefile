.POSIX:
CLANG_FLAGS = -std=c99 -O3 -Wall -Wextra -Werror -nostdlib -lSystem -fdelete-null-pointer-checks
CLANG_DB_FLAGS = -std=c99 -O0 -Wall Wextra -Werror -g

GCC_FLAGS = -s -O3 -fdata-sections -Wl,-gc-sections -Wl,--gc-sections \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-builtin -std=c99 \
	-Wall -Werror -Wa,--noexecstack -fno-stack-protector \
	-fdelete-null-pointer-checks -static
GCC_DB_FLAGS = -O0 -ggdb -z noexecstack

C_SRC=$(wildcard **.c)
H_SRC=$(wildcard **.h)

jsonfmt_mac: start_mac.S $(C_SRC) $(H_SRC)
	clang $(CLANG_FLAGS) -o $@ start_mac.S root.c
jsonfmt_mac_db: start_mac.S $(C_SRC) $(H_SRC)
	clang $(CLANG_DB_FLAGS) -o $@ start_mac.S root.c

jsonfmt_linux: start_linux.S $(C_SRC) $(H_SRC)
	gcc $(GCC_FLAGS) -o $@ start_linux.S root.c
jsonfmt_linux_db: start_linux.S $(C_SRC) $(H_SRC)
	gcc $(GCC_DB_FLAGS) -o $@ start_linux.S root.c
