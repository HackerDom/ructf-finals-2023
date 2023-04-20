#include <iostream>
#include <filesystem>

#include <gtest/gtest.h>

#include <utils/defer/defer.h>
#include <utils/files/files.h>
#include <utils/strings/strings.h>

#include <storage/storage.h>

TEST(Storage, WriteRead) {
    auto path = GetTempUniquePath("");
    Defer remover([path] { std::filesystem::remove_all(path); });

    constexpr auto kCount = 100;
    std::vector<std::vector<uint8_t>> codes;
    std::vector<std::string> descriptions;
    for (auto i = 0; i < kCount; ++i) {
        codes.emplace_back(i);
        for (auto &k : codes.back()) {
            k = i;
        }
        descriptions.push_back(Format("description-%d", i));
    }

    auto storage = Storage(path);
    std::vector<std::string> tokens;

    ASSERT_EQ(codes.size(), descriptions.size());
    for (auto i = 0; i < kCount; ++i) {
        auto r = storage.Save(codes[i], descriptions[i]);
        EXPECT_EQ(r->Status, Storage::Success);
        ASSERT_EQ(r->Error, "");
        ASSERT_NE(r->Token, "");
        tokens.push_back(r->Token);
    }

    for (auto i = 0; i < kCount; ++i) {
        auto g = storage.Get(tokens[i]);
        EXPECT_EQ(g->Status, Storage::Success);
        ASSERT_EQ(g->Error, "");
        ASSERT_EQ(g->Description, descriptions[i]);
        ASSERT_EQ(g->Code.size(), i);
        for (const auto &k: g->Code) {
            ASSERT_EQ(k, i);
        }
    }
}
