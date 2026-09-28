#ifndef __SOMETEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>

struct Hobbit {
    std::string name;
    int hp, off, def;

    friend bool operator == (const Hobbit&, const Hobbit&) = default;
};

std::ostream& operator << (std::ostream& out, const Hobbit& h) {
    return out
            << "Hobbit{\"" << h.name << "\", "
            << ".hp=" << h.hp << ", "
            << ".off=" << h.off << ", "
            << ".def=" << h.def << "}";
}

template < typename T >
std::ostream& operator << (std::ostream& out, const std::optional<T>& x) {
    if (!x) return out << "EMPTY_OPTIONAL";
    return out << "Optional{" << *x << "}";
}

#endif


struct HobbitArmy {
public:
    static constexpr bool CHECK_NEGATIVE_HP = true;

    HobbitArmy() = default;

    HobbitArmy(const HobbitArmy &) = delete;
    HobbitArmy & operator=(const HobbitArmy&) = delete;

    ~HobbitArmy()
    {
        delete root;
    }

    bool add(const Hobbit& hobbit)
    {
        if(hobbit.hp <= 0)
            return false;

        if(find(hobbit.name))
            return false;

        Statistics stats{hobbit.hp, hobbit.off, hobbit.def};
        root = AVLInsert(root, hobbit.name, stats);

        if(root)
            root->parent = nullptr;

        return true;
    }

    std::optional<Hobbit> erase(const std::string& hobbit_name)
    {
        Node * found = find(hobbit_name);

        if(!found)
            return std::nullopt;

        Hobbit hobbit{found->key, found->value._hp,
                   found->value._off, found->value._def};

        root = AVLDelete(root, hobbit_name);

        if(root)
            root->parent = nullptr;

        return hobbit;
    }

    std::optional<Hobbit> stats(const std::string& hobbit_name) const
    {
        Node * found = find(hobbit_name);

        if(!found)
            return std::nullopt;

        Hobbit hobbit{found->key, found->value._hp,
                   found->value._off, found->value._def};

        return hobbit;
    }

    bool enchant(
            const std::string& first,
            const std::string& last,
            int hp_diff,
            int off_diff,
            int def_diff
    )
    {
        if(first > last)
            return true;

        if(hp_diff < 0)
        {
            if(!checkRec(root, first, last, hp_diff))
                return false;
        }

        Statistics diff{hp_diff, off_diff, def_diff};

        updateRec(root, first, last, diff);

        return true;
    }

    void for_each(auto&& fun) const
    {
        for_each_impl(root, fun);
    }
private:
    struct Statistics {
        int _hp = 0;
        int _off = 0;
        int _def = 0;

        Statistics operator+(const Statistics & oth) const
        {
            return {_hp + oth._hp, _off + oth._off, _def + oth._def};
        }

        bool operator==(const Statistics & oth) const
        {
            return _hp == oth._hp && _off == oth._off && _def == oth._def;
        }

        bool operator!=(const Statistics & oth) const
        {
            return !(*this == oth);
        }
    };

    struct Node {
        std::string key;
        Statistics value;
        int depth;
        int min_hp;
        Statistics pending_update;
        Node * left;
        Node * right;
        Node * parent;

        Node(const std::string & k, const Statistics & val) : key(std::move(k)), value(val), depth(1), min_hp(val._hp), pending_update(), left(nullptr), right(nullptr), parent(nullptr) {}

        ~Node()
        {
            delete left;
            delete right;
        }
    };

    Node * root = nullptr;

    static int maxDepth(int a, int b)
    {
        return (a > b ? a : b);
    }

    static int getDepth(Node * node)
    {
        if(!node)
            return 0;

        return node->depth;
    }

    int getBalance(Node * node) const
    {
        if(!node)
            return 0;

        return getDepth(node->left) - getDepth(node->right);
    }

    void recalculate(Node * node) const
    {
        if(!node)
            return;

        node->depth = 1 + maxDepth(getDepth(node->left), getDepth(node->right));

        int minimum = node->value._hp;

        if(node->left)
            minimum = std::min(minimum, node->left->min_hp);

        if(node->right)
            minimum = std::min(minimum, node->right->min_hp);

        node->min_hp = minimum;
    }

    void applyUpdate(Node * node, const Statistics & diff) const
    {
        if(!node)
            return;

        node->value = node->value + diff;
        node->pending_update = node->pending_update + diff;
        node->min_hp += diff._hp;
    }

    void lazyUpdate(Node * node) const
    {
        if(!node || node->pending_update == Statistics{})
            return;

        if(node->left)
            applyUpdate(node->left, node->pending_update);

        if(node->right)
            applyUpdate(node->right, node->pending_update);

        const_cast<Node *>(node)->pending_update = Statistics{};
    }

    Node * rightRotation(Node* y)
    {
        lazyUpdate(y);

        if(y->left)
            lazyUpdate(y->left);

        Node * x = y->left;
        Node * T2 = x->right;

        x->right = y;
        y->left = T2;

        x->parent = y->parent;
        y->parent = x;

        if(T2)
            T2->parent = y;

        recalculate(y);
        recalculate(x);

        return x;
    }

    Node * leftRotation(Node * x)
    {
        lazyUpdate(x);

        if(x->right)
            lazyUpdate(x->right);

        Node * y = x->right;
        Node * T2 = y->left;

        y->left = x;
        x->right = T2;

        y->parent = x->parent;
        x->parent = y;

        if(T2)
            T2->parent = x;

        recalculate(x);
        recalculate(y);

        return y;
    }

    Node * rebalanceTreeInsert(Node * node, const std::string & key)
    {
        int currBalance = getBalance(node);

        if(currBalance > 1 && (key < node->left->key))
            return rightRotation(node);

        if(currBalance < -1 && (node->right->key < key))
            return leftRotation(node);

        if(currBalance > 1 && (node->left->key < key))
        {
            node->left = leftRotation(node->left);

            if(node->left)
                node->left->parent = node;

            return rightRotation(node);
        }

        if(currBalance < -1 && (key < node->right->key))
        {
            node->right = rightRotation(node->right);

            if(node->right)
                node->right->parent = node;

            return leftRotation(node);
        }

        return node;
    }

    Node * rebalanceTreeDelete(Node * node)
    {
        int currBalance = getBalance(node);

        if(currBalance > 1 && getBalance(node->left) >= 0)
            return rightRotation(node);

        if(currBalance > 1 && getBalance(node->left) < 0)
        {
            node->left = leftRotation(node->left);

            if(node->left)
                node->left->parent = node;

            return rightRotation(node);
        }

        if(currBalance < -1 && getBalance(node->right) <= 0)
            return leftRotation(node);

        if(currBalance < -1 && getBalance(node->right) > 0)
        {
            node->right = rightRotation(node->right);

            if(node->right)
                node->right->parent = node;

            return leftRotation(node);
        }

        return node;
    }

    Node * minValue(Node * node)
    {
        if(!node)
            return nullptr;

        lazyUpdate(node);

        Node * curr = node;

        while(curr->left)
        {
            curr = curr->left;
            lazyUpdate(curr);
        }

        return curr;
    }

    Node * find(const std::string & key) const
    {
        Node* curr = root;

        while(curr)
        {
            lazyUpdate(curr);

            if(key < curr->key)
            {
                curr = curr->left;
            }
            else if(curr->key < key)
            {
                curr = curr->right;
            }
            else
            {
                return curr;
            }
        }

        return nullptr;
    }

    Node* AVLInsert(Node* node, const std::string & key, const Statistics & value)
    {
        if(!node)
            return new Node(key, value);

        lazyUpdate(node);

        if(key < node->key)
        {
            node->left = AVLInsert(node->left, key, value);

            if(node->left)
                node->left->parent = node;
        }
        else if(node->key < key)
        {
            node->right = AVLInsert(node->right, key, value);

            if(node->right)
                node->right->parent = node;
        }
        else
        {
            return node;
        }

        recalculate(node);

        node = rebalanceTreeInsert(node, key);

        return node;
    }

    Node* AVLDelete(Node * node, const std::string & key)
    {
        if(!node)
            return node;

        lazyUpdate(node);

        if(key < node->key)
        {
            node->left = AVLDelete(node->left, key);

            if(node->left)
                node->left->parent = node;
        }
        else if(node->key < key)
        {
            node->right = AVLDelete(node->right, key);

            if(node->right)
                node->right->parent = node;
        }
        else
        {
            if(!node->left || !node->right)
            {
                Node * tmp = node;
                Node * child = node->left ? node->left : node->right;

                if(child)
                    child->parent = node->parent;

                tmp->left = tmp->right = nullptr;
                node = child;
                delete tmp;
            }
            else
            {
                Node * tmp = minValue(node->right);
                node->key = tmp->key;
                node->value = tmp->value;
                node->right = AVLDelete(node->right, tmp->key);

                if(node->right)
                    node->right->parent = node;
            }
        }

        if(!node)
            return node;

        recalculate(node);

        node = rebalanceTreeDelete(node);

        return node;
    }

    void updateLeftSubtree(Node * node, const std::string & key, const Statistics & value)
    {
        if(!node)
            return;

        lazyUpdate(node);

        bool flag = (node->key < key);

        if(flag)
        {
            updateLeftSubtree(node->right, key, value);
        }
        else
        {
            node->value = node->value + value;
            node->min_hp += value._hp;

            if(node->right)
                applyUpdate(node->right, value);

            updateLeftSubtree(node->left, key, value);
        }

        recalculate(node);
    }

    void updateRightSubtree(Node * node, const std::string & key, const Statistics & value)
    {
        if(!node)
            return;

        lazyUpdate(node);

        bool flag = (key < node->key);

        if(flag)
        {
            updateRightSubtree(node->left, key, value);
        }
        else
        {
            node->value = node->value + value;
            node->min_hp += value._hp;

            if(node->left)
                applyUpdate(node->left, value);

            updateRightSubtree(node->right, key, value);
        }

        recalculate(node);
    }

    void updateRec(Node * node, const std::string & key1, const std::string & key2, const Statistics & value)
    {
        if(!node)
            return;

        lazyUpdate(node);

        bool flag1 = (node->key < key1);
        bool flag2 = (key2 < node->key);

        if(flag1)
        {
            updateRec(node->right, key1, key2, value);
        }
        else if(flag2)
        {
            updateRec(node->left, key1, key2, value);
        }
        else
        {
            node->value = node->value + value;
            node->min_hp += value._hp;

            updateLeftSubtree(node->left, key1, value);

            updateRightSubtree(node->right, key2, value);
        }

        recalculate(node);
    }

    bool checkLeftSubtree(Node * node, const std::string & key, int diff)
    {
        if(!node)
            return true;

        lazyUpdate(node);

        bool flag = (node->key < key);

        if(flag)
        {
            return checkLeftSubtree(node->right, key, diff);
        }
        else
        {
            if(node->value._hp + diff <= 0)
                return false;

            if(node->right && node->right->min_hp + diff <= 0)
                return false;

            return checkLeftSubtree(node->left, key, diff);
        }
    }

    bool checkRightSubtree(Node * node, const std::string & key, int diff)
    {
        if(!node)
            return true;

        lazyUpdate(node);

        bool flag = (key < node->key);

        if(flag)
        {
            return checkRightSubtree(node->left, key, diff);
        }
        else
        {
            if(node->value._hp + diff <= 0)
                return false;

            if(node->left && node->left->min_hp + diff <= 0)
                return false;

            return checkRightSubtree(node->right, key, diff);
        }
    }

    bool checkRec(Node * node, const std::string & key1, const std::string & key2, int diff)
    {
        if(!node)
            return true;

        lazyUpdate(node);

        bool flag1 = (node->key < key1);
        bool flag2 = (key2 < node->key);

        if(flag1)
        {
            return checkRec(node->right, key1, key2, diff);
        }
        else if(flag2)
        {
            return checkRec(node->left, key1, key2, diff);
        }
        else
        {
            if(node->value._hp + diff <= 0)
                return false;

            if(!checkLeftSubtree(node->left, key1, diff))
                return false;

            if(!checkRightSubtree(node->right, key2, diff))
                return false;
        }

        return true;
    }

    static void for_each_impl(Node *node, auto& fun)
    {
        if (!node) return;

        if(node->pending_update != Statistics{})
        {
            if(node->left)
            {
                node->left->value = node->left->value + node->pending_update;
                node->left->pending_update = node->left->pending_update + node->pending_update;
                node->left->min_hp += node->pending_update._hp;
            }

            if(node->right)
            {
                node->right->value = node->right->value + node->pending_update;
                node->right->pending_update = node->right->pending_update + node->pending_update;
                node->right->min_hp += node->pending_update._hp;
            }

            node->pending_update = Statistics{};
        }

        for_each_impl(node->left, fun);

        Hobbit hobbit{node->key, node->value._hp,
                      node->value._off, node->value._def};

        fun(hobbit);

        for_each_impl(node->right, fun);
    };
};

#ifndef __SOMETEST__

////////////////// Dark magic, ignore ////////////////////////

template < typename T >
auto quote(const T& t) { return t; }

std::string quote(const std::string& s) {
    std::string ret = "\"";
    for (char c : s) if (c != '\n') ret += c; else ret += "\\n";
    return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
      std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")" << std::endl; \
      fail++; \
    } else ok++; \
  } while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)


////////////////// End of dark magic ////////////////////////


void check_army(const HobbitArmy& A, const std::vector<Hobbit>& ref, int& ok, int& fail) {
    size_t i = 0;

    A.for_each([&](const Hobbit& h) {
        CHECK(i < ref.size(), true);
        CHECK(h, ref[i]);
        i++;
    });

    CHECK(i, ref.size());
}

void test1(int& ok, int& fail) {
    HobbitArmy A;
    check_army(A, {}, ok, fail);

    CHECK(A.add({"Frodo", 100, 10, 3}), true);
    CHECK(A.add({"Frodo", 200, 10, 3}), false);
    CHECK(A.erase("Frodo"), std::optional(Hobbit("Frodo", 100, 10, 3)));
    CHECK(A.add({"Frodo", 200, 10, 3}), true);

    CHECK(A.add({"Sam", 80, 10, 4}), true);
    CHECK(A.add({"Pippin", 60, 12, 2}), true);
    CHECK(A.add({"Merry", 60, 15, -3}), true);
    CHECK(A.add({"Smeagol", 0, 100, 100}), false);

    if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
        CHECK(A.add({"Smeagol", -100, 100, 100}), false);

    CHECK(A.add({"Smeagol", 200, 100, 100}), true);

    CHECK(A.enchant("Frodo", "Frodo", 10, 1, 1), true);
    CHECK(A.enchant("Sam", "Frodo", -1000, 1, 1), true); // empty range
    CHECK(A.enchant("Bilbo", "Bungo", 1000, 0, 0), true); // empty range

    if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
        CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), false);

    CHECK(A.enchant("Frodo", "Sam", 1, 0, 0), true);
    CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), true);

    CHECK(A.stats("Gandalf"), std::optional<Hobbit>{});
    CHECK(A.stats("Frodo"), std::optional(Hobbit("Frodo", 151, 12, 5)));
    CHECK(A.stats("Merry"), std::optional(Hobbit("Merry", 1, 16, -2)));

    check_army(A, {
            {"Frodo", 151, 12, 5},
            {"Merry", 1, 16, -2},
            {"Pippin", 1, 13, 3},
            {"Sam", 21, 11, 5},
            {"Smeagol", 200, 100, 100},
    }, ok, fail);
}

int main() {
    int ok = 0, fail = 0;
    test1(ok, fail);

    if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
    else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif


