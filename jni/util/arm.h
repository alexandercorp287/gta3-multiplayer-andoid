#pragma once

#include <sys/mman.h>

void PatchMemory(void* addr, const void* data, size_t size);
void UnFuck(void* addr, size_t size);
void NoOperation(void* addr);