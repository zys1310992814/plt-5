# PLT/GOT 延迟绑定演示

通过实验直观展示 ELF 动态链接中 **PLT（Procedure Linkage Table）** 和 **GOT（Global Offset Table）** 的延迟绑定（Lazy Binding）机制。

同时提供 **x86-32** 和 **x86-64** 两种架构的对比。

## 背景知识

```
调用流程:
  call func@plt
    └→ PLT 桩: jmp *GOT[n]
         ├─ 首次调用: GOT[n] 指向 PLT 桩下一条指令
         │   └→ push reloc_index
         │       └→ jmp PLT0
         │           └→ _dl_runtime_resolve(link_map, reloc_index)
         │               └→ 查找符号 → 写入 GOT[n] → 跳转执行
         └─ 后续调用: GOT[n] 已是函数真实地址 → 直接跳转
```

## 项目结构

```
plt-5/
├── test_so.h          # 公共头文件（函数声明）
├── test_a.c           # 共享库函数 a
├── test_b.c           # 共享库函数 b
├── test_c.c           # 共享库函数 c
├── test.c             # 32 位主程序（PLT/GOT 演示）
├── test_x64.c         # 64 位主程序（PLT/GOT 演示）
├── Makefile           # 构建、运行、分析一体化
└── README.md
```

## 快速开始

```bash
# 编译所有目标
make

# 运行演示（观察 GOT 表变化）
make run        # 32 位
make run64      # 64 位

# 查看 PLT/GOT 分析报告
make dump       # 32 位
make dump64     # 64 位

# 清理
make clean
```

## 实验要点

1. **`-no-pie`** — 禁用 PIE，确保 GOT 表地址固定，便于硬编码地址读取
2. **`--shared -fPIC`** — 编译位置无关的共享库
3. **延迟绑定** — 函数首次调用时才解析真实地址，后续调用直接跳转
4. **GOT[0]~GOT[2]** — 保留给动态链接器使用（.dynamic / link_map / _dl_runtime_resolve）
5. **32 位 vs 64 位** — PLT 入口使用 `jmp *GOT[n]` 的寻址方式不同

## 输出示例

```
================================================
    PLT/GOT 延迟绑定演示 (x86-64 位)
================================================

+----------------------+------------------------+------------------------+
| 函数名                 | 调用前 GOT 值               | 调用后 GOT 值               |
+----------------------+------------------------+------------------------+
| test_a               | 0x401030               | 0x401030               |
| printf               | 0x401036               | 0x401036               |
+----------------------+------------------------+------------------------+
| test_a               | 0x401030               | 0x7f1234567890         |
| printf               | 0x401036               | 0x7f1234561234         |
+----------------------+------------------------+------------------------+

[延迟绑定前] GOT 表项指向 PLT0 公共桩，等待 _dl_runtime_resolve 解析
[延迟绑定后] GOT 表项被动态链接器更新为函数真实地址
```

## 手动分析命令

```bash
# 查看 PLT 反汇编
objdump -d -j .plt test

# 查看动态重定位表（确定 GOT 表项和函数对应关系）
objdump -R test

# 查看 GOT 段十六进制内容
readelf -x .got.plt test

# 运行时查看内存映射
cat /proc/$(pidof test)/maps
```

## 依赖

- GCC（支持 `-m32` 需安装 `gcc-multilib` 和 `libc6-dev-i386`）
- binutils（objdump、readelf）
