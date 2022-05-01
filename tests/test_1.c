#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "ptentry.h"

#define PGSIZE 4096

/*static int 
err(char *msg, ...) {
    printf(2, "XV6_TEST_OUTPUT: %s\n", msg);
    exit();
}*/

int main(void) {
    const uint PAGES_NUM = 10;
    
    // Allocate one pages of space
    sbrk(PAGES_NUM * PGSIZE);
    struct pt_entry pt_entries[PAGES_NUM];
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
        }
    } else
        printf(1, "XV6_TEST_OUTPUT: getpgtable returned incorrect value: expected %d, got %d\n", PAGES_NUM, retval);
    exit();
}
