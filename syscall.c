#pragma once
#include "root.unity.h"

s64 syscall5(const void *arg1, const void *arg2, const void *arg3, const void *arg4, const void *arg5,
	     const void *number);

void *syscall6(const void *arg1, const void *arg2, const void *arg3, const void *arg4, const void *arg5,
	       const void *arg6, const void *number);
