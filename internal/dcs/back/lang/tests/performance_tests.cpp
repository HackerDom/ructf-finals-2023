#include <string>
#include <chrono>

#include <gtest/gtest.h>
#include <glog/logging.h>

#include <lexer/lexer.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <translator/translator.h>

static constexpr int kMinLength = 10'000;
static constexpr int kIterationsCount = 200;

TEST(Perf, CompileManyAssignments) {
    std::ostringstream text;
    text << "fun main() {" << std::endl;
    text << "x = 1;" << std::endl;

    while (text.tellp() < kMinLength) {
        text << "x = (x + x) - (x / 3.14);" << std::endl;
    }

    text << "return 0;" << std::endl << "}";

    auto s = text.str();

    ASSERT_GE(s.size(), kMinLength);
    LOG(INFO) << "text size: " << s.size();

    auto tokens = TokenizeString(s);
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

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kIterationsCount; ++i) {
        tokens = TokenizeString(s);
        parsed = ParseTokens(tokens.Tokens);
        compiled = CompileToAssembly(parsed.ProgramNode);
        translated = TranslateAssembly(compiled.AssemblyCode);
        ASSERT_TRUE(translated.Success);
    }
    auto dt = std::chrono::high_resolution_clock::now() - start;

    LOG(INFO) << "linear large compilation took "
              << (std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() / kIterationsCount) << "ms";
    LOG(INFO) << "assembly string length: " << compiled.AssemblyCode.size();
    LOG(INFO) << "translated code size: " << translated.Translated->size() << "bytes";
}

TEST(Perf, CompilationLargeExpression) {
    std::ostringstream text;
    text << "fun main() {" << std::endl;
    text << "y = 1; z = 2; x = ";

    while (text.tellp() < kMinLength) {
        text << "((y + z) * (z - y) / y) + ";
    }

    text << "0;" << std::endl << "return 0;" << std::endl << "}";
    auto s = text.str();
    ASSERT_GE(s.size(), kMinLength);
    LOG(INFO) << "text size: " << s.size();

    auto tokens = TokenizeString(s);
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

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kIterationsCount; ++i) {
        tokens = TokenizeString(s);
        parsed = ParseTokens(tokens.Tokens);
        compiled = CompileToAssembly(parsed.ProgramNode);
        translated = TranslateAssembly(compiled.AssemblyCode);
        ASSERT_TRUE(translated.Success);
    }
    auto dt = std::chrono::high_resolution_clock::now() - start;

    LOG(INFO) << "large expression compilation took "
              << (std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() / kIterationsCount) << "ms";
    LOG(INFO) << "assembly string length: " << compiled.AssemblyCode.size();
    LOG(INFO) << "translated code size: " << translated.Translated->size() << "bytes";
}

static void addCondition(std::ostringstream &o, int n) {
    o << "if (y < z) {" << std::endl;
    if (n > 0) {
        addCondition(o, n - 1);
    } else {
        o << "x = x + 1;" << std::endl;
    }
    o << "}" << std::endl;
}

TEST(Perf, CompilationNestedConditions) {
    std::ostringstream text;
    text << "fun main() {" << std::endl;
    text << "y = 1; z = 2;";

    while (text.tellp() < kMinLength) {
        addCondition(text, 100);
    }

    text << "return x; }";

    auto s = text.str();
    ASSERT_GE(s.size(), kMinLength);
    LOG(INFO) << "text size: " << s.size();

    auto tokens = TokenizeString(s);
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

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kIterationsCount; ++i) {
        tokens = TokenizeString(s);
        parsed = ParseTokens(tokens.Tokens);
        compiled = CompileToAssembly(parsed.ProgramNode);
        translated = TranslateAssembly(compiled.AssemblyCode);
        ASSERT_TRUE(translated.Success);
    }
    auto dt = std::chrono::high_resolution_clock::now() - start;

    LOG(INFO) << "large nesting compilation took "
              << (std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() / kIterationsCount) << "ms";
    LOG(INFO) << "assembly string length: " << compiled.AssemblyCode.size();
    LOG(INFO) << "translated code size: " << translated.Translated->size() << "bytes";
}
