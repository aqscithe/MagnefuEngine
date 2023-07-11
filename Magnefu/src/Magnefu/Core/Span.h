#pragma once


namespace Magnefu
{
    template <typename T>
    class Span
    {
    public:
        Span()
            : m_Data(nullptr), m_Size(0) {}

        Span(const std::initializer_list<T>&& initList)
            : m_Data(initList.begin()), m_Size(initList.size()) {}

        Span(const T* data, std::size_t size)
            : m_Data(data), m_Size(size) {}

        std::size_t GetSize() { return m_Size; }
        const T* GetData() { return m_Data; }

    private:
        const T* m_Data;
        std::size_t m_Size;
    };
}