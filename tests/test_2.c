#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "ptentry.h"

#define PGSIZE 4096

static int 
err(char *msg, ...) {
    printf(1, "XV6_TEST_OUTPUT %s\n", msg);
    exit();
}

int main(void) {
    const uint PAGES_NUM = 1;
    char *buffer = sbrk(PGSIZE * sizeof(char));
    while ((uint)buffer != 0x6000)
        buffer = sbrk(PGSIZE * sizeof(char));
    // Allocate one pages of space
    char *ptr = sbrk(PAGES_NUM * PGSIZE);
    struct pt_entry pt_entries[PAGES_NUM];

    // Initialize the pages
    for (int i = 0; i < PAGES_NUM * PGSIZE; i++)
        ptr[i] = 0xAA;
    int retval = getpgtable(pt_entries, PAGES_NUM);
    if (retval == PAGES_NUM) {
        for (int i = 0; i < PAGES_NUM; i++) {
            printf(1, "XV6_TEST_OUTPUT Index %d: pdx: 0x%x, ptx: 0x%x, present bit: %d, writable bit: %d, encrypted: %d\n", 
                i,
                pt_entries[i].pdx,
                pt_entries[i].ptx,
                pt_entries[i].present,
                pt_entries[i].writable,
                pt_entries[i].encrypted
            );

            if (dump_rawphymem((uint)(pt_entries[i].ppage * PGSIZE), buffer) != 0)
                err("dump_rawphymem return non-zero value\n");
            
            for (int j = 0; j < PGSIZE; j++) {
                if (buffer[j] != (char)0xAA)
                    err("physical memory is dumped incorrectly\n");
            }
        }
    } else
        printf(1, "XV6_TEST_OUTPUT: getpgtable returned incorrect value: expected %d, got %d\n", PAGES_NUM, retval);
    
    exit();
}
