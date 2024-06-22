#include <gtest/gtest.h>
#include "chunk/chunk.h"
#include "encoder/xdelta.h"
using namespace Delta;
TEST(XDELTA_TEST, BasicEncodeDecodeTest) {
    int test_size = 1024;
    uint8_t buf1[test_size], buf2[test_size];
    for (int i = 0; i < test_size; i++) {
        buf2[i] = buf1[i] = i % 256;
    }
    for (int i = 0; i < test_size; i += 16) {
        buf2[i] = 256 - (i % 256);
    }
    auto chunk1 = Chunk::FromMemoryRef(buf1, test_size, 1);
    auto chunk2 = Chunk::FromMemoryRef(buf2, test_size, 2);
    XDelta xdelta;
    auto delta_chunk = xdelta.encode(chunk1, chunk2)->DeepCopy();
    auto decoded_chunk = xdelta.decode(chunk1, delta_chunk);
    auto result_buf = decoded_chunk->buf();
    for (int i = 0; i < test_size; i++) {
        ASSERT_EQ(result_buf[i], buf2[i]);
    }
}