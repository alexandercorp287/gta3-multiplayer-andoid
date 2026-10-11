#include "../main.h"
#include "arm.h"


void PatchMemory(void* addr, const void* data, size_t size)
{
    size_t pagesize = sysconf(_SC_PAGE_SIZE);

    uintptr_t start = (uintptr_t)addr & ~(pagesize - 1);
    uintptr_t end = ((uintptr_t)addr + size + pagesize - 1) & ~(pagesize - 1);

    size_t len = end - start;

    mprotect((void*)start, len, PROT_READ | PROT_WRITE | PROT_EXEC);

    memcpy(addr, data, size);

    __builtin___clear_cache((char*)addr, (char*)addr + size);
    mprotect((void*)start, len, PROT_READ | PROT_EXEC);
}

void UnFuck(void* addr, size_t size)
{
    size_t pagesize = sysconf(_SC_PAGE_SIZE);

    uintptr_t start = (uintptr_t)addr & ~(pagesize - 1);
    uintptr_t end = ((uintptr_t)addr + size + pagesize - 1) & ~(pagesize - 1);

    size_t len = end - start;

    mprotect((void*)start, len, PROT_READ | PROT_WRITE | PROT_EXEC);
}

void NoOperation(void* addr)
{
    uint16_t nop_instruction[] = { 0x4770, 0xBF00 };
    PatchMemory(addr, nop_instruction, sizeof(nop_instruction));
}