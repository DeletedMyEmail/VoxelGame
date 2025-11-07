#pragma once
#include <array>
#include <cstddef>

template<typename T, size_t N>
class RingBuffer
{
public:
    class RingIterator
    {
    public:
        RingIterator(const RingBuffer* rb, size_t pos);
        T& operator*();
        RingIterator& operator++();
        bool operator==(const RingIterator& other);
        bool operator!=(const RingIterator& other);
    private:
        const RingBuffer* m_Buffer;
        size_t m_Pos;
    };

    void push(const T& value);
    RingIterator begin();
    RingIterator end();
    size_t size() const { return m_Count; }
    bool empty() const { return m_Count == 0; }
private:
    std::array<T, N> m_Buffer;
    size_t m_Head = 0;
    size_t m_Count = 0;
};

template<typename T, size_t N>
RingBuffer<T, N>::RingIterator::RingIterator(const RingBuffer* rb, const size_t pos) : m_Buffer(rb), m_Pos(pos) {}

template<typename T, size_t N>
T& RingBuffer<T, N>::RingIterator::operator*()
{
    return const_cast<T&>(m_Buffer->m_Buffer[(m_Buffer->m_Head + m_Pos) % N]);
}

template<typename T, size_t N>
RingBuffer<T, N>::RingIterator& RingBuffer<T, N>::RingIterator::operator++()
{
    ++m_Pos;
    return *this;
}

template<typename T, size_t N>
bool RingBuffer<T, N>::RingIterator::operator==(const RingIterator& other)
{
    return m_Buffer == other.m_Buffer && m_Pos == other.m_Pos;
}

template<typename T, size_t N>
bool RingBuffer<T, N>::RingIterator::operator!=(const RingIterator& other)
{
    return !(*this == other);
}

template<typename T, size_t N>
void RingBuffer<T, N>::push(const T& value)
{
    m_Buffer[(m_Head + m_Count) % N] = value;
    if (m_Count < N)
        ++m_Count;
    else
        m_Head = (m_Head + 1) % N;
}

template<typename T, size_t N>
RingBuffer<T, N>::RingIterator RingBuffer<T, N>::begin()
{
    return RingIterator(this, 0);
}

template<typename T, size_t N>
RingBuffer<T, N>::RingIterator RingBuffer<T, N>::end()
{
    return RingIterator(this, m_Count);
}