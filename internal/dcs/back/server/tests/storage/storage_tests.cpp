#include <iostream>
#include <filesystem>

#include <gtest/gtest.h>
#include <glog/logging.h>

#include <utils/defer/defer.h>
#include <utils/strings/strings.h>

#include <storage/storage.h>

TEST(Storage, WriteRead) {
    auto storage = Storage(std::filesystem::temp_directory_path());

    std::vector<uint8_t> code{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string description = "this Is my awesome description";

    auto r = storage.SaveNotExecuted(code, description);
    EXPECT_EQ(r->Status, Storage::Success);
    ASSERT_EQ(r->Error, "");
    ASSERT_NE(r->Token, "");

    LOG(INFO) << Format("generated token: %s", r->Token.c_str());

    auto g = storage.Get(r->Token);
    EXPECT_EQ(g->Status, Storage::Success);
    ASSERT_EQ(g->Error, "");
    ASSERT_EQ(g->Description, description);
    ASSERT_EQ(g->Code, code);
    ASSERT_FALSE(g->IsExecuted);

    std::string err;
    EXPECT_EQ(storage.Remove(r->Token, err), Storage::Success);
    ASSERT_EQ(err, "");
}

TEST(Storage, WriteReadUpdateRead) {
    auto storage = Storage(std::filesystem::temp_directory_path());

    std::vector<uint8_t> code{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string description = "this Is my awesome description";

    auto r = storage.SaveNotExecuted(code, description);
    EXPECT_EQ(r->Status, Storage::Success);
    ASSERT_EQ(r->Error, "");
    ASSERT_NE(r->Token, "");

    LOG(INFO) << Format("generated token: %s", r->Token.c_str());

    std::string err;
    ASSERT_EQ(storage.UpdateValue(r->Token, 1337.1337, err), Storage::Success);
    ASSERT_EQ(err, "");

    auto g = storage.Get(r->Token);
    EXPECT_EQ(g->Status, Storage::Success);
    ASSERT_EQ(g->Error, "");
    ASSERT_EQ(g->Description, description);
    ASSERT_EQ(g->Code, code);
    ASSERT_TRUE(std::abs(g->Value - 1337.1337) < 1e-6);
    ASSERT_TRUE(g->IsExecuted);

    EXPECT_EQ(storage.Remove(r->Token, err), Storage::Success);
    ASSERT_EQ(err, "");
}
