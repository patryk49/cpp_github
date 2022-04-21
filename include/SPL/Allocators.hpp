#pragma once

#include "Utils.hpp"

namespace sp{ // BEGINING OF NAMESPACE ///////////////////////////////////////////////////////////



template<class T, class A>
SP_CSI Range<T> alloct(A &al, size_t size) noexcept{
	static_assert(alignof(T) <= A::Alignment, "this alignment is not supported by allocator");

	Memblock blk;
	if constexpr (A::Alignment)
		blk = alloc(al, size*sizeof(T));
	else
		blk = alloc(al, size*sizeof(T), alignof(T));
	return Range<T>{(T *)blk.ptr, blk.size / sizeof(T)};
}



// NULL ALLOCATOR
struct NullAllocator{
	constexpr static size_t Alignment = 0;
	constexpr static bool IsAware = true;
	constexpr static bool HasMassFree = true;
};
SP_CSI bool contains(NullAllocator, Memblock blk) noexcept{ return blk.ptr == nullptr; }
SP_CSI Memblock alloc(NullAllocator, size_t, size_t = 0) noexcept{
	return Memblock{nullptr, 0};
}
SP_CSI void free(NullAllocator, Memblock = Memblock{}) noexcept{}
SP_CSI Memblock realloc(NullAllocator, Memblock, size_t) noexcept{
	return Memblock{nullptr, 0};
}





// FALLBACK ALLOCATOR
template<class T0, class T1>
struct FallbackAllocator{
	constexpr static size_t Alignment = (
		std::remove_pointer_t<T0>::Alignment && std::remove_pointer_t<T1>::Alignment ? (
			std::remove_pointer_t<T0>::Alignment<std::remove_pointer_t<T1>::Alignment ? (
				std::remove_pointer_t<T0>::Alignment
			) : (
				std::remove_pointer_t<T1>::Alignment
			)
		) : (
			std::remove_pointer_t<T0>::Alignment | std::remove_pointer_t<T1>::Alignment
		)
	);
	constexpr static bool IsAware = std::remove_pointer_t<T1>::IsAware;
	constexpr static bool HasMassFree = (
		std::remove_pointer_t<T0>::HasMassFree && std::remove_pointer_t<T1>::HasMassFree
	);

	typedef std::remove_pointer_t<T0> MainType;
	typedef std::remove_pointer_t<T1> SpareType;

	static_assert(
		std::remove_pointer_t<T0>::IsAware,
		"main allocator of fallback allocator must be aware of what memory belongs to it"
	);

	[[no_unique_address]] T0 main;
	[[no_unique_address]] T1 spare;
};

template<class T0, class T1>
SP_CSI bool contains(const FallbackAllocator<T0, T1> &al, Memblock blk) noexcept{
	return contains(deref(al.main), blk) || contains(deref(al.spare), blk);
}

template<class T0, class T1>
SP_CSI Memblock alloc(FallbackAllocator<T0, T1> &al, size_t size) noexcept{
	static_assert(
		std::remove_pointer_t<T0>::Alignment || std::remove_pointer_t<T1>::Alignment,	
		"missing alignemt parameter for fallback allocator with unspecified alignment"
	);

	Memblock blk;
	if constexpr (std::remove_pointer_t<T0>::Alignment)
		blk = alloc(deref(al.main), size);
	else
		blk = alloc(deref(al.main), size, std::remove_pointer_t<T1>::Alignment);
	if (blk.ptr) return blk;
	
	if constexpr (std::remove_pointer_t<T1>::Alignment)
		return alloc(deref(al.spare), size);
	else
		return alloc(deref(al.spare), size, std::remove_pointer_t<T0>::Alignment);
}

template<class T0, class T1>
SP_CSI Memblock alloc(
	FallbackAllocator<T0, T1> &al, size_t size, size_t alignment
) noexcept{
	static_assert(
		!std::remove_pointer_t<T0>::Alignment || !std::remove_pointer_t<T1>::Alignment	,
		"alignment for both suballocators is fixed, you should not specify it"
	);

	Memblock blk;
	if constexpr (std::remove_pointer_t<T0>::Alignment)
		blk = alloc(deref(al.main), size);
	else
		blk = alloc(deref(al.main), size, alignment);
	if (blk.ptr) return blk;
	
	if constexpr (std::remove_pointer_t<T1>::Alignment)
		return alloc(deref(al.spare), size);
	else
		return alloc(deref(al.spare), size, alignment);
}

template<class T0, class T1>
SP_CSI void free(FallbackAllocator<T0, T1> &al, Memblock blk) noexcept{
	if (contains(deref(al.main), blk))
		free(deref(al.main), blk);
	else
		free(deref(al.spare), blk);
}

template<class T0, class T1>
SP_CSI void free(FallbackAllocator<T0, T1> &al) noexcept{
	free(deref(al.main));
	free(deref(al.spare));
}

template<class T0, class T1>
SP_CSI Memblock realloc(
	FallbackAllocator<T0, T1> &al, Memblock blk, size_t size
) noexcept{
	static_assert(
		std::remove_pointer_t<T0>::Alignment || std::remove_pointer_t<T1>::Alignment,	
		"missing alignemt parameter for fallback allocator with unspecified alignment"
	);

	if (contains(deref(al.main), blk)){
		Memblock newblk;
		if constexpr (std::remove_pointer_t<T0>::Alignment)
			newblk = realloc(deref(al.main), blk, size);
		else
			newblk = realloc(deref(al.main), blk, size, std::remove_pointer_t<T0>::Alignment);
	
		if (newblk.ptr) return newblk;
		
		if constexpr (std::remove_pointer_t<T1>::Alignment)
			newblk = alloc(deref(al.spare), size);
		else
			newblk = alloc(deref(al.spare), size, std::remove_pointer_t<T0>::Alignment);
		
		if (newblk.ptr){
			for (uint8_t *I=blk.ptr, *J=newblk.ptr; I!=blk.ptr+blk.size; ++I, ++J) *J=*I;
			free(deref(al.main), blk);
		}
		return newblk;
	}
	if constexpr (std::remove_pointer_t<T1>::Alignment)
		return realloc(deref(al.spare), blk, size);
	else
		return realloc(deref(al.spare), blk, size, std::remove_pointer_t<T0>::Alignment);
}

template<class T0, class T1>
SP_CSI Memblock realloc(
	FallbackAllocator<T0, T1> &al, Memblock blk, size_t size, size_t alignment
) noexcept{
	static_assert(
		!std::remove_pointer_t<T0>::Alignment || !std::remove_pointer_t<T1>::Alignment,
		"alignment for both suballocators is fixed, you should not specify it"
	);

	if (contains(deref(al.main), blk)){
		Memblock newblk;
		if constexpr (std::remove_pointer_t<T0>::Alignment)
			newblk = realloc(deref(al.main), size);
		else
			newblk = realloc(deref(al.main), size, alignment);
	
		if (newblk.ptr) return newblk;
		
		if constexpr (std::remove_pointer_t<T1>::Alignment)
			newblk = alloc(deref(al.spare), size);
		else
			newblk = alloc(deref(al.spare), size, alignment);
		
		if (newblk.ptr){
			for (uint8_t *I=blk.ptr, *J=newblk.ptr; I!=blk.ptr+blk.size; ++I, ++J) *J=*I;
			free(deref(al.main), blk);
		}
		return newblk;
	}
	if constexpr (std::remove_pointer_t<T1>::Alignment)
		return realloc(deref(al.spare), blk, size);
	else
		return realloc(deref(al.spare), blk, size, alignment);
}





// MALLOC ALLOCATOR
template<
	void *(*M)(size_t) = (void *(*)(size_t))::malloc,
	void (*F)(void *) = nullptr,
	void *(*R)(void *, size_t) = nullptr
>
struct MallocAllocator{
	constexpr static size_t Alignment = alignof(max_align_t);
	constexpr static bool IsAware = false;
	constexpr static bool HasMassFree = false;
	
	constexpr static auto Malloc = M;
	constexpr static auto Free = (
		M==(void *(*)(size_t))::malloc ? (void (*)(void *))::free : F
	);
	constexpr static auto Realloc = (
		M==(void *(*)(size_t))::malloc ? (void *(*)(void *, size_t))::realloc : R
	);

	static_assert(
		F || M==(void *(*)(size_t))::malloc,
		"\"free\" function must come in pair with costum malloc"
	);
};

template<void *(*M)(size_t), void (*F)(void *), void *(*R)(void *, size_t)>
SP_CSI bool contains(MallocAllocator<M, F, R>, Memblock) noexcept{
	return true;
}

template<void *(*M)(size_t), void (*F)(void *), void *(*R)(void *, size_t)>
SP_CSI Memblock alloc(MallocAllocator<M, F, R>, size_t size) noexcept{
	return Memblock{(uint8_t *)MallocAllocator<M, F, R>::Malloc(size), size};
}

template<void *(*M)(size_t), void (*F)(void *), void *(*R)(void *, size_t)>
SP_CSI void free(MallocAllocator<M, F, R>, Memblock blk) noexcept{
	MallocAllocator<M, F, R>::Free(blk.ptr);
}

template<void *(*M)(size_t), void (*F)(void *), void *(*R)(void *, size_t)>
SP_CSI Memblock realloc(
	MallocAllocator<M, F, R>, Memblock blk, size_t size
) noexcept{
	if constexpr (MallocAllocator<M, F, R>::Realloc){
		return Memblock{(uint8_t *)MallocAllocator<M, F, R>::Realloc(blk.ptr, size), size};
	} else{
		Memblock newBlk;
		newBlk.ptr = (uint8_t *)MallocAllocator<M, F, R>::Malloc(size);
		newBlk.size = size;
		if (newBlk.ptr){
			for (uint8_t *I=beg(blk), *J=beg(newBlk); I!=end(blk); ++I, ++J) *J = *I;
			MallocAllocator<M, F, R>::Free(blk.ptr);
		}
		return newBlk;
	}
}





// ALIGNED ALLOC ALLOCATOR
template<
	void *(*M)(size_t, size_t) = (void *(*)(size_t, size_t))::aligned_alloc,
	void (*F)(void *) = nullptr,
	void *(*R)(void *, size_t, size_t) = nullptr
>
struct AlignedAllocAllocator{
	constexpr static size_t Alignment = 0;
	constexpr static bool IsAware = false;
	constexpr static bool HasMassFree = false;

	constexpr static auto Malloc = M;
	constexpr static auto Free = (
		M==(void *(*)(size_t, size_t))::aligned_alloc ? (void (*)(void *))::free : F
	);
	constexpr static auto Realloc = R;

	static_assert(
		F || M==(void *(*)(size_t, size_t))::aligned_alloc,
		"\"free\" function must come in pair with costum aligned malloc"
	);
};

template<void *(*M)(size_t, size_t), void (*F)(void *), void *(*R)(void *, size_t, size_t)>
SP_CSI bool contains(AlignedAllocAllocator<M, F, R>, Memblock) noexcept{
	return true;
}

template<void *(*M)(size_t, size_t), void (*F)(void *), void *(*R)(void *, size_t, size_t)>
SP_CSI Memblock alloc(
	AlignedAllocAllocator<M, F, R>, size_t size, size_t alignment
) noexcept{
	return Memblock{
		(uint8_t *)AlignedAllocAllocator<M, F, R>::Malloc(size, alignment), size
	};
}

template<void *(*M)(size_t, size_t), void (*F)(void *), void *(*R)(void *, size_t, size_t)>
SP_CSI void free(AlignedAllocAllocator<M, F, R>, Memblock blk) noexcept{
	AlignedAllocAllocator<M, F, R>::Free(blk.ptr);
}

template<void *(*M)(size_t, size_t), void (*F)(void *), void *(*R)(void *, size_t, size_t)>
SP_CSI Memblock realloc(
	AlignedAllocAllocator<M, F, R>, Memblock blk, size_t size, size_t alignment
) noexcept{
	if constexpr (AlignedAllocAllocator<M, F, R>::Realloc){
		return Memblock{
			(uint8_t *)AlignedAllocAllocator<M, F, R>::Realloc(blk.ptr, size, alignment),
			size
		};
	} else{
		Memblock newBlk;
		newBlk.ptr = (uint8_t *)AlignedAllocAllocator<M, F, R>::Malloc(size, alignment);
		newBlk.size = size;
		if (newBlk.ptr){
			for (uint8_t *I=beg(blk), *J=beg(newBlk); I!=end(blk); ++I, ++J) *J = *I;
			AlignedAllocAllocator<M, F, R>::Free(blk.ptr);
		}
		return newBlk;
	}
}




// BUMP ALLOCATOR
template<size_t N, size_t A = alignof(max_align_t)>
struct BumpAllocator{
	constexpr static size_t Alignment = A;
	constexpr static bool IsAware = true;
	constexpr static bool HasMassFree = true;
	
	constexpr static size_t Size = N;

	static_assert(!((Alignment-1) & -Alignment), "alignment must be a power of 2");
	static_assert(N % Alignment == 0, "storage size must be a multiple of the alignment");

	uint8_t *back = storage;
	alignas(A) uint8_t storage[N];
};

template<size_t N, size_t A>
SP_CSI bool contains(const BumpAllocator<N, A> &al, Memblock blk) noexcept{
	return al.storage <= blk.ptr && blk.ptr < al.back;
}

template<size_t N, size_t A>
SP_CSI Memblock alloc(BumpAllocator<N, A> &al, size_t size) noexcept{
	uint8_t *newPointer = align(al.back+size, A);
	size_t allocSize = newPointer - al.back;

	Memblock blk{al.back, allocSize};

	if (newPointer > al.storage+N) return Memblock{nullptr, allocSize};
	
	al.back = newPointer;
	return blk;
}

template<size_t N, size_t A>
SP_CSI void free(BumpAllocator<N, A> &al) noexcept{ al.back = al.storage; }

template<size_t N, size_t A>
SP_CSI void free(BumpAllocator<N, A> &al, Memblock blk) noexcept{
	if (blk.ptr+blk.size == al.back) al.back = blk.ptr;
}

template<size_t N, size_t A>
SP_CSI Memblock realloc(
	BumpAllocator<N, A> &al, Memblock blk, size_t size
) noexcept{
	if (blk.ptr+blk.size == al.back){
		uint8_t *newBack = align(blk.ptr+size, A);
		if (newBack <= al.storage+N){
			al.back = newBack;
			return Memblock{blk.ptr, newBack-blk.ptr};
		}
	}
	return Memblock{nullptr, 0};
}



// STACK ALLOCATOR
template<size_t N, size_t A = alignof(max_align_t)>
struct StackAllocator{
	constexpr static size_t Alignment = A;
	constexpr static bool IsAware = true;
	constexpr static bool HasMassFree = true;
	
	constexpr static size_t Size = N;

	static_assert(!((Alignment-1) & -Alignment), "alignment must be a power of 2");
	static_assert(N % Alignment == 0, "storage size must be a multiple of the alignment");

	uint8_t *back = storage;
	uint8_t *front = storage;
	alignas(A) uint8_t storage[N];
};

template<size_t N, size_t A>
SP_CSI bool contains(const StackAllocator<N, A> &al, Memblock blk) noexcept{
	return al.front <= blk.ptr && blk.ptr < al.back;
}

template<size_t N, size_t A>
SP_CSI Memblock alloc(StackAllocator<N, A> &al, size_t size) noexcept{
	uint8_t *newPointer = sp::align(al.back+size, A);
	size_t allocSize = newPointer - al.back;

	Memblock blk{al.back, allocSize};

	if (newPointer > al.storage+N){
		if (allocSize < (size_t)(al.front-al.storage))
			al.front -= allocSize;
		else
			blk.ptr = nullptr;
		return blk;
	}
	
	al.back = newPointer;
	return blk;
}

template<size_t N, size_t A>
SP_CSI void free(StackAllocator<N, A> &al) noexcept{
	al.back = al.storage;
	al.front = al.storage;
}

template<size_t N, size_t A>
SP_CSI void free(StackAllocator<N, A> &al, Memblock blk) noexcept{
	if (blk.ptr+blk.size == al.back)
		al.back = blk.ptr;
	else if (blk.ptr == al.front)
		al.front = blk.ptr + blk.size;
}

template<size_t N, size_t A>
SP_CSI Memblock realloc(
	StackAllocator<N, A> &al, Memblock blk, size_t size
) noexcept{
	if (blk.ptr+blk.size == al.back){
		uint8_t *newBack = align(blk.ptr+size, A);
		if (newBack <= al.storage+N){		
			al.back = newBack;
			return Memblock{blk.ptr, newBack-blk.ptr};
		}
	} // else if (blk.ptr == al.front) // maybe add this

	return Memblock{nullptr, 0};
}


/*

// FALLBUMP ALLOCATOR
template<class T = MallocAllocator<>, size_t A = alignof(max_align_t)>
struct FallbumpAllocator{
	constexpr static size_t Alignment = A;
	constexpr static bool IsAware = std::remove_pointer<T>::IsAware;
	constexpr static bool HasMassFree = true;
	
	constexpr static size_t Size = N;

	static_assert(!((Alignment-1) & -Alignment), "alignment must be a power of 2");
	static_assert(N % Alignment == 0, "storage size must be a multiple of the alignment");

	Memblock data = Memblock{nullptr, 0};
	uint8_t *back = nullptr;
	T *allocator;
};

template<class T, size_t A>
SP_CSI bool contains(const FallbumpAllocator<T, A> &al, Memblock blk) noexcept{
	return al.data.ptr <= blk.ptr && blk.ptr < al.back;
}

template<class T, size_t A>
SP_CSI Memblock alloc(FallbumpAllocator<T, A> &al, size_t size) noexcept{
	uint8_t *new_pointer = align(al.back+size, A);
	size_t alloc_size = new_pointer - al.back;


	if (new_ptr > al.data.ptr+al.data.size){
		al.data = realloc(al.allocator, al.data, al.data.size+alloc_size);
		new_pointer = al.data.ptr + al
	}
	
	Memblock blk{al.back, alloc_size};
	al.back = new_pointer;
	return blk;
}

template<size_t N, size_t A>
SP_CSI void free(FallbumpAllocator<N, A> &al) noexcept{ al.back = al.storage; }

template<size_t N, size_t A>
SP_CSI void free(FallbumpAllocator<N, A> &al, Memblock blk) noexcept{
	if (blk.ptr+blk.size == al.back) al.back = blk.ptr;
}

template<size_t N, size_t A>
SP_CSI Memblock realloc(
	FallbumpAllocator<N, A> &al, Memblock blk, size_t size
) noexcept{
	if (blk.ptr+blk.size == al.back){
		uint8_t *newBack = align(blk.ptr+size, A);
		if (newBack <= al.storage+N){
			al.back = newBack;
			return Memblock{blk.ptr, newBack-blk.ptr};
		}
	}
	return Memblock{nullptr, 0};
}

*/

















// FREE LIST
template<class A>
struct FreeListAllocator{
	constexpr static size_t Alignment = std::remove_pointer_t<A>::Alignment;
	constexpr static bool IsAware = std::remove_pointer_t<A>::IsAware;
	constexpr static bool HasMassFree = std::remove_pointer_t<A>::HasMassFree;
	
	static_assert(
		alignof(uint8_t *) <= std::remove_pointer_t<A>::Alignment,
		"aglignment of underlying allocator must be"
		"compltible with the alignment of the pointer"
	);

	struct Node{
		Node *next;
		size_t size;
	};

	typedef std::remove_pointer_t<A> BaseType;

	[[no_unique_address]] A allocator;
	Node *head = nullptr;
};


template<class A>
SP_CSI bool contains(const FreeListAllocator<A> &al, Memblock blk) noexcept{
	for (const typename FreeListAllocator<A>::Node *node=al.head; node; node=node->next)
		if (blk.ptr == (uint8_t *)node) return true;
	return contains(al.allocator, blk);
}

template<class A>
SP_CSI Memblock alloc(FreeListAllocator<A> &al, size_t size) noexcept{
	typename FreeListAllocator<A>::Node **nodePtr = &al.head;
	for (typename FreeListAllocator<A>::Node *node=al.head; node; node=node->next){
		if (size <= node->size){
			if (size <= node->size/2){
				auto end = (typename FreeListAllocator<A>::Node *)align(
					(uint8_t *)node+size, std::remove_pointer_t<A>::Alignment
				);
				size = (uint8_t *)end - (uint8_t *)node;
				end->next = node->next;
				end->size = node->size - size;
				*nodePtr = end;
				return Memblock{(uint8_t *)node, size};
			} else{
				*nodePtr = node->next;
				return Memblock{(uint8_t *)node, node->size};
			}
		}
		nodePtr = &node->next;
	}
	return alloc(deref(al.allocator), size);
}

template<class A>
SP_CSI void free(FreeListAllocator<A> &al, Memblock blk) noexcept{
	typename FreeListAllocator<A>::Node *prevHead = al.head;
	al.head = (typename FreeListAllocator<A>::Node *)blk.ptr;
	((typename FreeListAllocator<A>::Node *)blk.ptr)->next = prevHead;
	((typename FreeListAllocator<A>::Node *)blk.ptr)->size = blk.size;
}

template<class A>
SP_CSI void free(std::enable_if_t<A::HasMassFree, FreeListAllocator<A>> &al) noexcept{
	free(deref(al.allocator));
	al.head = nullptr;
}

template<class A>
SP_CSI Memblock realloc(FreeListAllocator<A> &al, Memblock blk, size_t size) noexcept{
	Memblock newBlk = alloc(al, size);
	if (newBlk.ptr){
		for (uint8_t *I=beg(blk), *J=beg(newBlk); I!=end(blk); ++I, ++J) *J = *I;
		free(al, blk);
	}
	return newBlk;
}

/*

// BLOCK ALLOCATOR
template<size_t N, class A>
struct BlockAllocator{
	constexpr static size_t Alignment = std::remove_pointer<A>::Alignment;
	constexpr static bool IsAware = A;
	constexpr static bool HasMassFree = true;
	
	constexpr static size_t BlockNumber = N;

	static_assert(!((Alignment-1) & -Alignment), "alignment must be a power of 2");
	static_assert(N % Alignment == 0, "storage size must be a multiple of the alignment");
	
	struct Header{
		uint32_t pos;
		uint32_t size;
		Header *prev;
	};

	Header *ptr = nullptr;
	[[no_unique_address]] A allocator;
};

template<size_t N, class A>
SP_CSI bool contains(const BlockAllocator<N, A> &al, Memblock blk) noexcept{
	return al.storage <= blk.ptr && blk.ptr < al.back;
}

template<size_t N, class A>
SP_CSI Memblock alloc(BlockAllocator<N, A> &al, size_t size) noexcept{
	Header header = *al.ptr;
	uint8_t *data = (uint8_t *)(al.ptr+1);

	size_t newPos = align(data+haeder.pos+size, A) - data;
	size_t allocSize = newPos - header.pos;

	Memblock blk{data+header.pos, allocSize};

	if (newPos > header.size){
		// alloc new block
	}
	
	al.ptr->pos = newPos;
	return blk;
}

template<size_t N, class A>
SP_CSI void free(BumpAllocator<N, A> &al) noexcept{ al.back = al.storage; }

template<size_t N, size_t A>
SP_CSI void free(BumpAllocator<N, A> &al, Memblock blk) noexcept{
	if (blk.ptr+blk.size == al.back) al.back = blk.ptr;
}

template<size_t N, size_t A>
SP_CSI Memblock realloc(
	BumpAllocator<N, A> &al, Memblock blk, size_t size
) noexcept{
	if (blk.ptr+blk.size == al.back){
		uint8_t *newBack = align(blk.ptr+size, A);
		if (newBack <= al.storage+N){
			al.back = newBack;
			return Memblock{blk.ptr, newBack-blk.ptr};
		}
	}
	return Memblock{nullptr, 0};
}
*/



} // END OF NAMESPACE	///////////////////////////////////////////////////////////////////
