#pragma once

#include <utility>


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

        Span(const Span& other)
            : m_Data(other.GetData()), m_Size(other.GetSize())
        {

        }

        const std::size_t GetSize() const { return m_Size; }
        const T* GetData() const { return m_Data; }
        


    private:
        const T* m_Data;
        std::size_t m_Size;
    };


    struct DataBlock 
    {
        DataBlock()
            : span(Span<const uint8_t>(nullptr, 0)), data() {}

        DataBlock(const void* ptr, std::size_t byteSize)
        {
            // Allocate the vector and copy the data
            data.resize(byteSize);
            std::memcpy(data.data(), ptr, byteSize);

            // Initialize the span to cover the vector
            span = Span<const uint8_t>(data.data(), byteSize);
        }

        DataBlock(DataBlock&& other) noexcept
            : data(std::move(other.data)), span(std::move(other.span))
        {
            other.span = Span<const uint8_t>(nullptr, 0);
        }

        DataBlock& operator=(DataBlock&& other) noexcept
        {
            if (this != &other) 
            {
                data = std::move(other.data);
                span = Span<const uint8_t>(data.data(), data.size());

                // Leave other in a valid state
                other.span = Span<const uint8_t>(nullptr, 0);
            }
            return *this;
        }



        std::vector<uint8_t> data;
        Span<const uint8_t> span;

        
    };

}
