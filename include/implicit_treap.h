#include <iostream>
#include <random>
#include <stack>
#include <vector>

template <typename T>
class ImplicitTreap {
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

    std::vector<Node> tree_;
    std::stack<int> unused_;
    int root_;
    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<int> dist{-1000000000, 1000000000};

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
            std::swap(tree_[idx].left, tree_[idx].right);
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
        std::cout << tree_[root].data;
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
    ImplicitTreap() : root_(-1) {}

    ImplicitTreap(const ImplicitTreap& other) = default;

    ImplicitTreap& operator=(const ImplicitTreap& other) = default;
    
    ImplicitTreap(std::initializer_list<T> init) : ImplicitTreap() {
        int position = 0;
        for (auto data : init) {
            Insert(position++, data);
        }
    }

    ImplicitTreap(ImplicitTreap&& other) noexcept : tree_(std::move(other.tree_)), root_(other.root_) {
        other.root_ = -1;
    }

    ImplicitTreap& operator=(ImplicitTreap&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        tree_ = std::move(other.tree_);
        root_ = other.root_;
        other.root_ = -1;
        return *this;
    }

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
        ImplicitTreap* treap_;
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

        Iterator(ImplicitTreap* treap, int idx) : treap_(treap), idx_(idx) {}

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

template <typename T>
std::vector<T> ToVector(ImplicitTreap<T>& t) {
    std::vector<T> res;
    for (const T& x : t) {
        res.push_back(x);
    }
    return res;
}