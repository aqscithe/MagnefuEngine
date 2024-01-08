#pragma once


// -- Core Includes ---------------------------------- //
#include "MemoryAllocation/Memory.hpp"
#include "Platform.hpp"
#include "Assertions.h"

// -- other Includes -------------- //

#include <utility>



namespace Magnefu
{

	// -- Data Structures ----------------------------------- //

	template<typename T>
	struct Array
	{
		// -- Constructors --------------------------- //

		Array();
		Array(Allocator* allocator, u32 initialCapacity, u32 initialSize = 0);
		Array(Allocator* allocator, T* begin, T* end);

		// Enables the following initialization: 
		// Array<int> nums = { 2, 4, 1};
		/*template<typename... Args>
		Array(Args&&... args)
		{
			u32 count = sizeof...(Args);
			init(&MemoryService::instance()->systemAllocator, count, count);

			initializeElements(std::forward<Args>(args)...);
		}*/

		~Array();

		// -- Methods -- //

		void		init(Allocator* allocator, u32 initialCapacity, u32 initialSize = 0);
		void		shutdown();

		void		push(const T& element);
		T&			push_use();

		void		pop();
		void		delete_swap(u32 index);

		T&			operator[](u32 index);
		const T&	operator[](u32 index) const;

		void		clear();
		void		set_size(u32 newSize);
		void		set_capacity(u32 newCapacity);
		void		grow(u32 newCapacity);

		T&			back();
		const T&	back() const;

		T&			front();
		const T&	front() const;

		T*			begin() { return data; }
		const T*	begin() const { return data; }

		T*			end();
		const T*	end() const;

		T*			raw();
		const T*	raw() const;

		bool		empty() { return size <= 0; }

		u32			get_index(const T& value);
		u32			count() const;
		u32         size_in_bytes() const;
		u32         capacity_in_bytes() const;

		//template<typename... Args>
		//void		initializeElements(Args&&... args)
		//{
		//	int index = 0;
		//	(..., (new(&data[index++]) T(std::forward<Args>(args)))); // Placement new for each argument
		//}


		// -- Members -- //


		T*			data;
		u32         size;       // Occupied size
		u32         capacity;   // Allocated capacity
		Allocator*	allocator;
	};

	// Array View
	// View over a contiguous memory block
	template <typename T>
	struct ArrayView
	{
		ArrayView(T* data, u32 size);

		// -- Methods -- //

		void		set(T* data, u32 size);

		T&			operator[](u32 index);
		const T&	operator[](u32 index) const;

		T*			raw();
		const T*	raw() const;

		u32			count() const;

		// -- Members -- //


		T*			data;
		u32			size;
	};


	// -- Implementation ----------------------------------- //


	// -- Array Aligned -- //

	template<typename T>
	inline Array<T>::Array()
	{
		data = nullptr;
		capacity = 0;
		size = 0;
		allocator = &MemoryService::Instance()->systemAllocator;
	}

	template<typename T>
	inline Array<T>::Array(Allocator* allocator_, u32 initialCapacity, u32 initialSize)
	{
		init(allocator_, initialCapacity, initialSize);
	}

	template<typename T>
	inline Array<T>::Array(Allocator* allocator_, T* begin, T* end)
	{
		sizet initialSize = end - begin;
		MF_CORE_ASSERT((initialSize > 0), "End pointer comes before begin");

		//init(allocator_, initialSize, initialSize);

		allocator = allocator_;

		u32 initialCapacity = initialSize;
		u32 capacityBytes = initialCapacity * sizeof(T);
		T* new_data = (T*)allocator->allocate(capacityBytes, alignof(T));

		memoryCopy(new_data, begin, capacityBytes);
		
		data = new_data;
		capacity = initialCapacity;
		size = initialSize;
	}

	template<typename T>
	inline Array<T>::~Array()
	{
		shutdown();
	}

	template<typename T>
	inline void Array<T>::init(Allocator* allocator_, u32 initialCapacity, u32 initialSize)
	{
		data = nullptr;
		size = initialSize;
		capacity = 0;
		allocator = allocator_;

		if (initialCapacity > 0)
		{
			grow(initialCapacity);
		}

	}

	template<typename T>
	inline void Array<T>::shutdown()
	{
		if (capacity > 0)
		{
			allocator->deallocate(data);
		}

		data = nullptr;

		size = capacity = 0;
	}


	template<typename T>
	inline void Array<T>::push(const T& element)
	{
		if (size >= capacity)
		{
			grow(capacity + 1);
		}

		data[size++] = element;
	}

	template<typename T>
	inline T& Array<T>::push_use()
	{
		if (size >= capacity)
		{
			grow(capacity + 1);
		}

		size++;

		return back();
	}

	template<typename T>
	inline void Array<T>::pop()
	{
		MF_CORE_ASSERT((size > 0), "Empty array");
		size--;
	}

	template<typename T>
	inline u32 Array<T>::get_index(const T& value)
	{
		for (u32 index = 0; index < size; index++)
		{
			if (data[index] == value)
			{
				return index;
			}
		}

		return (u32)-1;
	}

	template<typename T>
	inline void Array<T>::delete_swap(u32 index)
	{
		MF_CORE_ASSERT((size > 0 && index < size), "Empty array and/or Index out of bounds");
		data[index] = data[--size];
	}

	template<typename T>
	inline T& Array<T>::operator [](u32 index) 
	{
		MF_CORE_ASSERT((index < size), "Index out of bounds");
		return data[index];
	}

	template<typename T>
	inline const T& Array<T>::operator [](u32 index) const 
	{
		MF_CORE_ASSERT((index < size), "Index out of bounds");
		return data[index];
	}

	template<typename T>
	inline void Array<T>::clear() 
	{
		size = 0;
	}

	template<typename T>
	inline void Array<T>::set_size(u32 newSize) 
	{
		if (newSize > capacity) 
		{
			grow(newSize);
		}
		size = newSize;
	}

	template<typename T>
	inline void Array<T>::set_capacity(u32 newCapacity) 
	{
		if (newCapacity > capacity) 
		{
			grow(newCapacity);
		}
	}

	template<typename T>
	inline void Array<T>::grow(u32 newCapacity)
	{
		if (newCapacity < capacity * 2)
		{
			newCapacity = capacity * 2;
		}
		else if (newCapacity < 4)
		{
			newCapacity = 4;
		}

		T* new_data = (T*)allocator->allocate(newCapacity * sizeof(T), alignof(T));
		if (capacity)
		{
			memoryCopy(new_data, data, capacity * sizeof(T));

			allocator->deallocate(data);
		}

		data = new_data;
		capacity = newCapacity;
	}

	template<typename T>
	inline T& Array<T>::back() 
	{
		MF_CORE_ASSERT(size, "Empty array");
		return data[size - 1];
	}

	template<typename T>
	inline const T& Array<T>::back() const 
	{
		MF_CORE_ASSERT(size, "Empty array");
		return data[size - 1];
	}

	template<typename T>
	inline T& Array<T>::front() 
	{
		MF_CORE_ASSERT(size, "Empty array");
		return data[0];
	}

	template<typename T>
	inline const T& Array<T>::front() const 
	{
		MF_CORE_ASSERT(size, "Empty array");
		return data[0];
	}

	template<typename T>
	inline T* Array<T>::end()
	{ 
		MF_CORE_ASSERT(size, "Empty array");
		return data + size; 
	}

	template<typename T>
	const T* Array<T>::end() const
	{ 
		MF_CORE_ASSERT(size, "Empty array");
		return data + size; 
	}

	template<typename T>
	inline T* Array<T>::raw()
	{
		return data;
	}

	template<typename T>
	inline const T* Array<T>::raw() const
	{
		return data;
	}

	template<typename T>
	inline u32 Array<T>::count() const
	{
		return size;
	}

	template<typename T>
	inline u32 Array<T>::size_in_bytes() const 
	{
		return size * sizeof(T);
	}

	template<typename T>
	inline u32 Array<T>::capacity_in_bytes() const 
	{
		return capacity * sizeof(T);
	}


	// -- ArrayView -- //

	template<typename T>
	inline ArrayView<T>::ArrayView(T* data_, u32 size_)
		: data(data_), size(size_) 
	{
	}

	template<typename T>
	inline void ArrayView<T>::set(T* data_, u32 size_) 
	{
		data = data_;
		size = size_;
	}

	template<typename T>
	inline T& ArrayView<T>::operator[](u32 index) 
	{
		MF_CORE_ASSERT(index < size, "Index out of bounds");
		return data[index];
	}

	template<typename T>
	inline const T& ArrayView<T>::operator[](u32 index) const 
	{
		MF_CORE_ASSERT(index < size, "Index out of bounds");
		return data[index];
	}

	template<typename T>
	inline T* ArrayView<T>::raw()
	{
		return data;
	}

	template<typename T>
	inline const T* ArrayView<T>::raw() const
	{
		return data;
	}

	template<typename T>
	inline u32 ArrayView<T>::count() const
	{
		return size;
	}

} // namespace Magnefu