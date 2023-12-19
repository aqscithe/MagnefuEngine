#pragma once

// -- Core  Includes ----------- //
#include "PoolAllocator.h"


namespace Magnefu
{
    template<typename T>
    class LinkedListAlloc {
    public:
        struct Node {
            T Data;
            Node* Next;
        };

        /*LinkedListAlloc(PoolAllocator& allocator) : 
            m_Allocator(allocator), 
            m_Head(nullptr)
        {
        }*/

        ~LinkedListAlloc() {
            Node* node = m_Head;
            while (node) {
                Node* Next = node->Next;
                m_Allocator.Deallocate(node);
                node = Next;
            }
        }

        void Init(PoolAllocator& allocator)
        {
            m_Allocator = allocator;
        }

        T& push_front(const T& value) {
            Node* node = static_cast<Node*>(m_Allocator.Allocate());
            node->Data = value;
            node->Next = m_Head;
            m_Head = node;
            return node->Data;
        }

        T& front() {
            return m_Head->Data;
        }

        const T& front() const {
            return m_Head->Data;
        }

        void pop_front() {
            Node* node = m_Head;
            m_Head = node->Next;
            m_Allocator.Deallocate(node);
        }

        bool empty() const {
            return m_Head == nullptr;
        }

    private:
        PoolAllocator& m_Allocator;
        Node* m_Head;              
    };
}

