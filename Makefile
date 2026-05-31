#################################
# PLT/GOT 延迟绑定实验 Makefile
#################################

TARGET1 := test_x64
TARGET2 := test
CC      := gcc
LIBS    :=
LDFLAGS := -L.
DEFINES :=
INCLUDE := -I.
CFLAGS  := 
CXXFLAGS:=

.PHONY: all clean run run64 dump dump64 help

all: $(TARGET1) $(TARGET2)

# ========== 64 位目标 ==========
$(TARGET1):
	@echo ">>> 编译 64 位共享库 libtest_x64.so ..."
	$(CC) test_a.c test_b.c test_c.c --shared -fPIC -o libtest_x64.so
	@echo ">>> 编译 64 位可执行文件 test_x64 ..."
	$(CC) test_x64.c -L . -ltest_x64 -no-pie -g -o test_x64 -Wl,-t
	@echo ">>> 生成分析文件 ..."
	objdump -d -j .plt $(TARGET1) > objdump-plt_x64
	objdump -d -j .plt.sec $(TARGET1) > objdump-plt_sec_x64 2>/dev/null || true
	objdump -d -j .plt.got $(TARGET1) > objdump-plt_got_x64 2>/dev/null || true
	objdump -R $(TARGET1) > objdump-R_x64
	objdump -d $(TARGET1) > objdump-d_x64
	objdump -s $(TARGET1) > objdump-s_x64
	objdump -S $(TARGET1) > objdump-S_x64
	readelf -r $(TARGET1) > readelf-r_x64
	readelf -S $(TARGET1) > readelf-S_x64
	readelf -x .got $(TARGET1) > got_x64 2>/dev/null || true
	readelf -x .got.plt $(TARGET1) > got_plt_x64 2>/dev/null || true
	hexdump -C $(TARGET1) > hexdump-C_x64
	@echo ">>> test_x64 构建完成！"
	@echo "    运行: LD_LIBRARY_PATH=. ./test_x64"
	@echo "    查看 PLT: cat objdump-plt_x64"
	@echo "    查看 GOT: cat got_x64 got_plt_x64"

# ========== 32 位目标 ==========
$(TARGET2):
	@echo ">>> 编译 32 位共享库 libtest.so ..."
	$(CC) test_a.c test_b.c test_c.c -m32 -no-pie -g --shared -fPIC -o libtest.so
	@echo ">>> 编译 32 位可执行文件 test ..."
	$(CC) test.c -L . -ltest -m32 -no-pie -g -o test -Wl,-t
	@echo ">>> 生成分析文件 ..."
	objdump -d -j .plt $(TARGET2) > objdump-plt
	objdump -d -j .plt.sec $(TARGET2) > objdump-plt_sec 2>/dev/null || true
	objdump -d -j .plt.got $(TARGET2) > objdump-plt_got 2>/dev/null || true
	objdump -R $(TARGET2) > objdump-R
	objdump -d $(TARGET2) > objdump-d
	objdump -s $(TARGET2) > objdump-s
	objdump -S $(TARGET2) > objdump-S
	readelf -r $(TARGET2) > readelf-r
	readelf -S $(TARGET2) > readelf-S
	readelf -x .got $(TARGET2) > got 2>/dev/null || true
	readelf -x .got.plt $(TARGET2) > got_plt 2>/dev/null || true
	hexdump -C $(TARGET2) > hexdump-C
	@echo ">>> test 构建完成！"
	@echo "    运行: LD_LIBRARY_PATH=. ./test"
	@echo "    查看 PLT: cat objdump-plt"
	@echo "    查看 GOT: cat got got_plt"

# ========== 运行 ==========
run:
	@echo "=========================================="
	@echo "  运行 32 位 PLT/GOT 演示"
	@echo "=========================================="
	LD_LIBRARY_PATH=. ./test

run64:
	@echo "=========================================="
	@echo "  运行 64 位 PLT/GOT 演示"
	@echo "=========================================="
	LD_LIBRARY_PATH=. ./test_x64

# ========== 汇总展示 ==========
dump:
	@echo ""
	@echo "=========================================="
	@echo "  32 位 PLT/GOT 分析报告"
	@echo "=========================================="
	@echo ""
	@echo "--- [1] ELF 段表 (.plt, .got, .got.plt) ---"
	@readelf -S test | grep -E '\.plt|\.got'
	@echo ""
	@echo "--- [2] 动态重定位表 (R_386_JUMP_SLOT) ---"
	@objdump -R test | grep JUMP_SLOT
	@echo ""
	@echo "--- [3] PLT 反汇编 ---"
	@cat objdump-plt
	@echo ""
	@echo "--- [4] GOT 十六进制内容 ---"
	@cat got_plt 2>/dev/null || cat got

dump64:
	@echo ""
	@echo "=========================================="
	@echo "  64 位 PLT/GOT 分析报告"
	@echo "=========================================="
	@echo ""
	@echo "--- [1] ELF 段表 (.plt, .got, .got.plt) ---"
	@readelf -S test_x64 | grep -E '\.plt|\.got'
	@echo ""
	@echo "--- [2] 动态重定位表 (R_X86_64_JUMP_SLOT) ---"
	@objdump -R test_x64 | grep JUMP_SLOT
	@echo ""
	@echo "--- [3] PLT 反汇编 ---"
	@cat objdump-plt_x64
	@echo ""
	@echo "--- [4] GOT 十六进制内容 ---"
	@cat got_plt_x64 2>/dev/null || cat got_x64

# ========== 清理 ==========
clean:
	rm -fr *.o *.so
	rm -fr $(TARGET1) hexdump-C_x64 objdump-*_x64 readelf-*_x64 got*_x64
	rm -fr $(TARGET2) hexdump-C objdump-* readelf-* got

help:
	@echo "=============================="
	@echo " PLT/GOT 实验 Makefile 帮助"
	@echo "=============================="
	@echo ""
	@echo " make          - 编译 32 位和 64 位所有目标"
	@echo " make test     - 只编译 32 位版本"
	@echo " make test_x64 - 只编译 64 位版本"
	@echo " make run      - 运行 32 位演示"
	@echo " make run64    - 运行 64 位演示"
	@echo " make dump     - 查看 32 位 PLT/GOT 分析报告"
	@echo " make dump64   - 查看 64 位 PLT/GOT 分析报告"
	@echo " make clean    - 清理所有生成文件"
	@echo ""
	@echo " 注意: 运行前请确保已安装 32 位库 (gcc-multilib / libc6-dev-i386)"
