#pragma once


namespace Magnefu
{
    template <typename T>
    class VectorAllocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        template <typename U>
        struct rebind {
            using other = VectorAllocator<U>;
        };

        VectorAllocator() noexcept {}

        template <typename U> 
        VectorAllocator(const VectorAllocator<U>&) noexcept {}

        pointer allocate(size_type n) {
            // Allocate memory from the stack
            pointer p = reinterpret_cast<pointer>(&m_data[m_index]);
            m_index += n;
            return p;
        }

        void deallocate(pointer p, std::size_t n) noexcept {
            // The stack allocator does not free memory individually
        }


    private:
        static constexpr std::size_t size = 1024; // The size of the stack in bytes
        alignas(alignof(T)) char m_data[size]; // The stack memory
        std::size_t m_index = 0; // The current index of the stack
    };
}