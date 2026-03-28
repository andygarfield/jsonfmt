#pragma once
#include "../root.unity.h"

#ifdef __APPLE__
#define SYSCALL_WRITE 0x2000004
#endif
#ifdef __linux__
#define SYSCALL_WRITE 0x1
#endif

internal s64 write_(int fd, const void *data, u64 nbytes) {
	return (s64)syscall5(INT2VOIDP(fd), (const void *)data, UINT2VOIDP(nbytes), 0, 0, (void *)SYSCALL_WRITE);
}
