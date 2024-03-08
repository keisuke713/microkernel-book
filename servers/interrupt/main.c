#include <libs/common/print.h>
#include <libs/user/ipc.h>
#include <libs/user/syscall.h>

void main(void) {
    // interruptサーバとして登録する
    ASSERT_OK(ipc_register("interrupt"));
    TRACE("ready");

    // main-loop
    while (true) {
        struct message m;
        ASSERT_OK(ipc_recv(IPC_ANY, &m));
        switch (m.type) {
            case INTERRUPT_MSG: {
                m.type = INTERRUPT_REPLY_MSG;
                m.ping_reply.value = sys_number_of_interrupt();
                ipc_reply(m.src, &m);
                break;
            }
            default:
                WARN("unhandled message: %s (%x)", msgtype2str(m.type), m.type);
                break;
        }
    }
}
