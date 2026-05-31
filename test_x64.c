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

    printf("+----------------------+------------------------+------------------------+\n");
    printf("| 函数名                 | 调用前 GOT 值               | 调用后 GOT 值               |\n");
    printf("+----------------------+------------------------+------------------------+\n");

#define PRINT_GOT_BEFORE64(label, ptr) \
    printf("| %-20s | %-22p | %-22p |\n", label, (void*)*(ptr), (void*)*(ptr))

    PRINT_GOT_BEFORE64("test_a",  got_test_a);
    PRINT_GOT_BEFORE64("puts",    got_puts);
    PRINT_GOT_BEFORE64("getpid",  got_getpid);
    PRINT_GOT_BEFORE64("printf",  got_printf);

    // ====== 实际调用，触发延迟绑定 ======
    test_a();

    printf("+----------------------+------------------------+------------------------+\n");

#define PRINT_GOT_AFTER64(label, ptr) \
    printf("| %-20s | %-22p | %-22p |\n", label, (void*)*(ptr), (void*)*(ptr))

    PRINT_GOT_AFTER64("test_a",  got_test_a);
    PRINT_GOT_AFTER64("puts",    got_puts);
    PRINT_GOT_AFTER64("getpid",  got_getpid);
    PRINT_GOT_AFTER64("printf",  got_printf);
    printf("+----------------------+------------------------+------------------------+\n\n");

    printf("[延迟绑定前] GOT 表项指向 PLT0 公共桩，等待 _dl_runtime_resolve 解析\n");
    printf("[延迟绑定后] GOT 表项被动态链接器更新为函数真实地址\n");
    printf("[注意] test_a 内部调用了 puts/printf，会间接触发这些函数的解析\n\n");

    printf(">>> 进程 PID=%d，正在挂起，Ctrl+C 退出...\n", getpid());
    while(1);
    return 0;
}
