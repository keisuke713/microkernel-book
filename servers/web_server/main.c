#include <libs/common/print.h>
#include <libs/common/string.h>
#include <libs/user/ipc.h>
#include <libs/user/syscall.h>
#include <libs/user/malloc.h>

static task_t tcpip_server;
static char request[512];

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

// スペースに挟まれている文字列をファイルパスとする
// リクエストは下記のようになっているため
// GET /path/index.html HTTP1.1
static char* extract_path(uint8_t *buf) {
    int start;
    start = 0;
    while (*(buf + start++) != 32) {}
    INFO("message start: %d", *(buf + start));

    int end;
    end = start;
    while (*(buf + end++) != 32) {}
    INFO("message end: %d", *(buf + end));

    int len;
    len = end - start;
    char *path = malloc(len);
    memcpy(path, (void *)(buf + start), len);
    return path;
}

void main(void) {
    // 初期化
    init();

    // pongサーバとして登録する
    ASSERT_OK(ipc_register("web_server"));
    TRACE("ready");

    // メインループ
    while (true) {
        struct message m;
        ASSERT_OK(ipc_recv(IPC_ANY, &m));

        switch (m.type) {
            case TCPIP_DATA_MSG: {
                m.type = TCPIP_READ_MSG;
                m.tcpip_read.sock = m.tcpip_data.sock;
                error_t err = ipc_call(tcpip_server, &m);

                ASSERT_OK(err);

                // リクエストを受け取ったらパスを抽出
                char* path = extract_path((uint8_t *)m.tcpip_read_reply.data);
                INFO("after start: %s", path);

                // TODO ファイルシステムに該当のファイルを問い合わせて
                // それをレスポンスと共に返す
            }
        }
    }
}
