#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <stdexcept>


template<typename T_>
class CMatrix2 {
public:
    CMatrix2(const int &xFrom, const int &xTo, const int &yFrom, const int &yTo) : m_ColMin(xFrom), m_ColMax(xTo), m_RowMin(yFrom), m_RowMax(yTo)
    {
        m_R = m_RowMax - m_RowMin + 1;
        m_C = m_ColMax - m_ColMin + 1;

        m_data = new T_[m_R * m_C];
    }

    CMatrix2(const CMatrix2 &other) : m_ColMin(other.m_ColMin), m_ColMax(other.m_ColMax), m_RowMin(other.m_RowMin), m_RowMax(other.m_RowMax),
    m_R(other.m_R), m_C(other.m_C), m_data(new T_[m_R * m_C])
    {
        for (size_t i = 0; i < (size_t)m_R * m_C; ++i) {
            m_data[i] = other.m_data[i];
        }
    }

    CMatrix2 &operator=(const CMatrix2 &other) {
        if (this != &other) {
            CMatrix2 tmp = other;

            std::swap(m_RowMin, tmp.m_RowMin);
            std::swap(m_RowMax, tmp.m_RowMax);
            std::swap(m_ColMin, tmp.m_ColMin);
            std::swap(m_ColMax, tmp.m_ColMax);
            std::swap(m_R, tmp.m_R);
            std::swap(m_C, tmp.m_C);
            std::swap(m_data, tmp.m_data);
        }

        return *this;
    }

    ~CMatrix2() {
        delete [] m_data;
    }

    bool operator==(const CMatrix2 &other) const {
        if (m_RowMin != other.m_RowMin || m_RowMax != other.m_RowMax ||
        m_ColMin != other.m_ColMin || m_ColMax != other.m_ColMax) {
            return false;
        }

        for (size_t i = 0; i < (size_t)m_R * m_C; ++i) {
            if (m_data[i] != other.m_data[i]) return false;
        }

        return true;
    }

    class Matrix1 {
        T_ * m_data;
        int m_ColMin, m_ColMax;
    public:
        Matrix1(T_ * data, int cMin, int cMax) : m_data(data), m_ColMin(cMin), m_ColMax(cMax) {}

        T_ & operator[] (int c) {
            if (c < m_ColMin || c > m_ColMax)
                throw std::out_of_range("Column index out of bounds");

            return m_data[c - m_ColMin];
        }

        const T_ & operator[] (int c) const{
            if (c < m_ColMin || c > m_ColMax)
                throw std::out_of_range("Column index out of bounds");

            return m_data[c - m_ColMin];
        }
    };

    Matrix1 operator[](int r) {
        if (r < m_RowMin || r > m_RowMax)
            throw std::out_of_range("Row index out of bounds");

        int row = r - m_RowMin;

        T_ * data = &m_data[row * m_C];

        return Matrix1(data, m_ColMin, m_ColMax);
    }

private:
    int m_ColMin, m_ColMax;
    int m_RowMin, m_RowMax;
    int m_R, m_C;
    T_ * m_data;
};

int main() {
    // Integer test
    CMatrix2<int> m1(-2, 2, -2, 2);
    assert(( m1[-2][-2] = 5 ) == 5);
    assert(( m1[2][2] = 10 ) == 10);


    try {
        int x = m1[3][3];
    } catch (const std::out_of_range &e) {
        std::cout << e.what() << std::endl;
    }

    // Double test
    CMatrix2<double> m2(-2, 2, -2, 2);
    assert(( m2[-2][-2] = 5.5 ) == 5.5);
    assert(( m2[2][2] = 10.5 ) == 10.5);

    try {
        double x = m2[3][3];
    } catch (const std::out_of_range &e) {
        std::cout << e.what() << std::endl;
    }

    // String test
    CMatrix2<std::string> m3(-2, 2, -2, 2);
    assert(( m3[-2][-2] = "test1" ) == "test1");
    assert(( m3[2][2] = "test2" ) == "test2");

    try {
        std::string x = m3[3][3];
    } catch (const std::out_of_range &e) {
        std::cout << e.what() << std::endl;
    }

    // Test copy constructor and equality operator
    CMatrix2<int> m4(-2, 2, -2, 2);
    m4[-2][-2] = 5;
    m4[2][2] = 10;
    CMatrix2<int> m5(m4);
    assert(m4 == m5);

    // Test assignment operator and equality operator
    CMatrix2<int> m6(-2, 2, -2, 2);
    m6[-2][-2] = 5;
    m6[2][2] = 10;
    CMatrix2<int> m7 = m6;
    assert(m6 == m7);

    return 0;
}

