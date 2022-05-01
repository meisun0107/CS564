#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "ptentry.h"

#define PGSIZE 4096


int main(void) {
    const uint PAGES_NUM = 1;
    
    // Allocate one pages of space
    char *ptr = sbrk(PAGES_NUM * PGSIZE);
    printf(1, "XV6_TEST_OUTPUT %d\n", mencrypt(ptr, PAGES_NUM));
    
    exit();
}
