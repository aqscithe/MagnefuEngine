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

        const std::size_t GetSize() const { return m_Size; }
        const T* GetData() const { return m_Data; }


    private:
        const T* m_Data;
        std::size_t m_Size;
    };


    struct DataBlock 
    {
        DataBlock()
            : span(Span<const uint8_t>()), data() {}

        DataBlock(const void* ptr, std::size_t byteSize)
        {
            // Allocate the vector and copy the data
            data.resize(byteSize);
            std::memcpy(data.data(), ptr, byteSize);

            // Initialize the span to cover the vector
            span = Span<const uint8_t>(data.data(), byteSize);
        }


        std::vector<uint8_t> data;
        Span<const uint8_t> span;

        
    };

}
