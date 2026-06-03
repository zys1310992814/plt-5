#include "test_so.h"
#include "got_finder.h"

// 使用 got_find() 在运行时自动获取 GOT 表项地址，无需硬编码
int main(void)
{
    setbuf(stdout, NULL);  // 禁用缓冲，确保输出实时可见
    // 运行时自动查找 GOT 表项
    int *got_libc   = (int*)got_find("__libc_start_main");
    int *got_printf = (int*)got_find("printf");
    int *got_test_a = (int*)got_find("test_a");
    int *got_test_c = (int*)got_find("test_c");
    int *got_getpid = (int*)got_find("getpid");
    int *got_puts   = (int*)got_find("puts");
    int *got_test_b = (int*)got_find("test_b");

    printf("================================================\n");
    printf("    PLT/GOT 延迟绑定演示 (x86-32 位)\n");
    printf("================================================\n\n");

    // 打印 GOT 地址，验证自动查找结果
    printf("[GOT 表项地址]  (通过遍历 .dynamic 段自动获取)\n");
    printf("  __libc_start_main  -> %p\n", (void*)got_libc);
    printf("  printf             -> %p\n", (void*)got_printf);
    printf("  test_a             -> %p\n", (void*)got_test_a);
    printf("  test_c             -> %p\n", (void*)got_test_c);
    printf("  getpid             -> %p\n", (void*)got_getpid);
    printf("  puts               -> %p\n", (void*)got_puts);
    printf("  test_b             -> %p\n", (void*)got_test_b);
    printf("\n");

    // ====== 先记录调用前的 GOT 值 ======
    uint32_t before_libc   = *got_libc;
    uint32_t before_printf = *got_printf;
    uint32_t before_test_a = *got_test_a;
    uint32_t before_test_c = *got_test_c;
    uint32_t before_getpid = *got_getpid;
    uint32_t before_puts   = *got_puts;
    uint32_t before_test_b = *got_test_b;

    // ====== 实际调用，触发延迟绑定 ======
    test_a();
    test_b();
    test_c();

    // ====== 再记录调用后的 GOT 值 ======
    uint32_t after_libc   = *got_libc;
    uint32_t after_printf = *got_printf;
    uint32_t after_test_a = *got_test_a;
    uint32_t after_test_c = *got_test_c;
    uint32_t after_getpid = *got_getpid;
    uint32_t after_puts   = *got_puts;
    uint32_t after_test_b = *got_test_b;

    // ====== 打印对比表格，每行同时显示调用前和调用后的值 ======
    printf("+--------------------------+------------------+------------------+\n");
    printf("| 函数名                   | 调用前 GOT 值    | 调用后 GOT 值    |\n");
    printf("+--------------------------+------------------+------------------+\n");

#define PRINT_GOT_ROW(label, before_val, after_val) \
    printf("| %-24s | 0x%08x       | 0x%08x       |\n", \
           label, before_val, after_val)

    PRINT_GOT_ROW("__libc_start_main", before_libc,   after_libc);
    PRINT_GOT_ROW("printf",           before_printf, after_printf);
    PRINT_GOT_ROW("test_a",           before_test_a, after_test_a);
    PRINT_GOT_ROW("test_c",           before_test_c, after_test_c);
    PRINT_GOT_ROW("getpid",           before_getpid, after_getpid);
    PRINT_GOT_ROW("puts",             before_puts,   after_puts);
    PRINT_GOT_ROW("test_b",           before_test_b, after_test_b);
    printf("+--------------------------+------------------+------------------+\n\n");

    // 解析状态说明
    printf("[解析状态说明]\n");
#define CHECK(label, before_val, after_val) \
    do { \
        if ((before_val) != (after_val)) \
            printf("  %-20s  调用前 PLT 桩 → 调用后已解析为真实地址 ✅\n", label); \
        else if ((after_val) & 0xf0000000) \
            printf("  %-20s  调用前已由运行时预先解析 ✅\n", label); \
        else \
            printf("  %-20s  从未被调用，GOT 保持 PLT 桩地址 ⏭️\n", label); \
    } while(0)

    CHECK("__libc_start_main", before_libc,   after_libc);
    CHECK("printf",           before_printf, after_printf);
    CHECK("test_a",           before_test_a, after_test_a);
    CHECK("test_c",           before_test_c, after_test_c);
    CHECK("getpid",           before_getpid, after_getpid);
    CHECK("puts",             before_puts,   after_puts);
    CHECK("test_b",           before_test_b, after_test_b);

    printf("\n>>> 进程 PID=%d，正在挂起，Ctrl+C 退出...\n", getpid());
    while(1);
    return 0;
}
