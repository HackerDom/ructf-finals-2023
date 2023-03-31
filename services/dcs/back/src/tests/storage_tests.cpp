#include <iostream>
#include <filesystem>

#include <gtest/gtest.h>

#include "storage/storage.h"
#include "utils/defer.h"
#include "utils/utils.h"

TEST(Storage, ReadWrite) {
    auto storage = Storage(std::filesystem::temp_directory_path());

    std::vector<uint8_t> code{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string description = "this Is my awesome description";

    auto r = storage.SaveNotExecuted(code, description);
    ASSERT_EQ(r->Error, "");
    ASSERT_NE(r->Token, "");

    auto g = storage.Get(r->Token);
    ASSERT_EQ(g->Error, "");
    ASSERT_EQ(g->Description, description);
    ASSERT_EQ(g->Code, code);
    ASSERT_FALSE(g->IsExecuted);

    std::string err;
    ASSERT_TRUE(storage.Remove(r->Token, err));
    ASSERT_EQ(err, "");
}