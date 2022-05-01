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

int 
main(void){
    const uint PAGES_NUM = 5;
    char *buffer = sbrk(PGSIZE * sizeof(char));
    while ((uint)buffer != 0x6000)
        buffer = sbrk(PGSIZE * sizeof(char));
    // Allocate NUM pages of space
    char *ptr = sbrk(PAGES_NUM * PGSIZE);
    struct pt_entry pt_entries[PAGES_NUM];
    // Initialize the pages
    for (int i = 0; i < PAGES_NUM * PGSIZE; i++)
        ptr[i] = 0xAA;
    

    if (mencrypt(ptr, PAGES_NUM) != 0)
        err("mencrypt return non-zero value when mencrypt is called on valid range\n");

    if (getpgtable(pt_entries, PAGES_NUM) >= 0) {
        for (int i = 0; i < PAGES_NUM; i++) {
            printf(1, "XV6_TEST_OUTPUT Index %d: pdx: 0x%x, ptx: 0x%x, present: %d, writable: %d, encrypted: %d\n", 
                i,
                pt_entries[i].pdx,
                pt_entries[i].ptx,
                //pt_entries[i].ppage,
                pt_entries[i].present,
                pt_entries[i].writable,
                pt_entries[i].encrypted
            );

            if (dump_rawphymem((uint)(pt_entries[i].ppage * PGSIZE), buffer) != 0)
                err("dump_rawphymem return non-zero value\n");
            
            uint expected = ~0xAA;
            uint is_failed = 0;
            for (int j = 0; j < PGSIZE; j ++) {
                if (buffer[j] != (char)expected) {
                    is_failed = 1;
                    break;
                }
            }
            if (is_failed) {
                printf(1, "XV6_TEST_OUTPUT wrong content at physical page 0x%x\n", pt_entries[i].ppage * PGSIZE);
                for (int j = 0; j < PGSIZE; j +=64) {
                    printf(1, "XV6_TEST_OUTPUT ");
                    for (int k = 0; k < 64; k ++) {
                        if (k < 63) {
                            printf(1, "0x%x ", (uint)buffer[j + k] & 0xFF);
                        } else {
                            printf(1, "0x%x\n", (uint)buffer[j + k] & 0xFF);
                        }
                    }
                }
                err("physical memory is encrypted incorrectly\n");
            }
        }
    }

    for (int i = 0; i < PAGES_NUM; i++) 
    {
        ptr[(i + 1) * PGSIZE - 1] = 0xAA;
    }

    if (getpgtable(pt_entries, PAGES_NUM) >= 0) {
        for (int i = 0; i < PAGES_NUM; i++) {
            printf(1, "XV6_TEST_OUTPUT Index %d: pdx: 0x%x, ptx: 0x%x, present: %d, writable: %d, encrypted: %d\n", 
                i,
                pt_entries[i].pdx,
                pt_entries[i].ptx,
                //pt_entries[i].ppage,
                pt_entries[i].present,
                pt_entries[i].writable,
                pt_entries[i].encrypted
            );

            if (dump_rawphymem((uint)(pt_entries[i].ppage * PGSIZE), buffer) != 0)
                err("dump_rawphymem return non-zero value\n");
            
            uint expected = 0xAA;
            uint is_failed = 0;
            for (int j = 0; j < PGSIZE; j ++) {
                if (buffer[j] != (char)expected) {
                    is_failed = 1;
                    break;
                }
            }
            if (is_failed) {
                printf(1, "XV6_TEST_OUTPUT wrong content at physical page 0x%x\n", pt_entries[i].ppage * PGSIZE);
                for (int j = 0; j < PGSIZE; j +=64) {
                    printf(1, "XV6_TEST_OUTPUT ");
                    for (int k = 0; k < 64; k ++) {
                        if (k < 63) {
                            printf(1, "0x%x ", (uint)buffer[j + k] & 0xFF);
                        } else {
                            printf(1, "0x%x\n", (uint)buffer[j + k] & 0xFF);
                        }
                    }
                }
                err("physical memory is decrypted incorrectly\n");
            }
        }
    }
    exit();
}
