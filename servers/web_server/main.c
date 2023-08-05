#include <libs/common/print.h>
#include <libs/user/ipc.h>
#include <libs/user/syscall.h>

static task_t tcpip_server;

// TCPコネクションを開く(パッシブオープン)
// 合っているかは知らん
static void init(void) {
    tcpip_server = ipc_lookup("tcpip");

    uint16_t port;
    // webサーバーなので80番固定
    port = 80;
    struct message m;
    m.type = TCPIP_CONNECT_PASSIVE_MSG;
    m.tcpip_connect_passive.src_port = port;
    ASSERT_OK(ipc_call(tcpip_server, &m));
    int sock = m.tcpip_connect_passive_reply.sock;

    INFO("fd: %d is created", sock);
}

void main(void) {
    // 初期化
    init();

    // pongサーバとして登録する
    ASSERT_OK(ipc_register("web_server"));
    TRACE("ready");

    // メインループ
    while (true) {
        // struct message m;
        // ASSERT_OK(ipc_recv(IPC_ANY, &m));
        // switch (m.type) {
        //     case PING_MSG: {
        //         DBG("received ping message from #%d (value=%d)", m.src,
        //             m.ping.value);

        //         m.type = PING_REPLY_MSG;
        //         m.ping_reply.value = 42;
        //         ipc_reply(m.src, &m);
        //         break;
        //     }
        //     default:
        //         WARN("unhandled message: %s (%x)", msgtype2str(m.type), m.type);
        //         break;
        // }
    }
}
