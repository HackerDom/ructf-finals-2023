#include <iostream>
#include <string>
#include <cstring>

#include <unistd.h>
#include <sys/mman.h>

#include <utils/defer/defer.h>

#include <parser/parser.h>
#include <compiler/compiler.h>
#include <translator/translator.h>

int main() {
    std::string text;
    std::string line;

    while (std::getline(std::cin, line)) {
        text += line + '\n';
    }

    auto tokens = TokenizeString(text);
    if (!tokens.Success) {
        std::cout << tokens.ErrorMessage << std::endl;
        return 1;
    }
    auto ast = ParseTokens(tokens.Tokens);
    if (!ast.Success) {
        std::cout << ast.ErrorMessage << std::endl;
        return 1;
    }
    auto compiled = CompileToAssembly(ast.ProgramNode);
    if (!compiled.Success) {
        std::cout << compiled.ErrorMessage << std::endl;
        return 1;
    }
    auto translated = TranslateAssembly(compiled.AssemblyCode);
    if (!translated.Success) {
        std::cout << translated.ErrorMessage << std::endl;
        return 1;
    }

    auto pagesCount = (translated.Translated->size() + getpagesize()) / getpagesize();
    auto size = pagesCount * getpagesize();
    typedef double (*vptr)();
    auto region = mmap(nullptr, size, PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (region == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    Defer u(munmap, region, size);
    std::memcpy(reinterpret_cast<char*>(region), translated.Translated->data(), translated.Translated->size());
    if (mprotect(region, size, PROT_EXEC | PROT_READ) < 0) {
        perror("mprotect");
        return 1;
    }
    auto l = reinterpret_cast<vptr>(region);
    double r = l();

    std::cout << std::setprecision(30) << r << std::endl;

    return 0;
}
