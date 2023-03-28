#include <cstring>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>

#include "translator.h"
#include "utils/utils.h"
#include "utils/defer.h"

bool IsElfSupported(Elf64_Ehdr *hdr) {
    if (hdr->e_ident[EI_MAG0] != ELFMAG0) {
        return false;
    }
    if (hdr->e_ident[EI_MAG1] != ELFMAG1) {
        return false;
    }
    if (hdr->e_ident[EI_MAG2] != ELFMAG2) {
        return false;
    }
    if (hdr->e_ident[EI_MAG3] != ELFMAG3) {
        return false;
    }
    if (hdr->e_ident[EI_CLASS] != ELFCLASS64) {
        return false;
    }
    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        return false;
    }
    if (hdr->e_machine != EM_X86_64) {
        return false;
    }
    if (hdr->e_ident[EI_VERSION] != EV_CURRENT) {
        return false;
    }
    if (hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) {
        return false;
    }

    return true;
}

TranslationResult TranslateAssembly(const std::string &assembly) {
    auto assemblyFilePath = GetTempUniquePath(".S");
    auto objectFilePath = GetTempUniquePath(".o");
    Defer assemblyFileRemover([&p = std::as_const(assemblyFilePath)]() { std::filesystem::remove(p); });
    Defer objectFileRemover([&p = std::as_const(objectFilePath)]() { std::filesystem::remove(p); });

    if (assemblyFilePath.empty()) {
        return {false, "cant create assembly temp file"};
    }

    if (objectFilePath.empty()) {
        return {false, "cant create object temp file"};
    }

    if (!WriteAllToFile(assemblyFilePath, assembly)) {
        return {false, "cant write assembly file"};
    }

    std::string asOutput;
    ExecuteAndGetStdout(Format("as %s -o %s", assemblyFilePath.c_str(), objectFilePath.c_str()), asOutput);
    if (!asOutput.empty()) {
        return {false, Format("compilation failed: '%s'", asOutput.c_str())};
    }

    int fd = open(objectFilePath.c_str(), O_RDONLY);
    if (fd < 0) {
        return {false, Format("cant open object file: %s", GetErrnoDescription().c_str())};
    }
    Defer fdCloser(close, fd);

    struct stat statResult{};
    if (fstat(fd, &statResult) == -1) {
        return {false, Format("fstat failed: %s", GetErrnoDescription().c_str())};
    }

    auto mapped = mmap(nullptr, statResult.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        return {false, Format("mmap on object file failed: %s", GetErrnoDescription().c_str())};
    }
    Defer elfHeaderUnMapper(munmap, mapped, statResult.st_size);
    auto *elfHeader = reinterpret_cast<Elf64_Ehdr *>(mapped);

    if (!IsElfSupported(elfHeader)) {
        return {false, "assembler generated invalid elf"};
    }

    auto *sectionHeaderTable = reinterpret_cast<Elf64_Shdr *>(reinterpret_cast<char *>(mapped) + elfHeader->e_shoff);

    if (elfHeader->e_shstrndx == SHN_UNDEF) {
        return {false, "section header string table index undefined"};
    }
    auto *strTableSectionHeader = &sectionHeaderTable[elfHeader->e_shstrndx];
    auto *stringTable = reinterpret_cast<char *>(reinterpret_cast<char *>(mapped) + strTableSectionHeader->sh_offset);

    for (Elf64_Half i = 0; i < elfHeader->e_shnum; ++i) {
        Elf64_Shdr *section = &sectionHeaderTable[i];

        if (section->sh_name == SHN_UNDEF) {
            continue;
        }

        auto *name = stringTable + section->sh_name;

        if (std::strcmp(name, ".text") == 0) {
            TranslationResult result{.success = true, .errorMessage=""};
            result.translated = std::make_shared<std::vector<uint8_t>>();
            result.translated->resize(section->sh_size);
            std::memcpy(
                result.translated->data(),
                reinterpret_cast<char *>(reinterpret_cast<char *>(mapped) + section->sh_offset),
                section->sh_size
            );

            return result;
        }
    }

    return {false, ".text section not found"};
}