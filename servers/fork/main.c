#include <libs/common/print.h>
#include <libs/user/fork.h>

void main(void) {
    int p = fork();
    if (p < 0) {
        PANIC("fork fail: %d", p);
    } else if (p == 0) {
        INFO("Hello from child");
    } else {
        INFO("Hello form parent");
    }
    INFO("bye");
}
