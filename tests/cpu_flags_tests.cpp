#include "../src/cpu/lazyflags.h"

#include <gtest/gtest.h>

namespace {

// Sets up the lazy flags the way SHRB and SHRW leave them. The 8086 core does
// not mask the shift count to 5 bits, so the count can be up to 255.
void SetShrFlags(Bitu type, uint16_t value, unsigned int count)
{
    lflags.type = type;
    lf_var1d = value;
    lf_var2d = count;
    lf_resd = 0;
}

class LazyFlagsShrTest : public testing::Test {
protected:
    void SetUp() override
    {
        saved_lflags = lflags;
        saved_flags = reg_flags;
    }

    void TearDown() override
    {
        lflags = saved_lflags;
        reg_flags = saved_flags;
    }

    static unsigned int CarryFromGet(Bitu type, uint16_t value, unsigned int count)
    {
        SetShrFlags(type, value, count);
        return get_CF() ? 1u : 0u;
    }

    static unsigned int CarryFromFill(Bitu type, uint16_t value, unsigned int count)
    {
        SetShrFlags(type, value, count);
        FillFlags();
        return (reg_flags & FLAG_CF) ? 1u : 0u;
    }

private:
    LazyFlags saved_lflags = {};
    Bitu saved_flags = 0;
};

TEST_F(LazyFlagsShrTest, CountAboveOperandSizeClearsCarry)
{
    for (unsigned int count = 9; count <= 255; count++) {
        EXPECT_EQ(0u, CarryFromGet(t_SHRb, 0xff, count)) << "SHR byte by " << count;
        EXPECT_EQ(0u, CarryFromFill(t_SHRb, 0xff, count)) << "SHR byte by " << count;
    }
    for (unsigned int count = 17; count <= 255; count++) {
        EXPECT_EQ(0u, CarryFromGet(t_SHRw, 0xffff, count)) << "SHR word by " << count;
        EXPECT_EQ(0u, CarryFromFill(t_SHRw, 0xffff, count)) << "SHR word by " << count;
    }
}

TEST_F(LazyFlagsShrTest, CountUpToOperandSizeKeepsLastBitShiftedOut)
{
    EXPECT_EQ(1u, CarryFromGet(t_SHRb, 0x01, 1));
    EXPECT_EQ(1u, CarryFromFill(t_SHRb, 0x01, 1));
    EXPECT_EQ(1u, CarryFromGet(t_SHRb, 0x80, 8));
    EXPECT_EQ(1u, CarryFromFill(t_SHRb, 0x80, 8));
    EXPECT_EQ(0u, CarryFromGet(t_SHRb, 0x7f, 8));
    EXPECT_EQ(0u, CarryFromFill(t_SHRb, 0x7f, 8));
    EXPECT_EQ(1u, CarryFromGet(t_SHRw, 0x8000, 16));
    EXPECT_EQ(1u, CarryFromFill(t_SHRw, 0x8000, 16));
    EXPECT_EQ(0u, CarryFromGet(t_SHRw, 0x7fff, 16));
    EXPECT_EQ(0u, CarryFromFill(t_SHRw, 0x7fff, 16));
}

}
