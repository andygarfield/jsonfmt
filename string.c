#pragma once
#include "root.unity.h"
#include <stddef.h>

// Max 1GB string
#define MAX_STRING_SIZE (1 << 30)

// static u8 stdoutBuffer[4096] = {0};
// static u8 stdoutBufferPosition[4096] = {0};

typedef struct {
	u8 *buffer;
	u64 len;
} String;

// RefString is a reference to a string in a buffer. The `start` field is the
// first index of the string, and `len` is the string's length.
typedef struct {
	u64 start;
	u64 len;
} RefString;

typedef struct {
	RefString *p;
	u64 len;
} RefStringArray;

// void appendRefString(RefStringArray *a, RefString item) {
//	a->p[a->len] = item;
//	a->len++;
// }

///
/// copies an array
///
void memcpy_(void *dest, const void *src, u64 n) {
	char *d = dest;
	const char *s = src;
	for (u64 i = 0; i < n; i++) {
		d[i] = s[i];
	}
}

static u16 writeBufferPos = 0;
static char writeBuffer[2 << 13] = {0};

#ifdef DEBUG_W_STDLIB
#include <stdio.h>
internal void printFlush() {}
internal void printRefString(String s, RefString r) { printf("%*.*s", (int)r.len, (int)r.len, s.buffer + r.start); }
internal void printChar(char *s) { printf("%s", s); }
#else
internal u64 strlen(u8 const *str) {
	u64 i;
	for (i = 0; i < MAX_STRING_SIZE; ++i) {
		if (str[i] == 0) {
			break;
		}
	}
	return i;
}

internal String newString(char *s) {
	u8 *buffer = (u8 *)s;
	String str = {.buffer = buffer, .len = strlen(buffer)};
	return str;
}

internal void print(String s) {
	u64 remaining = s.len;
	while (remaining) {
		u64 writeAmount = sizeof(writeBuffer) - writeBufferPos;
		if (writeAmount > remaining)
			writeAmount = remaining;

		memcpy_(writeBuffer + writeBufferPos, s.buffer + (s.len - remaining), writeAmount);

		writeBufferPos += writeAmount;
		if (writeBufferPos == sizeof(writeBuffer)) {
			write_(1, (void *)writeBuffer, sizeof(writeBuffer));
			writeBufferPos = 0;
		}
		remaining -= writeAmount;
	}
}
internal void printFlush() { write_(1, (void *)writeBuffer, writeBufferPos); }
internal void printRefString(String s, RefString r) { print((String){.buffer = s.buffer + r.start, .len = r.len}); }
internal void printChar(char *s) { print(newString(s)); }
#endif

#ifdef DEBUG_W_STDLIB
#else
#endif
