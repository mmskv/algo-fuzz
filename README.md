# dsa-fuzz

This project is a base for your DSA course submissions

## Features

- Cmake to build binaries and tests in one go with sanitizers
- Examples incorporating [gtest](https://github.com/google/googletest) and [fuzztest](https://github.com/google/fuzztest)
- clang presets

## Repository structure

- `./algos/` -- directory with algos
- `./tests/` -- directory with tests

simple as

## tldr

Run tests with the following command:

```
./tests/example_max --gtest_color=yes
```

Run fuzz tests **infinitely** with the following command:

```
./tests/example_max --gtest_color=yes --fuzz
```

## How to use

There are some sample algorithms and tests already present. Let's build and run them.

```bash
git submodule update --init
mkdir build
cd build
CC=clang CXX=clang++ cmake \
    -DCMAKE_BUILD_TYPE=RelWithDebugInfo -DFUZZTEST_FUZZING_MODE=on ..
cmake --build . -j $(nproc)
```

After compilation finishes you should see two directories inside the `./build/`
directory: `

- `./algos/` should have an example_max binary
- `./tests/` should also have an example_max binary

### Let's run the algorithm first

```
echo "1 2" | ./algos/example_max
```

You should see max of these two numbers output to the terminal.

### Now let's run the tests

```
./tests/example_max --gtest_color=yes --fuzz=
```

You should see output like this

```
[.] Sanitizer coverage enabled. Counter map size: 46841, Cmp map size: 262144
[==========] Running 2 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 2 tests from ExampleMax
[ RUN      ] ExampleMax.Simple
[       OK ] ExampleMax.Simple (0 ms)
[ RUN      ] ExampleMax.TestExampleMax
FUZZTEST_PRNG_SEED=vW6QXZC7D3usIpKiQsYDj8ZdFVyRZ9K15YSjpQje6OA
/home/mmskv/shad/dsa-fuzz/tests/example_max.cpp:31: Failure
Expected equality of these values:
  actual
    Which is: 1
  expectedG
    Which is: 694201337
Stack trace:
...blahblahblah
...

=================================================================
=== BUG FOUND!

/home/suck/shad/dsa-fuzz/tests/example_max.cpp:45: Counterexample found for ExampleMax.TestExampleMax.
The test fails with input:
argument 0: 694201337
argument 1: -102747479

=================================================================
=== Regression test draft

TEST(ExampleMax, TestExampleMaxRegression) {
  TestExampleMax(
    694201337,
    -102747479
  );
}

Please note that the code generated above is best effort and is intended
to use be used as a draft regression test.
For reproducing findings please rely on file based reproduction.

=================================================================
[  FAILED  ] ExampleMax.TestExampleMax (139 ms)
[----------] 2 tests from ExampleMax (140 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test suite ran. (140 ms total)
[  PASSED  ] 1 test.
[  FAILED  ] 1 test, listed below:
[  FAILED  ] ExampleMax.TestExampleMax

 1 FAILED TEST
```

#### Let me break down tests output for you

1. First the hardcoded tests are ran. They are defined like this in the `./tests/example_max.cpp` file

   ```cpp
   TEST(ExampleMax, Simple) {
       EXPECT_EQ(RunExampleMax(1, 2), 2);
       EXPECT_EQ(RunExampleMax(2, 1), 2);
       EXPECT_EQ(RunExampleMax(1, 1), 1);
   }
   ```

   They succeed thus the following output is outputted

   ```
   [ RUN      ] ExampleMax.Simple
   [       OK ] ExampleMax.Simple (0 ms)
   ```

2. Then the fuzz test is ran. But it fails and the ouptut tells us everything:

   Expected and actual outputs:

   ```
   Expected equality of these values:
     actual
       Which is: 1
     expected
       Which is: 694201337
   ```

   Input which triggers the fail:

   ```
   The test fails with input:
   argument 0: 694201337
   argument 1: -102747479
   ```

   It even suggest a new non-fuzz test for us to add:

   ```
   TEST(ExampleMax, TestExampleMaxRegression) {
     TestExampleMax(
       694201337,
       -102747479
     );
   }
   ```

##### But why did it fail? And how did fuzz test find the faling case?

You see, the test contains a silly mistake, an edge case, if you will.

See if you can find it as fast as the fuzzer.

```cpp
const int kRandomNumber = 694201337;

void ExampleMax(std::istream& in, std::ostream& out) {
    int first;
    int second;

    in >> first >> second;

    if (first == kRandomNumber) {  // crude edge case bug example
        out << 1 << std::endl;
    } else {
        out << (first > second ? first : second) << std::endl;
    }
}
```

Absolutely right, the program has a bug when the first number is
694201337. Traditional stress case iterating over every number would have
taken **69 gazillions iterations** to find it.

But `fuzztest` uses a profile guided fuzzing and finds this edge in **5 miliseconds**.

How does that work?

**Profile-guided fuzzing** uses instrumentation to monitor each branch in the
program. It then intelligently mutates input parameters, aiming to explore
every potential code path. By focusing on covering every branch, this
technique efficiently discovers rare edge cases, like the one in this
example, far more quickly than traditional methods.

## How to add new algorithms and tests

As you may have guessed, both your algorithm and test implementations must have
the same file name for them to build an link properly.

Let's add an ExampleMin algorithm.

1. Create a file `./algos/example_min.cpp` with the following content

```cpp
#include <iostream>

void ExampleMin(std::istream& in, std::ostream& out) {
    int first;
    int second;

    in >> first >> second;

    out << (first < second ? first : second) << std::endl;
}

#ifndef NOMAIN
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    ExampleMax(std::cin, std::cout);

    return 0;
}
#endif
```

Note the `#ifndef NOMAIN`. **You must add that around the `main()` function**
otherwise during building tests linker will link two `main()` funtions -- one
from your algorihm and one from the gtest and we don't want that.

Also note that we split the algorithm's logic that works with generic streams
into a function `ExampleMin`, while the `main` function sets up `cin`/`cout`
and calls ExampleMin with them.

We do that so that we're able to test the `ExampleMin` function in the test suite later.

2. Now let's create a file `./tests/example_min.cpp` that will have the tests

```cpp
#include <algorithm>
#include <sstream>

#include "fuzztest/fuzztest.h"
#include "gtest/gtest.h"

void ExampleMin(std::istream& in, std::ostream& out);

// As the ExampleMin function uses streams for input and output we use a wrapper
// function that creates in/out streams for ExampleMin
int RunExampleMin(int first, int second) {
    std::stringstream in;
    std::stringstream out;

    in << first << " " << second << std::endl;

    ExampleMin(in, out);

    int result;
    out >> result;

    return result;
}

int ReferenceMin(int first, int second) { return std::min(first, second); }

void TestExampleMin(int first, int second) {
    int actual = RunExampleMin(first, second);
    int expected = ReferenceMin(first, second);

    EXPECT_EQ(actual, expected);
}

// Non-fuzztest test case with hardcoded inputs and expected results
TEST(ExampleMin, TestExampleMin) {
    EXPECT_EQ(RunExampleMin(1, 2), 2);
    EXPECT_EQ(RunExampleMin(2, 1), 2);
    EXPECT_EQ(RunExampleMin(1, 1), 1);
}

// ExampleMin is a test suite name, TestExampleMin is a test case name
// .WithDomains controls what numbers are evaluated as inputs when doing
// profile-guided fuzzing
// In this case we expect to work with any int
FUZZ_TEST(ExampleMin, TestExampleMin)
    .WithDomains(fuzztest::Arbitrary<int>(), fuzztest::Arbitrary<int>());
```

3. Now when we run

    ```
    cmake --build . -j $(nproc)`
    ```

   It discovers a new algorithm and test file, links them and compiles two executables.
   You should see the following lines in cmake's output if you did everything right

   ```
   [ALGO] Adding executable: algos/example_min
   [TEST] Adding test executable: tests/example_min
   ```

## Further examples

Here are some fuzztest domains that I have used when solving other dsa tasks.
You can use them and fuzztest's docs as the starting ground.

### Merge sorted subsequences

This algorithm accepts a vectors of sorted vectors of same size and outputs a single merge sorted
array.

As you see the `WithDomains()` function doesn't generate sorted vectors and
vectors of the same size. Sorting of input is done in the `MergeSortedTest()`.
And I even skip cases where all generated vectors are not same size:

```cpp
int sequence_size = sequences.size();
int sequence_len = sequences[0].size();
for (const auto& seq : sequences) {
    if (seq.size() != sequence_len) {
        return;
    };
}
```

But I'm lazy and fuzztests are smart so that still works out perfectly without
much overhead in the end. Apparelty it quickly covers branch with sequences of
different sizes and then focuses on mutating inputs that keep sequences the same size.

```cpp
void MergeSortedTest(const std::vector<std::vector<int>>& sequences) {
    if (sequences.empty() || sequences[0].empty()) {
        return;
    }

    int sequence_size = sequences.size();
    int sequence_len = sequences[0].size();
    for (const auto& seq : sequences) {
        if (seq.size() != sequence_len) {
            return;
        };
    }

    std::vector<std::vector<int>> sorted_sequences = sequences;
    for (auto& seq : sorted_sequences) {
        std::sort(seq.begin(), seq.end());
    }

    std::stringstream in;

    in << sequence_size << " " << sequence_len << "\n";
    for (const auto& seq : sorted_sequences) {
        for (int val : seq) {
            in << val << " ";
        }
        in << "\n";
    }

    std::stringstream out;

    MergeSorted(in, out);

    std::vector<int> expected_output = ReferenceMergeSorted(sorted_sequences);

    std::vector<int> actual_output;
    int val;
    while (out >> val) {
        actual_output.push_back(val);
    }

    EXPECT_EQ(actual_output, expected_output);
}

FUZZ_TEST(MergeSortedSuite, MergeSortedTest)
    .WithDomains(
        fuzztest::VectorOf(fuzztest::VectorOf(fuzztest::InRange(-1000000000,
                                                                1000000000))
                               .WithMaxSize(1000))
            .WithMaxSize(1000));
```
