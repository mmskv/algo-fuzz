#include <algorithm>
#include <sstream>

#include "fuzztest/fuzztest.h"
#include "gtest/gtest.h"

void ExampleMax(std::istream& in, std::ostream& out);

// As the ExampleMax function uses streams for input and output we use a wrapper
// function that creates in/out streams for ExampleMax
int RunExampleMax(int first, int second) {
    std::stringstream in;
    std::stringstream out;

    in << first << " " << second << std::endl;

    ExampleMax(in, out);

    int result;
    out >> result;

    return result;
}

int ReferenceMax(int first, int second) { return std::max(first, second); }

void TestExampleMax(int first, int second) {
    int actual = RunExampleMax(first, second);
    int expected = ReferenceMax(first, second);

    EXPECT_EQ(actual, expected);
}

// Non-fuzztest test case with hardcoded inputs and expected results
TEST(ExampleMax, Simple) {
    EXPECT_EQ(RunExampleMax(1, 2), 2);
    EXPECT_EQ(RunExampleMax(2, 1), 2);
    EXPECT_EQ(RunExampleMax(1, 1), 1);
}

// ExampleMax is a test suite name, TestExampleMax is a test case name
// .WithDomains controls what numbers are evaluated as inputs when doing
// profile-guided fuzzing
// In this case we expect to work with any int
FUZZ_TEST(ExampleMax, TestExampleMax)
    .WithDomains(fuzztest::Arbitrary<int>(), fuzztest::Arbitrary<int>());
