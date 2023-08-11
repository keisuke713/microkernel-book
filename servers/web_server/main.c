#include <libs/common/print.h>
#include <libs/common/string.h>
#include <libs/user/ipc.h>
#include <libs/user/syscall.h>
#include <libs/user/malloc.h>

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

// スペースに挟まれている文字列をファイルパスとする
// リクエストは下記のようになっているため
// GET /path/index.html HTTP1.1
static char* extract_path(uint8_t *buf) {
    int start;
    start = 0;
    while (*(buf + start++) != 32) {}
    start++;

    int end;
    end = start;
    while (*(buf + end++) != 32) {}

    // 末尾のスペースを無視するため
    int len;
    len = end - (start + 1);
    char *path = malloc(len);
    memcpy(path, (void *)(buf + start), len);
    return path;
}

uint8_t* fs_read(const char *path) {
    task_t fs_server = ipc_lookup("fs");
    ASSERT_OK(fs_server);

    struct message m;
    m.type = FS_OPEN_MSG;
    strcpy_safe(m.fs_open.path, sizeof(m.fs_open.path), path);
    error_t err = ipc_call(fs_server, &m);
    if (IS_ERROR(err)) {
        WARN("failed to open a file: '%s' (%s)", path, err2str(err));
        return NULL;
    }

    ASSERT(m.type == FS_OPEN_REPLY_MSG);
    int fd = m.fs_open_reply.fd;

    while (true) {
        m.type = FS_READ_MSG;
        m.fs_read.fd = fd;
        m.fs_read.len = PAGE_SIZE;
        error_t err = err = ipc_call(fs_server, &m);
        if (err == ERR_EOF) {
            break;
        }

        if (IS_ERROR(err)) {
            WARN("failed to read a file: %s", err2str(err));
            return NULL;
        }

        ASSERT(m.type == FS_READ_REPLY_MSG);
        unsigned end =
            MIN(sizeof(m.fs_read_reply.data) - 1, m.fs_read_reply.data_len);
        m.fs_read_reply.data[end] = '\0';
        return (uint8_t *) m.fs_read_reply.data;
    }
    return NULL;
}

static void send(int sock, const uint8_t *buf, size_t len) {
    struct message m;
    ASSERT(len < sizeof(m.tcpip_write.data));

    m.type = TCPIP_WRITE_MSG;
    m.tcpip_write.sock = sock;
    memcpy(m.tcpip_write.data, buf, len);
    m.tcpip_write.data_len = len;
    ASSERT_OK(ipc_call(tcpip_server, &m));
}

static void active_close(int sock) {
    struct message m;
    
    m.type = TCPIP_CLOSE_ACTIVE_MSG;
    m.tcpip_close_active.sock = sock;
    ASSERT_OK(ipc_call(tcpip_server, &m));
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
                int sock = m.tcpip_data.sock;
                error_t err = ipc_call(tcpip_server, &m);

                ASSERT_OK(err);

                // リクエストを受け取ったらパスを抽出
                char* path = extract_path((uint8_t *)m.tcpip_read_reply.data);

                // TODO not foundの処理
                uint8_t* content = fs_read(path);

                // レスポンスヘッダーの構築
                int buf_len = 1024;
                char *buf = malloc(buf_len);

                char *p = buf;
                // レスポンス
                for (const char *s = "HTTP/1.1 200 OK\r\n"; *s; s++) {
                    *p++ = *s;
                }
                // ヘッダー
                for (const char *s = "Content-Type: text/plain\r\n\r\n"; *s; s++) {
                    *p++ = *s;
                }
                // bodyの構築
                for (const char *s = content; *s; s++) {
                    *p++ = *s;
                }

                send(sock, (uint8_t *)buf, strlen(buf));

                active_close(sock);
            }
        }
    }
}
