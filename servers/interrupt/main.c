#include <libs/common/print.h>
#include <libs/user/syscall.h>

void main(void) {
    INFO("interrupted %d times", sys_number_of_interrupt());
}
