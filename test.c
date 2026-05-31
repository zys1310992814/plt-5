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

    printf("+--------------------------+------------------+------------------+\n");
    printf("| 函数名                     | 调用前 GOT 值        | 调用后 GOT 值        |\n");
    printf("+--------------------------+------------------+------------------+\n");

#define PRINT_GOT_BEFORE(label, ptr) \
    printf("| %-24s | %p           | %p           |\n", \
           label, (void*)(uintptr_t)*(ptr), (void*)(uintptr_t)*(ptr))

    PRINT_GOT_BEFORE("__libc_start_main", got_libc);
    PRINT_GOT_BEFORE("printf",           got_printf);
    PRINT_GOT_BEFORE("test_a",           got_test_a);
    PRINT_GOT_BEFORE("test_c",           got_test_c);
    PRINT_GOT_BEFORE("getpid",           got_getpid);
    PRINT_GOT_BEFORE("puts",             got_puts);
    PRINT_GOT_BEFORE("test_b",           got_test_b);

    // ====== 实际调用，触发延迟绑定 ======
    test_a();
    test_b();
    test_c();

    printf("+--------------------------+------------------+------------------+\n");

#define PRINT_GOT_AFTER(label, ptr) \
    printf("| %-24s | %p           | %p           |\n", \
           label, (void*)(uintptr_t)*(ptr), (void*)(uintptr_t)*(ptr))

    PRINT_GOT_AFTER("__libc_start_main", got_libc);
    PRINT_GOT_AFTER("printf",           got_printf);
    PRINT_GOT_AFTER("test_a",           got_test_a);
    PRINT_GOT_AFTER("test_c",           got_test_c);
    PRINT_GOT_AFTER("getpid",           got_getpid);
    PRINT_GOT_AFTER("puts",             got_puts);
    PRINT_GOT_AFTER("test_b",           got_test_b);
    printf("+--------------------------+------------------+------------------+\n\n");

    printf("[延迟绑定前] GOT 表项指向 PLT 桩的下一条指令（push + jmp PLT0）\n");
    printf("[延迟绑定后] GOT 表项被 _dl_runtime_resolve 更新为函数真实地址\n");
    printf("[注意] test_a/c 内部调用了 printf/puts，会间接触发这些函数的解析\n\n");

    printf(">>> 进程 PID=%d，正在挂起，Ctrl+C 退出...\n", getpid());
    while(1);
    return 0;
}
