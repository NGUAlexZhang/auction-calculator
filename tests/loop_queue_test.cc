#include "loop_queue.h"
#include <gtest/gtest.h>

TEST(LoopQueueTest, TestSingleThreadPushAndPop) {
    LoopQueue<int> queue(1024);
    queue.push(1);
    queue.push(1);
    queue.push(4);
    queue.push(5);
    queue.push(1);
    queue.push(4);
    auto num = queue.pop();
    EXPECT_EQ(num, 1);

    num = queue.pop();
    EXPECT_EQ(num, 1);

    num = queue.pop();
    EXPECT_EQ(num, 4);

    num = queue.pop();
    EXPECT_EQ(num, 5);

    num = queue.pop();
    EXPECT_EQ(num, 1);

    num = queue.pop();
    EXPECT_EQ(num, 4);

    num = queue.pop();
    EXPECT_EQ(num, std::nullopt);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}