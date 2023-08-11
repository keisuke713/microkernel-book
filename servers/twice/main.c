#include <libs/common/print.h>
#include <libs/user/ipc.h>
#include <libs/user/syscall.h>

void main(void) {
    // twiceサーバとして登録する
    ASSERT_OK(ipc_register("twice"));
    TRACE("ready");

    // メインループ
    while (true) {
        struct message m;
        ASSERT_OK(ipc_recv(IPC_ANY, &m));
        switch (m.type) {
            case TWICE_MSG: {
                DBG("received ping message from #%d (value=%d)", m.src,
                    m.ping.value);

                m.type = TWICE_REPLY_MSG;
                m.ping_reply.value = m.ping.value * 2;
                ipc_reply(m.src, &m);
                break;
            }
            default:
                WARN("unhandled message: %s (%x)", msgtype2str(m.type), m.type);
                break;
        }
    }
}
