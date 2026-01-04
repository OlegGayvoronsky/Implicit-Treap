#include "implicit_treap.h"
#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <string>
#include <vector>

using std::reverse;
using std::rotate;
using std::string;
using std::swap;
using std::vector;

TEST(ImplicitTreapTest, Insert) {
    ImplicitTreap<char> treap;
    vector<char> vec;

    treap.Insert(0, 'g');
    vec.insert(vec.begin(), 'g');

    treap.Insert(1, 'v');
    vec.insert(vec.begin() + 1, 'v');

    treap.Insert(1, 'q');
    vec.insert(vec.begin() + 1, 'q');

    EXPECT_EQ(ToVector(treap), vec);
}

TEST(ImplicitTreapTest, Delete) {
    ImplicitTreap<char> treap = {'i', 'l'};
    vector<char> vec = {'i', 'l'};

    treap.Delete(0, 0);
    vec.erase(vec.begin());

    EXPECT_EQ(ToVector(treap), vec);
}

TEST(ImplicitTreapTest, Reverse) {
    ImplicitTreap<char> treap = {'a','b','c','d','e','f'};
    vector<char> vec = {'a','b','c','d','e','f'};

    treap.Reverse(1, 4);
    reverse(vec.begin() + 1, vec.begin() + 5);

    EXPECT_EQ(ToVector(treap), vec);
}

TEST(ImplicitTreapTest, CyclicShift) {
    ImplicitTreap<char> treap = {'a','b','c','d','e','f'};
    vector<char> vec = {'a','b','c','d','e','f'};

    int left = 1;
    int right = 4;
    int k_shift = 2;

    treap.CyclicShift(left, right, k_shift);

    rotate(
        vec.begin() + left,
        vec.begin() + left + k_shift,
        vec.begin() + right + 1
    );

    EXPECT_EQ(ToVector(treap), vec);
}

TEST(ImplicitTreapTest, Iterator) {
    ImplicitTreap<char> treap = {'h', 'e', 'l', 'l', 'o', '_', 'w', 'o', 'r', 'l', 'd'};
    vector<char> vec = {'h', 'e', 'l', 'l', 'o', '_', 'w', 'o', 'r', 'l', 'd'};

    auto it = treap.begin();
    for (auto c : vec) {
        ASSERT_NE(it, treap.end());
        EXPECT_EQ(*it, c);
        ++it;
    }

    EXPECT_EQ(it, treap.end());
}

TEST(ImplicitTreapTest, Stress) {
    ImplicitTreap<char> treap;
    vector<char> vec;

    std::mt19937 rng(123);

    for (int step = 0; step < 10000; ++step) {
        int op = rng() % 4;

        if (op == 0 && !vec.empty()) {
            int l = rng() % vec.size();
            int r = rng() % vec.size();
            if (l > r) swap(l, r);

            treap.Delete(l, r);
            vec.erase(vec.begin() + l, vec.begin() + r + 1);
        }
        else if (op == 1) {
            int pos = rng() % (vec.size() + 1);
            char data = 'a' + rng() % 26;

            treap.Insert(pos, data);
            vec.insert(vec.begin() + pos, data);
        }
        else if (op == 2 && !vec.empty()) {
            int l = rng() % vec.size();
            int r = rng() % vec.size();
            if (l > r) swap(l, r);

            treap.Reverse(l, r);
            reverse(vec.begin() + l, vec.begin() + r + 1);
        }
        else if (op == 3 && !vec.empty()) {
            int l = rng() % vec.size();
            int r = rng() % vec.size();
            if (l > r) swap(l, r);

            int k = rng() % (r - l + 1);
            treap.CyclicShift(l, r, k);

            rotate(vec.begin() + l,
                   vec.begin() + l + k,
                   vec.begin() + r + 1);
        }

        ASSERT_EQ(ToVector(treap), vec);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}