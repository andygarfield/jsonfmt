#pragma once
#include "root.unity.h"

// TODO: find the page size per system at runtime startup
#define PAGE_SIZE 4096

internal void *allocPages(u64 pages) {
	return mmap(0, PAGE_SIZE * pages, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}
