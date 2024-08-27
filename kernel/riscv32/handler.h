#pragma once
#include "trap.h"

void riscv32_trap_handler(void);
void riscv32_timer_handler(void);
void riscv32_fork_trap_handler(struct riscv32_trap_frame *frame);
