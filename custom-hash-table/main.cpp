#include <string>
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>

struct TItem
{
    TItem(std::string key, std::string val, TItem *nextHash, TItem *nextOrd)
            : m_Key(key), m_Val(val), m_NextHash(nextHash), m_NextOrder(nextOrd){}

    std::string m_Key, m_Val;
    TItem *m_NextHash, *m_NextOrder;
};

class CHash
{
public:
    CHash(int m) : m_Table(NULL), m_Size(m), m_FirstOrder(NULL), m_LastOrder(NULL)
    {
        m_Table = new TItem *[m];
        for (int i = 0; i < m; i++)
            m_Table[i] = NULL;
    }

    ~CHash()
    {
        delete [] m_Table;

        auto curr = m_FirstOrder;

        while (curr) {
            auto temp = curr->m_NextOrder;
            delete curr;
            curr = temp;
        }
    }

    CHash(const CHash & oth) : CHash(oth.m_Size) {
        auto curr = oth.m_FirstOrder;

        while (curr) {
            auto idx = hashFn(curr->m_Key, m_Size);

            auto newNode = new TItem(curr->m_Key, curr->m_Val, m_Table[idx], nullptr);

            if (!m_FirstOrder)
                m_FirstOrder = newNode;

            if (m_LastOrder)
                m_LastOrder->m_NextOrder = newNode;

            m_LastOrder = newNode;

            m_Table[idx] = newNode;

            curr = curr->m_NextOrder;
        }
    }

    CHash & operator=(CHash & oth) {
        if (this == &oth) return *this;

        CHash tmp = oth;

        std::swap(*m_Table, *tmp.m_Table);
        std::swap(m_Size, tmp.m_Size);
        std::swap(m_FirstOrder, tmp.m_FirstOrder);
        std::swap(m_LastOrder, tmp.m_LastOrder);

        return *this;
    }

    bool Insert(std::string key, std::string val)
    {
        if (IsSet(key)) return false;

        auto idx = hashFn(key, m_Size);

        auto newNode = new TItem(key, val, m_Table[idx], nullptr);

        if (!m_FirstOrder)
            m_FirstOrder = newNode;

        if (m_LastOrder)
            m_LastOrder->m_NextOrder = newNode;

        m_LastOrder = newNode;

        m_Table[idx] = newNode;

        return true;
    }

    // cannot iterate through all items in table (would be too slow)
    bool IsSet(std::string key)
    {
        auto idx = hashFn(key, m_Size);

        auto curr = m_Table[idx];

        while (curr) {
            if (curr->m_Key == key) return true;

            curr = curr->m_NextHash;
        }

        return false;
    }

    friend std::ostream & operator<<(std::ostream & os, const CHash & table) {
        auto curr = table.m_FirstOrder;

        while (curr) {
            os << curr->m_Key << " => " << curr->m_Val;

            if (curr->m_NextOrder)
                os << ", ";

            curr = curr->m_NextOrder;
        }

        return os;
    }

    TItem **m_Table;
    unsigned int m_Size;
    TItem *m_FirstOrder, *m_LastOrder;
private:
    unsigned int hashFn(std::string &str, size_t mod)
    {
        std::hash<std::string> hash_fn;
        return hash_fn(str) % mod;
    }
};

int main(int argc, char **argv)
{
    std::ostringstream oss;
    CHash hashtable(4);
    assert(hashtable.Insert("h1", "car") == true);
    assert(hashtable.Insert("h1", "phone") == false);
    assert(hashtable.Insert("h2", "field") == true);
    assert(hashtable.Insert("h3", "house") == true);
    assert(hashtable.Insert("h4", "tree") == true);
    
    assert(hashtable.m_FirstOrder->m_Key == "h1"
           && hashtable.m_Table[0]->m_Key == "h3" && hashtable.m_Table[0]->m_Val == "house"
           && hashtable.m_Table[1]->m_Key == "h1" && hashtable.m_Table[1]->m_Val == "car"
           && hashtable.m_Table[2] == nullptr
           && hashtable.m_Table[3]->m_Key == "h4" && hashtable.m_Table[3]->m_Val == "tree"
           && hashtable.m_Table[0]->m_NextHash->m_Key == "h2" && hashtable.m_Table[0]->m_NextHash->m_Val == "field"
           && hashtable.m_Table[0]->m_NextHash->m_NextHash == nullptr
           && hashtable.m_Table[1]->m_NextHash == nullptr
           && hashtable.m_Table[3]->m_NextHash == nullptr
    );


    oss << hashtable;
    assert(oss.str() == "h1 => car, h2 => field, h3 => house, h4 => tree");

    {
        CHash a(4);

        assert(a.Insert("h1", "car") == true);
        assert(a.Insert("h2", "field") == true);
        assert(a.Insert("h3", "house") == true);

        //
        // COPY CONSTRUCTOR
        //
        CHash b(a);

        // original content copied
        std::ostringstream oss1;
        oss1 << b;
        assert(oss1.str() == "h1 => car, h2 => field, h3 => house");

        // deep copy check
        assert(b.m_FirstOrder != a.m_FirstOrder);
        assert(b.m_Table != a.m_Table);

        // values preserved
        assert(b.IsSet("h1") == true);
        assert(b.IsSet("h2") == true);
        assert(b.IsSet("h3") == true);

        // modifying copy must not affect original
        assert(b.Insert("h4", "tree") == true);

        std::ostringstream oss2, oss3;
        oss2 << a;
        oss3 << b;

        assert(oss2.str() == "h1 => car, h2 => field, h3 => house");
        assert(oss3.str() == "h1 => car, h2 => field, h3 => house, h4 => tree");

        //
        // ASSIGNMENT OPERATOR
        //
        CHash c(10);

        assert(c.Insert("x1", "apple") == true);
        assert(c.Insert("x2", "banana") == true);

        c = a;

        // content copied
        std::ostringstream oss4;
        oss4 << c;

        assert(oss4.str() == "h1 => car, h2 => field, h3 => house");

        // old content removed
        assert(c.IsSet("x1") == false);
        assert(c.IsSet("x2") == false);

        // deep copy check
        assert(c.m_FirstOrder != a.m_FirstOrder);
        assert(c.m_Table != a.m_Table);

        // modifying assigned object must not affect source
        assert(c.Insert("h4", "tree") == true);

        std::ostringstream oss5, oss6;
        oss5 << a;
        oss6 << c;

        assert(oss5.str() == "h1 => car, h2 => field, h3 => house");
        assert(oss6.str() == "h1 => car, h2 => field, h3 => house, h4 => tree");

        //
        // SELF ASSIGNMENT
        //
        c = c;

        std::ostringstream oss7;
        oss7 << c;

        assert(oss7.str() == "h1 => car, h2 => field, h3 => house, h4 => tree");

        //
        // EMPTY TABLE COPY
        //
        CHash empty1(5);
        CHash empty2(empty1);

        std::ostringstream oss8;
        oss8 << empty2;

        assert(oss8.str() == "");

        //
        // EMPTY TABLE ASSIGNMENT
        //
        CHash d(3);

        assert(d.Insert("abc", "xyz") == true);

        d = empty1;

        std::ostringstream oss9;
        oss9 << d;

        assert(oss9.str() == "");
        assert(d.m_FirstOrder == nullptr);
    }
    return 0;
}

