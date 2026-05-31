#ifndef _GOT_FINDER_H
#define _GOT_FINDER_H

#include <elf.h>
#include <link.h>
#include <string.h>

/* ----------------------------------------------------------------
 * got_finder — 运行时自动遍历 .dynamic 段，获取 JUMP_SLOT 重定位
 *             表中每个符号的 GOT 表项地址，无需硬编码。
 *
 * 原理：
 *   - _DYNAMIC 是链接器提供的符号，指向 .dynamic 段起始
 *   - 遍历 DT_JMPREL / DT_SYMTAB / DT_STRTAB / DT_PLTRELSZ 等 tag
 *   - 64 位用 Elf64_Rela，32 位用 Elf32_Rel（无 addend）
 *   - 每个重定位记录了一个 GOT 地址（r_offset）+ 符号索引（r_info）
 * ---------------------------------------------------------------- */

// 根据符号名查找 GOT 表项地址
static inline void *got_find(const char *name)
{
    if (!name) return NULL;

    // 声明 _DYNAMIC 符号（链接器自动提供）
    extern ElfW(Dyn) _DYNAMIC[] __attribute__((weak));
    ElfW(Dyn) *dyn = (ElfW(Dyn) *)_DYNAMIC;
    if (!dyn) return NULL;

    // 遍历 .dynamic 获取关键段地址
    void       *jmprel   = NULL;   // .rel(a).plt 起始
    ElfW(Sym)  *symtab   = NULL;   // .dynsym
    const char *strtab   = NULL;   // .dynstr
    size_t      pltrelsz = 0;      // .rel(a).plt 大小
    int         is_rela  = 0;      // 64 位用 RELA，32 位用 REL

    for (ElfW(Dyn) *d = dyn; d->d_tag != DT_NULL; d++) {
        switch (d->d_tag) {
        case DT_JMPREL:  jmprel   = (void*)(uintptr_t)d->d_un.d_ptr; break;
        case DT_SYMTAB:  symtab   = (ElfW(Sym)*)(uintptr_t)d->d_un.d_ptr; break;
        case DT_STRTAB:  strtab   = (const char*)(uintptr_t)d->d_un.d_ptr; break;
        case DT_PLTRELSZ: pltrelsz = d->d_un.d_val;                         break;
        case DT_PLTREL:
            // DT_PLTREL 指示用 REL 还是 RELA
            is_rela = (d->d_un.d_val == DT_RELA);
            break;
        }
    }

    if (!jmprel || !symtab || !strtab || !pltrelsz) return NULL;

    // 64 位用 RELA，32 位用 REL
    if (is_rela) {
        // RELA 格式（64 位）
        ElfW(Rela) *rela = (ElfW(Rela)*)jmprel;
        size_t count = pltrelsz / sizeof(ElfW(Rela));
        for (size_t i = 0; i < count; i++) {
            uint32_t sym_idx = ELF64_R_SYM(rela[i].r_info);
            const char *sym_name = strtab + symtab[sym_idx].st_name;
            if (strcmp(sym_name, name) == 0)
                return (void *)(uintptr_t)rela[i].r_offset;
        }
    } else {
        // REL 格式（32 位，无 r_addend）
        ElfW(Rel) *rel = (ElfW(Rel)*)jmprel;
        size_t count = pltrelsz / sizeof(ElfW(Rel));
        for (size_t i = 0; i < count; i++) {
            uint32_t sym_idx = ELF32_R_SYM(rel[i].r_info);
            const char *sym_name = strtab + symtab[sym_idx].st_name;
            if (strcmp(sym_name, name) == 0)
                return (void *)(uintptr_t)rel[i].r_offset;
        }
    }

    return NULL;
}

#endif /* _GOT_FINDER_H */
