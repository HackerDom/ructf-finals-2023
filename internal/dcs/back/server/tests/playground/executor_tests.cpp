#include <gtest/gtest.h>
#include <glog/logging.h>

#include <translator/translator.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <playground/executor.h>

TEST(Executor, NoTimeout) {
    using namespace std::chrono_literals;

    auto tokens = TokenizeString(R"(
fun main() {
    return 3.14;
}
)");
    ASSERT_EQ(tokens.ErrorMessage, "");
    ASSERT_TRUE(tokens.Success);
    auto parsed = ParseTokens(tokens.Tokens);
    ASSERT_EQ(parsed.ErrorMessage, "");
    ASSERT_TRUE(parsed.Success);
    auto compiled = CompileToAssembly(parsed.ProgramNode);
    ASSERT_EQ(compiled.ErrorMessage, "");
    ASSERT_TRUE(compiled.Success);
    auto translated = TranslateAssembly(compiled.AssemblyCode);
    ASSERT_EQ(translated.ErrorMessage, "");
    ASSERT_TRUE(translated.Success);

    auto e = Executor(10);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 2'00; ++i) {
        auto s = e.Execute(*translated.Translated);

        ASSERT_EQ(s->ErrorMessage, "");
        ASSERT_EQ(s->Status, Executor::ExecuteResult::OK);
        ASSERT_TRUE(std::abs(s->Value - 3.14) < 1e-6);
    }
    auto duration = std::chrono::high_resolution_clock::now() - start;
    LOG(INFO) << "simple return of value execution tooks "
              << (std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 200) << "ms";
    ASSERT_LE(duration, 2'00 * 100ms);
}

TEST(Executor, Timeout) {
    /*
      lea     0(%rip),%rax
      jmp     *%rax
     */
    std::vector<uint8_t> code = {0x48, 0x8d, 0x05, 0, 0, 0, 0, 0xff, 0xe0};
    auto e = Executor(10);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 200; ++i) {
        auto s = e.Execute(code);
        ASSERT_EQ(s->Status, Executor::ExecuteResult::TIMEOUT);
    }
    auto duration = std::chrono::high_resolution_clock::now() - start;
    LOG(INFO) << "timeout execution got "
              << (std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 200) << "ms";
}
