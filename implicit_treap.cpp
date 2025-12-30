#include <iostream>
#include <random>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

using std::cin;
using std::cout;
using std::ios_base;
using std::mt19937;
using std::ostringstream;
using std::random_device;
using std::stack;
using std::string;
using std::swap;
using std::uniform_int_distribution;
using std::vector;

#define ASSERT_EQ(res, exp, func_name)             \
    do {                                            \
        if ((res) != (exp)) {                        \
            std::cerr << "ASSERT FAILED:\n"           \
                      << "In " << (func_name) << "\n"  \
                      << "Expected: " << (exp) << "\n"  \
                      << "Got: " << (res) << "\n"        \
                      << "Line: " << __LINE__ << "\n";    \
            std::exit(1);                                  \
        }                                                   \
    } while (0)


constexpr int kLeft = -1e9;
constexpr int kRight = 1e9;

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> dist(kLeft, kRight);

template <typename T>
class implicit_treap {
private:
    struct Roots {
        int left;
        int right;
    };

    class Node {
    public:
        int key;
        T data;
        int size;
        int prev;
        int left;
        int right;
        T add;
        bool reverse;

        Node(int out_key, T out_data)
            : key(out_key),
              data(out_data),
              size(1),
              prev(-1),
              left(-1),
              right(-1),
              add(T{}),
              reverse(false) {};
    };

    vector<Node> tree_;
    stack<int> unused_;
    int root_;

    void LazyPush(int idx) {
        if (idx == -1) {
            return;
        }
        int add = tree_[idx].add;
        tree_[idx].add = 0;
        tree_[idx].data += add;
        int left_son = tree_[idx].left;
        int right_son = tree_[idx].right;
        
        if (left_son != -1) {
            tree_[left_son].add += add;
        }
        if (right_son != -1) {
            tree_[right_son].add += add;
        }
    }

    void ReverseLazyPush(int idx) {
        if (idx == -1) {
            return;
        }

        bool reverse = tree_[idx].reverse;
        tree_[idx].reverse = false;

        if (reverse) {
            swap(tree_[idx].left, tree_[idx].right);
        }

        int left_son = tree_[idx].left;
        int right_son = tree_[idx].right;
        
        if (left_son != -1) {
            tree_[left_son].reverse ^= reverse;
        }
        if (right_son != -1) {
            tree_[right_son].reverse ^= reverse;
        }
    }

    Roots Split(int idx, int value) {
        LazyPush(idx);
        ReverseLazyPush(idx);
        if (idx == -1) {
            return {idx, idx};
        }

        int left_idx = tree_[idx].left;
        int left_size = (left_idx == -1) ? 0 : tree_[left_idx].size;
        int right_size;
        Roots res;
        if (value <= left_size) {
            res = Split(left_idx, value);

            if (res.left != -1 && tree_[res.left].prev != -1) {
                int prev = tree_[res.left].prev;
                tree_[prev].left = -1;
                tree_[res.left].prev = -1;
            }

            tree_[idx].left = res.right;
            if (res.right != -1) {
                tree_[res.right].prev = idx;
            }

            res.right = idx;
        } else {
            res = Split(tree_[idx].right, value - left_size - 1);

            if (res.right != -1 && tree_[res.right].prev != -1) {
                int prev = tree_[res.right].prev;
                tree_[prev].right = -1;
                tree_[res.right].prev = -1;
            }

            tree_[idx].right = res.left;
            if (res.left != -1) {
                tree_[res.left].prev = idx;
            }

            res.left = idx;
        }
        left_size = (tree_[idx].left == -1) ? 0 : tree_[tree_[idx].left].size;
        right_size = (tree_[idx].right == -1) ? 0 : tree_[tree_[idx].right].size;
        tree_[idx].size = left_size + right_size + 1;
        return res;
    }

    int Merge(int left, int right) {
        LazyPush(left);
        ReverseLazyPush(left);

        LazyPush(left);
        ReverseLazyPush(right);
        if (left == -1) {
            return right;
        }

        if (right == -1) {
            return left;
        }

        int root;
        if (tree_[left].key >= tree_[right].key) {
            tree_[left].right = Merge(tree_[left].right, right);
            int right_son = tree_[left].right;
            if (right_son != -1) {
                tree_[right_son].prev = left;
            }
            root = left;
        } else {
            tree_[right].left = Merge(left, tree_[right].left);
            int left_son = tree_[right].left;
            if (left_son != -1) {
                tree_[left_son].prev = right;
            }
            root = right;
        }
        int lss = (tree_[root].left == -1) ? 0 : tree_[tree_[root].left].size;
        int rss = (tree_[root].right == -1) ? 0 : tree_[tree_[root].right].size;
        tree_[root].size = lss + rss + 1;
        return root;
    }

    void Inorder(int root) {
        if (root != -1) {
            return;
        }
        Inorder(tree_[root].left);
        cout << tree_[root].data;
        Inorder(tree_[root].right);
    }

    void CollectDeletedItems(int root) {
        if (root == -1) {
            return;
        }
        unused_.push(root);
        CollectDeletedItems(tree_[root].left);
        CollectDeletedItems(tree_[root].right);
    }

public:
    implicit_treap() : root_(-1) {}

    int Add(int left, int right, int value) {
        if (root_ == -1) {
            return;
        }

        Roots res1 = Split(root_, right + 1);
        Roots res2 = Split(res1.left, left);

        if (res2.right != -1) {
            tree_[res2.right].add += value;
        }

        root_ = Merge(res2.left, res1.right);
    }

    void Reverse(int left, int right) {
        if (root_ == -1) {
            return;
        }

        Roots res1 = Split(root_, right + 1);
        Roots res2 = Split(res1.left, left);

        if (res2.right != -1) {
            tree_[res2.right].reverse ^= true;
        }

        left = Merge(res2.left, res2.right);
        root_ = Merge(left, res1.right);
    }

    void Insert(int value, T data) {
        Roots res1 = Split(root_, value);
        
        int idx;
        if (!unused_.empty()) {
            idx = unused_.top();
            unused_.pop();
            tree_[idx] = Node(dist(gen), data);    
        } else {
            tree_.push_back(Node(dist(gen), data));
            idx = tree_.size() - 1;
        }
        int left = Merge(res1.left, idx);
        root_ = Merge(left, res1.right);
    }

    void Delete(int left, int right) {
        if (root_ == -1) {
            return;
        }
        Roots res1 = Split(root_, right + 1);
        Roots res2 = Split(res1.left, left);
        root_ = Merge(res2.left, res1.right);
        CollectDeletedItems(res2.right);
    }

    void CyclicShift(int left, int right, int count) {
        Roots res_r = Split(root_, right + 1);
        Roots res_m = Split(res_r.left, left + count);
        Roots res_l = Split(res_m.left, left);
        int middle = Merge(res_m.right, res_l.right);
        int new_left = Merge(res_l.left, middle);
        root_ = Merge(new_left, res_r.right);
    }

    void Print(int root) { Inorder(root); }

    class Iterator {
    private:
        implicit_treap* treap_;
        int idx_;

        int LeftMost(int node) const {
            if (node == -1) {
                return node;
            }
            treap_->LazyPush(node);
            treap_->ReverseLazyPush(node);
            while (treap_->tree_[node].left != -1) {
                node = treap_->tree_[node].left;
                treap_->LazyPush(node);
                treap_->ReverseLazyPush(node);
            }
            return node;
        }

        int RightMost(int node) const {
            if (node == -1) {
                return node;
            }
            treap_->LazyPush(node);
            treap_->ReverseLazyPush(node);
            while (treap_->tree_[node].right != -1) {
                node = treap_->tree_[node].right;
                treap_->LazyPush(node);
                treap_->ReverseLazyPush(node);
            }
            return node;
        }

    public:
        using IteratorCategory = std::bidirectional_iterator_tag;
        using ValueType = T;
        using DifferenceType = std::ptrdiff_t;
        using Pointer = T*;
        using Reference = T&;

        Iterator(implicit_treap* treap, int idx) : treap_(treap), idx_(idx) {}

        T& operator*() { return treap_->tree_[idx_].data; }

        Iterator& operator++() {
            if (idx_ == -1) {
                return *this;
            }

            if (treap_->tree_[idx_].right != -1) {
                idx_ = LeftMost(treap_->tree_[idx_].right);
                return *this;
            }

            int prev = treap_->tree_[idx_].prev;
            while (prev != -1 && treap_->tree_[prev].right == idx_) {
                idx_ = prev;
                prev = treap_->tree_[prev].prev;
            }
            idx_ = prev;
            return *this;
        }

        Iterator& operator--() {
            if (idx_ == -1 || idx_ == 0) {
                return *this;
            }

            if (treap_->tree_[idx_].left != -1) {
                idx_ = RightMost(treap_->tree_[idx_].left);
                return *this;
            }

            int prev = treap_->tree_[idx_].prev;
            while (prev != -1 && treap_->tree_[prev].left == idx_) {
                idx_ = prev;
                prev = treap_->tree_[prev].prev;
            }
            idx_ = prev;
            return *this;
        }

        bool operator==(const Iterator& other) const {
            return idx_ == other.idx_;
        }

        bool operator!=(const Iterator& other) const {
            return idx_ != other.idx_;
        }
    };

    Iterator Get(int value) {
        if (root_ == -1) {
            return Iterator(this, -1);;
        }
        int lss = (tree_[root_].left == -1) ? 0 : tree_[tree_[root_].left].size;
        Roots res1 = Split(root_, value + 1);
        Roots res2 = Split(res1.left, value);
        if (value <= lss) {
            int left = Merge(res2.left, res2.right);
            Merge(left, res1.right);
        } else {
            int right = Merge(res2.right, res1.right);
            Merge(res1.left, right);
        }
        return Iterator(this, res2.right);
    }

    Iterator begin() {
        int node = root_;
        if (node == -1) {
            return Iterator(this, -1);
        }
        LazyPush(node);
        ReverseLazyPush(node);
        while (tree_[node].left != -1) {
            node = tree_[node].left;
            LazyPush(node);
            ReverseLazyPush(node);
        }
        return Iterator(this, node);
    }

    Iterator end() { return Iterator(this, -1); }
};

string ToString(implicit_treap<char>& t) {
    string res;
    for (char data : t) {
        res.push_back(data);
    }
    return res;
}

void TestInsert() {
    implicit_treap<char> treap;
    string str;

    treap.Insert(0, 'g');
    str.insert(0, 1, 'g');
    treap.Insert(1, 'v');
    str.insert(1, 1, 'v');
    treap.Insert(1, 'q');
    str.insert(1, 1, 'q');

    ASSERT_EQ(ToString(treap), str, "TestInsert");
}

void TestDelete() {
    implicit_treap<char> treap;
    string str = "il";

    for (int i = 0; i < (int)str.size(); ++i) {
        treap.Insert(i, str[i]);
    }

    treap.Delete(0, 0);
    str.erase(0, 1);

    ASSERT_EQ(ToString(treap), str, "TestDelete");
}

void TestReverse() {
    implicit_treap<char> treap;
    string str = "abcdef";

    for (int i = 0; i < (int)str.size(); ++i) {
        treap.Insert(i, str[i]);
    }

    treap.Reverse(1, 4);
    std::reverse(str.begin() + 1, str.begin() + 5);

    ASSERT_EQ(ToString(treap), str, "TestReverse");
}

void TestCyclicShift() {
    implicit_treap<char> treap;
    string str = "abcdef";

    for (int i = 0; i < (int)str.size(); ++i) {
        treap.Insert(i, str[i]);
    }

    int left = 1;
    int right = 4;
    int k_shift = 2;

    treap.CyclicShift(left, right, k_shift);

    string tmp = str.substr(left, right - left + 1);
    std::rotate(tmp.begin(), tmp.begin() + k_shift, tmp.end());
    str.replace(left, right - left + 1, tmp);

    ASSERT_EQ(ToString(treap), str, "TestCyclicShift");
}

void PrintProgress(int current, int total, const string& last_op) {
    static const int bar_width = 40;
    static size_t last_len = 0;

    float progress = float(current) / total;
    int filled = int(bar_width * progress);

    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < bar_width; ++i) {
        oss << (i < filled ? '#' : '-');
    }
    oss << "] "
        << int(progress * 100) << "% "
        << "(" << current << "/" << total << ") "
        << last_op;

    string line = oss.str();

    cout << "\r" << line;

    if (line.size() < last_len) {
        cout << string(last_len - line.size(), ' ');
    }

    last_len = line.size();
    cout << std::flush;
}


void StressTest() {
    implicit_treap<char> treap;
    string str;
    mt19937 rng(123);
    for (int step = 0; step < 10000; ++step) {
        int op = rng() % 4;

        string func_name;
        if (op == 0 && !str.empty()) {
            int left = rng() % str.size();
            int right = rng() % str.size();
            if (left > right) {
                swap(left, right);
            }
            treap.Delete(left, right);
            str.erase(left, right - left + 1);
            func_name = "TestDelete";
        }
        else if (op == 1) {
            int pos = rng() % (str.size() + 1);
            char data = 'a' + rng() % 26;
            treap.Insert(pos, data);
            str.insert(pos, 1, data);
            func_name = "TestInsert";
        }
        else if (op == 2 && !str.empty()) {
            int left = rng() % str.size();
            int right = rng() % str.size();
            if (left > right) {
                swap(left, right);
            }
            treap.Reverse(left, right);
            std::reverse(str.begin() + left, str.begin() + right + 1);
            func_name = "TestReverse";
        }
        else if (op == 3 && !str.empty()) {
            int left = rng() % str.size();
            int right = rng() % str.size();
            if (left > right) {
                swap(left, right);
            }
            int k_shift = rng() % (right - left + 1);
            treap.CyclicShift(left, right, k_shift);

            string tmp = str.substr(left, right - left + 1);
            std::rotate(tmp.begin(), tmp.begin() + k_shift, tmp.end());
            str.replace(left, right - left + 1, tmp);
            func_name = "TestCyclicShift";
        }
        if (step % 50 == 0) {
            PrintProgress(step, 10000, func_name);
        }

        ASSERT_EQ(ToString(treap), str, func_name);
    }
    PrintProgress(10000, 10000, "DONE");
    cout << "\n";
}

void TestIterator() {
    implicit_treap<char> treap;
    string str = "hello_world";

    for (int i = 0; i < (int)str.size(); ++i)
        treap.Insert(i, str[i]);

    auto it = treap.begin();
    for (char c : str) {
        ASSERT_EQ(*it, c, "TestIterator");
        ++it;
    }
    ASSERT_EQ(it == treap.end(), true, "TestIterator");
}

int main() {
    TestInsert();
    cout << "Insert test is passed: 1/6\n";
    TestDelete();
    cout << "Delete test is passed: 2/6\n";
    TestReverse();
    cout << "Reverse test is passed: 3/6\n";
    TestCyclicShift();
    cout << "CyclicShift test is passed: 4/6\n";
    TestIterator();
    cout << "Iterator test is passed: 5/6\n";
    StressTest();
    cout << "Stress test is passed: 6/6\n";

    cout << "ALL TESTS PASSED\n";
}