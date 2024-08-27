#include <libs/user/syscall.h>

int fork() {
    return sys_fork();
}