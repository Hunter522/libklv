#include "gtest/gtest.h"

class KlvTest : public ::testing::Test {
protected:
    KlvTest() {

    }

    virtual ~KlvTest() {
        // Code here will be called immediately after the constructor (right
		// before each test).
    }

    virtual void SetUp() {
        // Code here will be called immediately after each test (right
		// before the destructor).
    }

    virtual void TearDown() {
        
    }

    // objects delclared here can be used by all tests in the test case for KlvTest
};

TEST_F(KlvTest, TestConstruction) {
    // test that fields are set correctly
}

TEST_F(KlvTest, TestTree) {
    // test that the tree is constructed correctly
}