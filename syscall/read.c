#pragma once
#include "../root.unity.h"

#ifdef __APPLE__
#define SYSCALL_READ 0x2000003
#endif
#ifdef __linux__
#define SYSCALL_READ 0x0
#endif

internal s64 read_(s64 fd, void *buf, u64 count) {
	return (s64)syscall5(INT2VOIDP(fd), (void *)buf, UINT2VOIDP(count), 0, 0, (void *)SYSCALL_READ);
}
