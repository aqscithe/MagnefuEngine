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
            : m_Data(data), m_Size(size) 
        {
            //MF_CORE_ASSERT(reinterpret_cast<uintptr_t>(m_Data) % 32 == 0, "data not 32-byte aligned");
        }

        std::size_t GetSize() { return m_Size; }
        const T* GetData() const { return m_Data; }

        void CheckAlign() { /*MF_CORE_ASSERT(reinterpret_cast<uintptr_t>(m_Data) % 32 == 0, "data not 32-byte aligned");*/ }

    private:
        const T* m_Data;
        std::size_t m_Size;
    };
}
