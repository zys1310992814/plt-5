#include "test_so.h"
#include "got_finder.h"

// 使用 got_find() 在运行时自动获取 GOT 表项地址，无需硬编码
int main(void)
{
    setbuf(stdout, NULL);  // 禁用缓冲，确保输出实时可见
    // 运行时自动查找 GOT 表项
    long *got_test_a  = (long*)got_find("test_a");
    long *got_puts    = (long*)got_find("puts");
    long *got_getpid  = (long*)got_find("getpid");
    long *got_printf  = (long*)got_find("printf");

    printf("================================================\n");
    printf("    PLT/GOT 延迟绑定演示 (x86-64 位)\n");
    printf("================================================\n\n");

    // 打印 GOT 地址，验证自动查找结果
    printf("[GOT 表项地址]  (通过遍历 .dynamic 段自动获取)\n");
    printf("  test_a   -> %p\n", (void*)got_test_a);
    printf("  puts     -> %p\n", (void*)got_puts);
    printf("  getpid   -> %p\n", (void*)got_getpid);
    printf("  printf   -> %p\n", (void*)got_printf);
    printf("\n");

    // ====== 先记录调用前的 GOT 值 ======
    uint64_t before_test_a = *got_test_a;
    uint64_t before_puts   = *got_puts;
    uint64_t before_getpid = *got_getpid;
    uint64_t before_printf = *got_printf;

    // ====== 实际调用，触发延迟绑定 ======
    test_a();

    // ====== 再记录调用后的 GOT 值 ======
    uint64_t after_test_a = *got_test_a;
    uint64_t after_puts   = *got_puts;
    uint64_t after_getpid = *got_getpid;
    uint64_t after_printf = *got_printf;

    // ====== 打印对比表格，每行同时显示调用前和调用后的值 ======
    printf("+----------------------+------------------------+------------------------+\n");
    printf("| 函数名                 | 调用前 GOT 值               | 调用后 GOT 值               |\n");
    printf("+----------------------+------------------------+------------------------+\n");

#define PRINT_GOT_ROW64(label, before_val, after_val) \
    printf("| %-20s | 0x%014lx          | 0x%014lx          |\n", \
           label, before_val, after_val)

    PRINT_GOT_ROW64("test_a",  before_test_a, after_test_a);
    PRINT_GOT_ROW64("puts",    before_puts,   after_puts);
    PRINT_GOT_ROW64("getpid",  before_getpid, after_getpid);
    PRINT_GOT_ROW64("printf",  before_printf, after_printf);
    printf("+----------------------+------------------------+------------------------+\n\n");

    // 解析状态说明
    printf("[解析状态说明]\n");
#define CHECK64(label, before_val, after_val) \
    do { \
        if ((before_val) != (after_val)) \
            printf("  %-20s  调用前 PLT 桩 → 调用后已解析为真实地址 ✅\n", label); \
        else if ((after_val) & 0x7f0000000000) \
            printf("  %-20s  调用前已由运行时预先解析 ✅\n", label); \
        else \
            printf("  %-20s  从未被调用，GOT 保持 PLT 桩地址 ⏭️\n", label); \
    } while(0)

    CHECK64("test_a",  before_test_a, after_test_a);
    CHECK64("puts",    before_puts,   after_puts);
    CHECK64("getpid",  before_getpid, after_getpid);
    CHECK64("printf",  before_printf, after_printf);

    printf("\n>>> 进程 PID=%d，正在挂起，Ctrl+C 退出...\n", getpid());
    while(1);
    return 0;
}
