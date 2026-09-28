#ifndef __SOMETEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <algorithm>
#include <memory>
#endif

struct Company{
    std::string m_id;
    std::string m_name;
    std::string m_address;

    std::string m_nameLower;
    std::string m_addressLower;

    unsigned int m_income = 0;

    Company(const std::string & id, const std::string & name, const std::string & addr,
            const std::string & nameLower, const std::string & addrLower)
    {
        m_id = id;
        m_name = name;
        m_address = addr;
        m_nameLower = nameLower;
        m_addressLower = addrLower;
    }

    // case-insensitive comparator used by std::lower_bound for searching by (name, address)
    static bool cmpNameAddress(const Company & a, const std::pair<std::string, std::string> & key)
    {
        if(a.m_nameLower != key.first)
            return a.m_nameLower < key.first;

        return a.m_addressLower < key.second;
    }

    // comparator used by std::lower_bound for searching by taxID
    static bool cmpID(const Company & a, const std::string & key)
    {
        return a.m_id < key;
    }
};

class CVATRegister
{
public:
    CVATRegister   () = default;
    ~CVATRegister  () = default;
    bool          newCompany     ( const std::string    & name,
                                   const std::string    & addr,
                                   const std::string    & taxID );
    bool          cancelCompany  ( const std::string    & name,
                                   const std::string    & addr );
    bool          cancelCompany  ( const std::string    & taxID );
    bool          invoice        ( const std::string    & taxID,
                                   unsigned int           amount );
    bool          invoice        ( const std::string    & name,
                                   const std::string    & addr,
                                   unsigned int           amount );
    bool          auditCompany   ( const std::string    & name,
                                   const std::string    & addr,
                                   unsigned int         & sumIncome ) const;
    bool          auditCompany   ( const std::string    & taxID,
                                   unsigned int         & sumIncome ) const;
    bool          firstCompany   ( std::string          & name,
                                   std::string          & addr ) const;
    bool          nextCompany    ( std::string          & name,
                                   std::string          & addr ) const;
    unsigned int  medianInvoice  () const;
private:
    std::vector<Company> m_nameAddress; // companies sorted by (name, address)
    std::vector<Company> m_id; // companies sorted by id

    std::vector<unsigned int> m_lowerHalf; // Max-Heap: stores the smaller half of invoices
    std::vector<unsigned int> m_upperHalf; // Min-Heap: stores the larger half of invoices (Invariant: median is always at index 0)

    void addIncome(unsigned int amount);
    void rebalanceHeaps();

    std::pair<size_t, bool> findByNameAddress(const std::string & nameLower, const std::string & addrLower) const;
    std::pair<size_t, bool> findByID(const std::string & id) const;

    static std::string toLowerCase(std::string str);
};

// Adds a new invoice amount and rebalances(if needed) heaps in O(log N)
void CVATRegister::addIncome(unsigned int amount)
{
    // insert the new amount into the correct half
    if(m_upperHalf.empty() || amount >= m_upperHalf.front())
    {
        m_upperHalf.push_back(amount);
        std::push_heap(m_upperHalf.begin(), m_upperHalf.end(), std::greater<>());
    }
    else
    {
        m_lowerHalf.push_back(amount);
        std::push_heap(m_lowerHalf.begin(), m_lowerHalf.end());
    }

    // rebalance heaps to keep size difference <= 1
    rebalanceHeaps();
}

// rebalance heaps to maintain the rule: size(lower) == size(upper) or size(upper) == size(lower) + 1
void CVATRegister::rebalanceHeaps()
{
    if(m_lowerHalf.size() > m_upperHalf.size())
    {
        // lowerHalf is too big, we need to move its largest element to the upperHalf
        m_upperHalf.push_back(m_lowerHalf.front());
        std::push_heap(m_upperHalf.begin(), m_upperHalf.end(), std::greater<>());

        std::pop_heap(m_lowerHalf.begin(), m_lowerHalf.end());
        m_lowerHalf.pop_back();
    }
    else if(m_upperHalf.size() > m_lowerHalf.size() + 1)
    {
        // upperHalf is too big, we need to move its smallest element to the lowerHalf
        m_lowerHalf.push_back(m_upperHalf.front());
        std::push_heap(m_lowerHalf.begin(), m_lowerHalf.end());

        std::pop_heap(m_upperHalf.begin(), m_upperHalf.end(), std::greater<>());
        m_upperHalf.pop_back();
    }
}

std::string CVATRegister::toLowerCase(std::string str)
{
    for(char & c : str)
        c = static_cast<char>(std::tolower(c));

    return str;
}

// binary search in vector sorted by (name, address). returns the index where the record exists or should be inserted
std::pair<size_t, bool> CVATRegister::findByNameAddress(const std::string &nameLower, const std::string &addrLower) const
{
    const auto key = std::make_pair(nameLower, addrLower);

    const auto it = std::lower_bound(m_nameAddress.begin(), m_nameAddress.end(), key, Company::cmpNameAddress);

    const size_t idx = it - m_nameAddress.begin();

    const bool isFound = (it != m_nameAddress.end() && it->m_nameLower == nameLower && it->m_addressLower == addrLower);

    return {idx, isFound};
}

// binary search in vector sorted by taxID. returns the index where the record exists or should be inserted
std::pair<size_t, bool> CVATRegister::findByID(const std::string &id) const
{
    const auto it = std::lower_bound(m_id.begin(), m_id.end(), id,Company::cmpID);

    const size_t idx = it - m_id.begin();

    const bool isFound = (it != m_id.end() && it->m_id == id);

    return {idx, isFound};
}

bool CVATRegister::newCompany(const std::string &name, const std::string &addr, const std::string &taxID)
{
    const auto nameToLower = toLowerCase(name);
    const auto addrToLower = toLowerCase(addr);

    const auto [idxNA, foundNA] = findByNameAddress(nameToLower, addrToLower);

    if(foundNA) return false;

    const auto [idxID, foundID] = findByID(taxID);

    if(foundID) return false;

    const Company newRecord = Company(taxID, name, addr, nameToLower, addrToLower);

    m_nameAddress.insert(m_nameAddress.begin() + idxNA, newRecord);
    m_id.insert(m_id.begin() + idxID, newRecord);

    return true;
}

bool CVATRegister::cancelCompany(const std::string &name, const std::string &addr)
{
    const auto nameToLower = toLowerCase(name);
    const auto addrToLower = toLowerCase(addr);

    const auto [idxNA, foundNA] = findByNameAddress(nameToLower, addrToLower);

    if(!foundNA) return false;

    const auto & company = m_nameAddress[idxNA];

    const auto [idxID, foundID] = findByID(company.m_id);

    if(!foundID) return false;

    m_nameAddress.erase(m_nameAddress.begin() + idxNA);
    m_id.erase(m_id.begin() + idxID);

    return true;
}

bool CVATRegister::cancelCompany(const std::string &taxID)
{
    const auto [idxID, foundID] = findByID(taxID);

    if(!foundID) return false;

    const auto & company = m_id[idxID];

    const auto [idxNA, foundNA] = findByNameAddress(company.m_nameLower, company.m_addressLower);

    if(!foundNA) return false;

    m_nameAddress.erase(m_nameAddress.begin() + idxNA);
    m_id.erase(m_id.begin() + idxID);

    return true;
}

bool CVATRegister::invoice(const std::string &taxID, unsigned int amount)
{
    const auto [idxID, foundID] = findByID(taxID);

    if(!foundID) return false;

    m_id[idxID].m_income += amount;

    const auto & company = m_id[idxID];

    const auto [idxNA, foundNA] = findByNameAddress(company.m_nameLower, company.m_addressLower);

    m_nameAddress[idxNA].m_income += amount;

    addIncome(amount);

    return true;
}

bool CVATRegister::invoice(const std::string &name, const std::string &addr, unsigned int amount)
{
    const auto nameToLower = toLowerCase(name);
    const auto addrToLower = toLowerCase(addr);

    const auto [idxNA, foundNA] = findByNameAddress(nameToLower, addrToLower);

    if(!foundNA) return false;

    m_nameAddress[idxNA].m_income += amount;

    const auto & company = m_nameAddress[idxNA];

    const auto [idxID, foundID] = findByID(company.m_id);

    m_id[idxID].m_income += amount;

    addIncome(amount);

    return true;
}

bool CVATRegister::auditCompany(const std::string &name, const std::string &addr, unsigned int &sumIncome) const
{
    const auto nameToLower = toLowerCase(name);
    const auto addrToLower = toLowerCase(addr);

    const auto [idxNA, foundNA] = findByNameAddress(nameToLower, addrToLower);

    if(!foundNA) return false;

    sumIncome = m_nameAddress[idxNA].m_income;

    return true;
}

bool CVATRegister::auditCompany(const std::string &taxID, unsigned int &sumIncome) const
{
    const auto [idxID, foundID] = findByID(taxID);

    if(!foundID) return false;

    sumIncome = m_id[idxID].m_income;

    return true;
}

bool CVATRegister::firstCompany(std::string &name, std::string &addr) const
{
    if(m_nameAddress.empty()) return false;

    name = m_nameAddress[0].m_name;
    addr = m_nameAddress[0].m_address;

    return true;
}

bool CVATRegister::nextCompany(std::string &name, std::string &addr) const
{
    const auto nameToLower = toLowerCase(name);
    const auto addrToLower = toLowerCase(addr);

    auto [idxNA, foundNA] = findByNameAddress(nameToLower, addrToLower);

    if(!foundNA) return false;

    if(++idxNA >= m_nameAddress.size()) return false;

    name = m_nameAddress[idxNA].m_name;
    addr = m_nameAddress[idxNA].m_address;

    return true;
}

unsigned int CVATRegister::medianInvoice() const
{
    if(m_lowerHalf.empty() && m_upperHalf.empty()) return 0;

    return m_upperHalf.front();
}

#ifndef __SOMETEST__
int               main           ()
{
    std::string name, addr;
    unsigned int sumIncome;

    CVATRegister b1;
    assert ( b1 . newCompany ( "ACME", "Thakurova", "666/666" ) );
    assert ( b1 . newCompany ( "ACME", "Kolejni", "666/666/666" ) );
    assert ( b1 . newCompany ( "Dummy", "Thakurova", "123456" ) );
    assert ( b1 . invoice ( "666/666", 2000 ) );
    assert ( b1 . medianInvoice () == 2000 );
    assert ( b1 . invoice ( "666/666/666", 3000 ) );
    assert ( b1 . medianInvoice () == 3000 );
    assert ( b1 . invoice ( "123456", 4000 ) );
    assert ( b1 . medianInvoice () == 3000 );
    assert ( b1 . invoice ( "aCmE", "Kolejni", 5000 ) );
    assert ( b1 . medianInvoice () == 4000 );
    assert ( b1 . auditCompany ( "ACME", "Kolejni", sumIncome ) && sumIncome == 8000 );
    assert ( b1 . auditCompany ( "123456", sumIncome ) && sumIncome == 4000 );
    assert ( b1 . firstCompany ( name, addr ) && name == "ACME" && addr == "Kolejni" );
    assert ( b1 . nextCompany ( name, addr ) && name == "ACME" && addr == "Thakurova" );
    assert ( b1 . nextCompany ( name, addr ) && name == "Dummy" && addr == "Thakurova" );
    assert ( ! b1 . nextCompany ( name, addr ) );
    assert ( b1 . cancelCompany ( "ACME", "KoLeJnI" ) );
    assert ( b1 . medianInvoice () == 4000 );
    assert ( b1 . cancelCompany ( "666/666" ) );
    assert ( b1 . medianInvoice () == 4000 );
    assert ( b1 . invoice ( "123456", 100 ) );
    assert ( b1 . medianInvoice () == 3000 );
    assert ( b1 . invoice ( "123456", 300 ) );
    assert ( b1 . medianInvoice () == 3000 );
    assert ( b1 . invoice ( "123456", 200 ) );
    assert ( b1 . medianInvoice () == 2000 );
    assert ( b1 . invoice ( "123456", 230 ) );
    assert ( b1 . medianInvoice () == 2000 );
    assert ( b1 . invoice ( "123456", 830 ) );
    assert ( b1 . medianInvoice () == 830 );
    assert ( b1 . invoice ( "123456", 1830 ) );
    assert ( b1 . medianInvoice () == 1830 );
    assert ( b1 . invoice ( "123456", 2830 ) );
    assert ( b1 . medianInvoice () == 1830 );
    assert ( b1 . invoice ( "123456", 2830 ) );
    assert ( b1 . medianInvoice () == 2000 );
    assert ( b1 . invoice ( "123456", 3200 ) );
    assert ( b1 . medianInvoice () == 2000 );
    assert ( b1 . firstCompany ( name, addr ) && name == "Dummy" && addr == "Thakurova" );
    assert ( ! b1 . nextCompany ( name, addr ) );
    assert ( b1 . cancelCompany ( "123456" ) );
    assert ( ! b1 . firstCompany ( name, addr ) );

    CVATRegister b2;
    assert ( b2 . newCompany ( "ACME", "Kolejni", "abcdef" ) );
    assert ( b2 . newCompany ( "Dummy", "Kolejni", "123456" ) );
    assert ( ! b2 . newCompany ( "AcMe", "kOlEjNi", "1234" ) );
    assert ( b2 . newCompany ( "Dummy", "Thakurova", "ABCDEF" ) );
    assert ( b2 . medianInvoice () == 0 );
    assert ( b2 . invoice ( "ABCDEF", 1000 ) );
    assert ( b2 . medianInvoice () == 1000 );
    assert ( b2 . invoice ( "abcdef", 2000 ) );
    assert ( b2 . medianInvoice () == 2000 );
    assert ( b2 . invoice ( "aCMe", "kOlEjNi", 3000 ) );
    assert ( b2 . medianInvoice () == 2000 );
    assert ( ! b2 . invoice ( "1234567", 100 ) );
    assert ( ! b2 . invoice ( "ACE", "Kolejni", 100 ) );
    assert ( ! b2 . invoice ( "ACME", "Thakurova", 100 ) );
    assert ( ! b2 . auditCompany ( "1234567", sumIncome ) );
    assert ( ! b2 . auditCompany ( "ACE", "Kolejni", sumIncome ) );
    assert ( ! b2 . auditCompany ( "ACME", "Thakurova", sumIncome ) );
    assert ( ! b2 . cancelCompany ( "1234567" ) );
    assert ( ! b2 . cancelCompany ( "ACE", "Kolejni" ) );
    assert ( ! b2 . cancelCompany ( "ACME", "Thakurova" ) );
    assert ( b2 . cancelCompany ( "abcdef" ) );
    assert ( b2 . medianInvoice () == 2000 );
    assert ( ! b2 . cancelCompany ( "abcdef" ) );
    assert ( b2 . newCompany ( "ACME", "Kolejni", "abcdef" ) );
    assert ( b2 . cancelCompany ( "ACME", "Kolejni" ) );
    assert ( ! b2 . cancelCompany ( "ACME", "Kolejni" ) );

    return EXIT_SUCCESS;
}
#endif
