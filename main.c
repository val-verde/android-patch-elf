// Android ELF patch utility
//
// This utility patch Android ELF binaries to conform to the following specification:
// 1. Favor use of DT_RUNPATH over DT_RPATH
// 2. RUNPATH is set to $ORIGIN
// 3. DT_NEEDED specified dependent libraries are renamed to drop the version suffix. i.e. libunwind.so.1 -> libunwind.so.
// 4. DT_SONAME if specified, is renamed to drop the version suffix.

#include <elf.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
 
int32_t
main(int32_t argc,
     char **argv)
{
    if (argc < 2){
        return 0;
    }

    for (int32_t fileIndex = 1; fileIndex < argc; ++fileIndex) {
        int fd = open(argv[fileIndex], O_RDWR);
        struct stat file_stat;

        if ((fd == -1) ||
            (fstat(fd, &file_stat) == -1)) {
            return 1;
        }

        uint8_t *mem = mmap(NULL,
                            file_stat.st_size,
                            PROT_READ |
                            PROT_WRITE,
                            MAP_SHARED,
                            fd,
                            0);
        Elf64_Ehdr * const elf_header = (Elf64_Ehdr *)mem;
        Elf64_Phdr * const program_headers = (Elf64_Phdr *)&mem[elf_header->e_phoff];

        for (size_t i = 0; i < elf_header->e_phnum; ++i) {
            if (program_headers[i].p_type == PT_DYNAMIC) {
                Elf64_Dyn * const dyn = (Elf64_Dyn *)&mem[program_headers[i].p_offset];
                size_t const num_dyn = program_headers[i].p_filesz / sizeof(*dyn);
                size_t stroffset = 0;

                for (size_t i = 0; i < num_dyn; ++i) {
                    if (dyn[i].d_tag == DT_STRTAB) {
                        Elf64_Shdr * const section_headers = (Elf64_Shdr *)&mem[elf_header->e_shoff];

                        for (size_t j = 0; j < elf_header->e_shnum; ++j) {
                            if (section_headers[j].sh_addr == dyn[i].d_un.d_ptr) {
                                stroffset = section_headers[j].sh_offset;
                                break;
                            }
                        }

                        break;
                    }
                }

                for (size_t i = 0; i < num_dyn; ++i) {
                    size_t const offset = stroffset + dyn[i].d_un.d_val;

                    switch (dyn[i].d_tag) {
                    case DT_NEEDED:
                    case DT_SONAME: {
                        char const *name = (char *)&mem[offset];
                        char *suffix_ptr = strstr(name, ".so");

                        // No-op version suffix for dependent libraries
                        if (suffix_ptr && suffix_ptr[3]) {
                            suffix_ptr[3] = '\0';
                        }

                        break;
                    }

                    case DT_RPATH:
                    case DT_RUNPATH: {
                        char const *new_rpath = "$ORIGIN";
                        char *rpath = (char *)&mem[offset];

                        // RUNPATH is set to $ORIGIN
                        strcpy(rpath, new_rpath);

                        if (dyn[i].d_tag == DT_RPATH) {
                            // Coerce tag from DT_RPATH to DT_RUNPATH
                            dyn[i].d_tag = DT_RUNPATH;
                        }

                        break;
                    }

                    default:
                        break;
                    }
                }

                break;
            }
        }

        msync(mem,
              file_stat.st_size,
              MS_SYNC);
        munmap(mem,
               file_stat.st_size);
    }

    return 0;
}
