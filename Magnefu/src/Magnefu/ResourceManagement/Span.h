#pragma once


namespace Magnefu
{
    template <typename T>
    class Span
    {
    public:
        Span(const std::initializer_list<T>&& initList)
            : m_Data(initList.begin()), m_Size(initList.size()) {}

        // Other constructors and member functions...

    private:
        const T* m_Data;
        std::size_t m_Size;
    };
}
