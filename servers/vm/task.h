#pragma once
#include <libs/common/elf.h>
#include <libs/common/list.h>
#include <libs/common/types.h>

// サービス名の最大長
#define SERVICE_NAME_LEN 64
// 動的に割り当てられる仮想アドレスの開始アドレス
#define VALLOC_BASE 0x20000000
// 動的に割り当てられる仮想アドレスの終了アドレス
#define VALLOC_END 0x40000000

// サービス管理構造体。サービス名とタスクIDの対応を保持し、サービスディスカバリに使われる。
struct service {
    list_elem_t next;
    char name[SERVICE_NAME_LEN];  // サービス名
    task_t task;                  // タスクID
};

// 物理ページと仮想アドレスのマッピング
struct phys_virt_mapping {
    paddr_t paddr;
    uaddr_t uaddr;
};

#define NUM_PHYS_VIRT_MAPPING_MAX 50

// タスク管理構造体
struct bootfs_file;
struct task {
    task_t tid;                          // タスクID
    task_t pager;                        // ページャタスクID
    char name[TASK_NAME_LEN];            // タスク名
    void *file_header;                   // ELFファイルの先頭を指す
    struct bootfs_file *file;            // BootFS上のELFファイル
    elf_ehdr_t *ehdr;                    // ELFヘッダ
    elf_phdr_t *phdrs;                   // プログラムヘッダ
    uaddr_t valloc_next;                 // 動的に割り当てられる仮想アドレスの次のアドレス
    char waiting_for[SERVICE_NAME_LEN];  // サービス登録待ちのサービス名
    bool watch_tasks;                    // タスクの終了を監視するかどうか
    struct phys_virt_mapping phys_virt_mapping_list[NUM_PHYS_VIRT_MAPPING_MAX];
    int index; // phys_virt_mapping_listのindex
};

struct task *task_find(task_t tid);
void task_fork(task_t parnet_id, task_t child_id);
task_t task_spawn(struct bootfs_file *file);
void task_destroy(struct task *task);
error_t task_destroy_by_tid(task_t tid);
void service_register(struct task *task, const char *name);
task_t service_lookup_or_wait(struct task *task, const char *name);
void service_dump(void);
